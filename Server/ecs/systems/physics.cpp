#include "../../header/ecs.h"
#include "../../header/helper.h"

using namespace ecs::system;
using namespace ecs::component;

void physics::userInput(json keysAndID)
{
	unsigned short entityID = sessionIDToEntityID[keysAndID["sessionID"]]; //will get the entity ID

	//does this lock below to prevent deadlocking
	mutexs::userLocationsMutex.lock(); //will block attempts to lock this mutex again, thereby allowing us to prevent accidentally accessing shared data at the wrong time, unlocked before the next lock

	unsigned short physicsVectorIndex = physicsObjects.entityToVectorMap(entityID);
	unsigned short drawablesVectorIndex = drawables.entityToVectorMap(entityID);

	sf::Vector2f *userVelocity = &ecs::component::physicsObjects.compVec[physicsVectorIndex].velocity;
	bool *onFloor = &ecs::component::physicsObjects.compVec[physicsVectorIndex].onFloor;
	sf::Vector2i *direction = &ecs::component::drawables.compVec[drawablesVectorIndex].direction;

	if ((keysAndID["left"] && keysAndID["right"]) || (!keysAndID["left"] && !keysAndID["right"]))
	{ //if both or neither of them are pressed, the velocity is zero
		userVelocity->x = 0;
	}
	else
	{
		if (keysAndID["left"]){
			userVelocity->x = -velocity.x;
		}
		if (keysAndID["right"]){
			userVelocity->x = velocity.x;
		}
	}

	if (keysAndID["jump"] && *onFloor)
	{									   //will jump up and indicate that it's not on the floor anymore
		userVelocity->y = -acceleration.y; //negative y is upwards
		*onFloor = false;
	}
	
	//if velocity in x/y is more than 0 (so right of the screen or down respectively), then direction = 1, otherwise the opposite is direction = -1, and 0 means it's not moving
	if(userVelocity->x > 0){
		direction->x = 1;
	}else if(userVelocity->x < 0){
		direction->x = -1;
	}else{
		//direction->x = 0;
	}
	
	if(userVelocity->y > 0){
		direction->y = 1;
	}else if(userVelocity->y < 0){
		direction->y = -1;
	}else{
		direction->y = 0;
	}

	sf::Vector2f userCoordinates = ecs::component::physicsObjects.compVec[physicsVectorIndex].coordinates; //gets this data before unlocking, we need it for thrown out item
	int direction_x = direction->x; //gets this data before unlocking, we need it for thrown out item

	mutexs::userLocationsMutex.unlock();

	if(keysAndID["throwItem"].get<bool>()){ //sends a message to the user to update their inventory and redraw it to include the fact that their item is now gone
		mutexs::mainUserLockMutex.lock(); //need to use this lock
		ecs::component::user* userPtr = &users.compVec[users.entityToVectorMap(entityID)]; //gets the user object
		int thrownItemTextureID = userInventories[userPtr->userID][0][userPtr->currentItemSelection]; //need this for the texture of the item

		if(thrownItemTextureID != 0){ //if it's not an empty inventory index
			userInventories[userPtr->userID][0][userPtr->currentItemSelection] = 0; //sets the item they've selected to 0 as they've thrown it out
			userPtr->currentItem = 0; //the current item they're holding is also now a 0

			sf::Packet packet;	 //a packet to hold a string
			std::string sendString = "USER::INVENTORY::" + userInventories[userPtr->userID].dump();
			packet << sendString; //puts json data into the packet

			userPtr->socket->send(packet); //sends the packet to the user currently being looped over

			int thrownEntityID = ecs::entity::superEntityManager.create({DRAWABLE, PHYSICAL});
			ecs::component::physical* physicalObj = &physicsObjects.compVec[physicsObjects.entityToVectorMap(thrownEntityID)];
			physicalObj->coordinates = sf::Vector2f(userCoordinates.x, userCoordinates.y - 5); //sets thrown item's coordinates as this (around player's waist)
			physicalObj->velocity = sf::Vector2f(direction_x, 0); //sets thrown item's velocity as this
			physicalObj->objType = ITEM; //special type of collision/collider object
			drawables.compVec[drawables.entityToVectorMap(thrownEntityID)].texture = "resources/items/" + std::to_string(thrownItemTextureID) + ".png"; //sets the texture of the fallen item

			ecs::system::coordinatesStruct thrownItemChunk = coordinatesStruct(chunkCoordHelperX(physicalObj->coordinates.x, chunkPixelSize_x), chunkCoordHelperY(physicalObj->coordinates.y, chunkPixelSize_y));
			//mutexs::getInstance()->chunkLockMutex.lock();
			chunks[thrownItemChunk].second.push_back(entity::entity(thrownEntityID));
			//mutexs::getInstance()->chunkLockMutex.unlock();
		}
		mutexs::mainUserLockMutex.unlock();
	}	
}

bool physics::AABB_collision(int collisionEntityID, int colliderEntityID){ //true = collision, false = no collision
	auto collisionObject = &physicsObjects.compVec[physicsObjects.entityToVectorMap(collisionEntityID)];
	auto colliderObject = &physicsObjects.compVec[physicsObjects.entityToVectorMap(colliderEntityID)];

	bool collision = false;
	if(collisionObject->objType == COLLISION){
		for(auto &colliderVert : colliderObject->boxCorners){
			if(colliderVert.x+colliderObject->coordinates.x >= collisionObject->boxCorners[0].x && colliderVert.x+colliderObject->coordinates.x <= collisionObject->boxCorners[3].x){
				if(colliderVert.y+colliderObject->coordinates.y <= collisionObject->boxCorners[3].y && colliderVert.y+colliderObject->coordinates.y >= collisionObject->boxCorners[0].y){
					colliderObject->coordinates.y = collisionObject->boxCorners[0].y; //collision resolution (hacky way, do it properly next time)
					if(colliderObject->velocity.y > 0){ //collision resolution (hacky way, do it properly next time)
						colliderObject->velocity.y = 0; //collision resolution (hacky way, do it properly next time)
					}
					colliderObject->onFloor = true; //collision resolution (hacky way, do it properly next time)
					//std::cout << colliderVert.x+colliderObject->coordinates.x << " -- " << collisionObject->boxCorners[0].x << " -- " << collisionObject->boxCorners[3].x << "\n";
					//std::cout << colliderVert.y+colliderObject->coordinates.y << " -- " << collisionObject->boxCorners[0].y << " -- " << collisionObject->boxCorners[3].y << "\n";
					collision = true;
					//break;
					//return true;
				}
			}
		}
	}
	
	if(collisionObject->objType == ITEM && users.entityToVectorMap(colliderEntityID) != -1){ //if the item 'collided with' (nearby) is near a user do this
		float diffX = colliderObject->coordinates.x - collisionObject->coordinates.x;
		float diffY = colliderObject->coordinates.y - collisionObject->coordinates.y;
		if(sqrt(pow(diffX, 2) + pow(diffY, 2)) < 5){ //if the distance between the coordinates is less than 5 units count it as a 'collision' (being nearby, so can be picked up)
			collision = true;
		}
	}

	return collision;
}

bool physics::checkCollision(entity::entity colliderEntity){
	sf::Vector2f *coordinates = &physicsObjects.compVec[physicsObjects.entityToVectorMap(colliderEntity.id)].coordinates;

	//checks collision detection, uses mod math to find current chunk, then does collision detection
	coordinatesStruct currentChunkCoords(chunkCoordHelperX(int(coordinates->x), chunkPixelSize_x), chunkCoordHelperY(int(coordinates->y), chunkPixelSize_y));

	std::vector<entity::entity> collisionObjects; //so like the floor/stairs etc
	for(int j = -1; j <= 1; j++){
		for(int i = -1; i <= 1; i++){ //will loop through the chunk the user is in, as well as the 2 adjacent chunks
			for(auto &entity : chunks[coordinatesStruct(currentChunkCoords.coordinates.first+i, currentChunkCoords.coordinates.second+j)].second){ //will loop through all the physics objects in this chunk
				if(physicsObjects.entityToVectorMap(entity.id) != -1){ //if it exists in the physics object map
					objectType objType = physicsObjects.compVec[physicsObjects.entityToVectorMap(entity.id)].objType;
					if(objType == COLLISION || objType == ITEM){ //if it is a static collision object (like the floor), or an item (thrown out of an inventory)
						collisionObjects.push_back(entity); 
					}
				}
			}
		}
	}

	bool hasCollisionHappened = false;

	for(auto &collisionEntity : collisionObjects){
		//std::cout << "checking for collisions..." << " ";
		if(AABB_collision(collisionEntity.id, colliderEntity.id)){ //if there is an actual collision
			if(users.entityToVectorMap(colliderEntity.id) != -1){ //this means the collider is a user
				if(physicsObjects.compVec[physicsObjects.entityToVectorMap(collisionEntity.id)].objType == ITEM && physicsObjects.compVec[physicsObjects.entityToVectorMap(collisionEntity.id)].onFloor == true){
					std::cout << "a user collides with an item" << "\n";
				}
			}
			if(physicsObjects.compVec[physicsObjects.entityToVectorMap(colliderEntity.id)].objType == ITEM && physicsObjects.compVec[physicsObjects.entityToVectorMap(colliderEntity.id)].onFloor == true){
				//above covers the other case where the item is a collider instead (still need to stop it)
				physicsObjects.compVec[physicsObjects.entityToVectorMap(colliderEntity.id)].velocity.x = 0; //once an item has fallen to the ground it should stop moving
			}
			hasCollisionHappened = true; //there has been a collision
		}
	}
	//TO DO: implement the collider-collision stuff, so like physical users which other users can't just walk through

	return hasCollisionHappened;
}

void physics::moveEntities()
{
	std::lock_guard<std::mutex> lock(mutexs::userLocationsMutex); //will block attempts to lock this mutex again, thereby allowing us to prevent accidentally accessing shared data at the wrong time, released at the end of scope
	
	int componentIndex = 0;
	ecs::entity::entity tempEntity; //used to put the users entity inside some chunk
	for (int j = 0; j < ecs::component::physicsObjects.compVec.size(); j++)
	{ //loops through physical structs, by reference
		tempEntity.id = 0;
		auto physicsalStruct = &ecs::component::physicsObjects.compVec[j];
		componentIndex = j;

		if(physicsalStruct->objType == COLLISION){
			continue; //we don't really care about floors/stairs colliding
		}

		////////1st we update any velocities as required
		physicsalStruct->velocity.y += deceleration.y; //if it's not on ground it should accelerate downwards

		////////2nd we do collision detection and resolution based on the current positions and velocities, but they will potentially be updated here
		checkCollision(physicsObjects.vectorToEntityMap(componentIndex)); //does the collision detection

		////////3rd we use the velocity which may or may not have been updated in the previous step to get the new coordinates/chunkCoordinates, and update the coordinates themselves
		coordinatesStruct currentChunkCoords(
			chunkCoordHelperX(physicsalStruct->coordinates.x, chunkPixelSize_x), 
			chunkCoordHelperY(physicsalStruct->coordinates.y, chunkPixelSize_y)
		);  //get what the current chunk coordinates would be in the world using simple mod math
		physicsalStruct->coordinates.x += physicsalStruct->velocity.x; //update the x position
		physicsalStruct->coordinates.y += physicsalStruct->velocity.y; //update the y position
		sf::Vector2f newCoordinates(physicsalStruct->coordinates.x, physicsalStruct->coordinates.y);
		coordinatesStruct newChunkCoords( //get what the new chunk coordinates would be in the world using simple mod math
			chunkCoordHelperX(newCoordinates.x, chunkPixelSize_x), 
			chunkCoordHelperY(newCoordinates.y, chunkPixelSize_y)
		);

		////////4th we update the information held in the chunks if the new chunk coordinate calculated is different from the current one
		if (newChunkCoords.coordinates.first != currentChunkCoords.coordinates.first || newChunkCoords.coordinates.second != currentChunkCoords.coordinates.second){
			unsigned int entityID = physicsObjects.vectorToEntityMap(componentIndex);

			for (int i = 0; i < chunks[currentChunkCoords].second.size(); i++)
			{ //get the coordinate retrieved above, and get the chunk at that coordinate, and loop through the vector storing all the entities at that coordinate
				if (chunks[currentChunkCoords].second[i].id == entityID)
				{																			  //if the entityID is equal to the one we retrieved, it's the user we're looking for
					chunks[currentChunkCoords].second.erase(chunks[currentChunkCoords].second.begin() + i); //remove the user from the vector
					chunks[currentChunkCoords].first.userCount--; //decrements the number of users in this chunk
					//std::cout << "decrementing user count (moving) \n";
					tempEntity.id = entityID; //sets the correct entityID
					chunks[newChunkCoords].second.push_back(tempEntity); //and pushes to the vector in the new chunk
					chunks[newChunkCoords].first.userCount++; //increments the number of users in this chunk
					break;
				}
			}
			//std::cout << chunks[newChunkCoords].size() << " -- " << chunks[currentChunkCoords].size() << std::endl;
			//std::cout << "moved " << users.compVec[users.entityToVectorMap(entityID)].username << " to chunk: " << newChunkCoords.coordinates.first << ", " << newChunkCoords.coordinates.second << std::endl;
			updateActiveChunkData::getInstance()->updateActiveChunks(); //updates the active chunks, we call it after anyone moves in any chunk as that's when a new chunk may need to be generated
		}
	}
}