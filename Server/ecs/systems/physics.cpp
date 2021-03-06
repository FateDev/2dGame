#include "../../header/ecs.h"
#include "../../header/helper.h"

using namespace ecs::system;
using namespace ecs::component;
using namespace::ecs;

void physics::userInput(json keysAndID) {
	unsigned short entityID = sessionIDToEntityID[keysAndID["sessionID"]]; //will get the entity ID

	//does this lock below to prevent deadlocking
	mutexs::userLocationsMutex.lock(); //will block attempts to lock this mutex again, thereby allowing us to prevent accidentally accessing shared data at the wrong time, unlocked before the next lock

	unsigned short physicsVectorIndex = physicsObjects.entityToVectorMap(entityID);
	unsigned short drawablesVectorIndex = drawables.entityToVectorMap(entityID);

	sf::Vector2f *userVelocity = &ecs::component::physicsObjects.compVec[physicsVectorIndex].velocity;
	bool *onFloor = &ecs::component::physicsObjects.compVec[physicsVectorIndex].onFloor;
	sf::Vector2i *direction = &ecs::component::drawables.compVec[drawablesVectorIndex].direction;

	if(keysAndID["use_item"] == true){
		int currentItem = users.compVec[users.entityToVectorMap(entityID)].currentItem;
		if(itemsFromFile[currentItem]["type"].get<std::string>() == "weapon"){
			float damage = itemsFromFile[currentItem]["damage"].get<float>();
			mutexs::attackVectorMutex.lock();
			attacks.push_back({entityID, damage}); //pushes atttacks to this for processing, so we can process all of the attack stuff in one place
			mutexs::attackVectorMutex.unlock();
		}
		else if(currentItem == 0) {
			mutexs::attackVectorMutex.lock();
			attacks.push_back({ entityID, 1 }); //if they are holding nothing they can do 1 damage
			mutexs::attackVectorMutex.unlock();
		}
	}

	if ((keysAndID["left"] && keysAndID["right"]) || (!keysAndID["left"] && !keysAndID["right"])) { //if both or neither of them are pressed, the velocity is zero
		userVelocity->x = 0;
	} else {
		if (keysAndID["left"]) {
			userVelocity->x = -velocity.x;
		}
		if (keysAndID["right"]) {
			userVelocity->x = velocity.x;
		}
	}

	if (keysAndID["jump"] && *onFloor) { //will jump up and indicate that it's not on the floor anymore
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

	itemSystem::getInstance()->throwItem(keysAndID, entityID, direction_x, userCoordinates); //runs a subroutine for throwing items (check if they should be thrown too)
}

bool physics::AABB_collision(int collisionEntityID, int colliderEntityID){ //true = collision, false = no collision
	auto collisionObject = &physicsObjects.compVec[physicsObjects.entityToVectorMap(collisionEntityID)];
	auto colliderObject = &physicsObjects.compVec[physicsObjects.entityToVectorMap(colliderEntityID)];

	sf::Vector2f topLeftCorner = {collisionObject->boxCorners[0].x + collisionObject->coordinates.x, collisionObject->boxCorners[0].y + collisionObject->coordinates.y};
	sf::Vector2f bottomRightCorner = {collisionObject->boxCorners[3].x + collisionObject->coordinates.x, collisionObject->boxCorners[3].y + collisionObject->coordinates.y};

	bool collision = false;
	if(collisionObject->objType == COLLISION){
		for(auto &colliderVert : colliderObject->boxCorners){
			if(colliderVert.x+colliderObject->coordinates.x >= topLeftCorner.x && colliderVert.x+colliderObject->coordinates.x <= bottomRightCorner.x){
				if(colliderVert.y+colliderObject->coordinates.y <= bottomRightCorner.y && colliderVert.y+colliderObject->coordinates.y >= topLeftCorner.y){
					colliderObject->coordinates.y = topLeftCorner.y; //collision resolution (hacky way, do it properly next time)
					if(colliderObject->velocity.y > 0){ //collision resolution (hacky way, do it properly next time)
						colliderObject->velocity.y = 0; //collision resolution (hacky way, do it properly next time)
					}
					colliderObject->onFloor = true; //collision resolution (hacky way, do it properly next time)
					collision = true;
				}
			}
		}
	} else if(users.entityToVectorMap(colliderEntityID) != -1 && !collision){
		collision = itemSystem::getInstance()->userItemCollision(collisionObject, colliderObject); //checks if a user should pick up an item
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
			if(mobs.entityToVectorMap(colliderEntity.id) != -1){ //this means it's a mob
				continue;
			}
			itemSystem::getInstance()->pickupItem(colliderEntity, collisionEntity);
			hasCollisionHappened = true; //there has been a collision
		}
	}
	//TO DO: implement the collider-collision stuff, so like physical users which other users can't just walk through

	return hasCollisionHappened;
}

void physics::moveEntities() {
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

		int entityID = physicsObjects.vectorToEntityMap(componentIndex);

		////////1st we update any velocities as required
		physicsalStruct->velocity.y += deceleration.y; //if it's not on ground it should accelerate downwards
		//and get the current chunk coords, before making any changes
		coordinatesStruct currentChunkCoords(
			chunkCoordHelperX(physicsalStruct->coordinates.x, chunkPixelSize_x), 
			chunkCoordHelperY(physicsalStruct->coordinates.y, chunkPixelSize_y)
		);  //get what the current chunk coordinates would be in the world using simple mod math, we do it at this stage rather than later as checkCollision and mobMovement may change the position after this point

		////////2nd we do collision detection and resolution based on the current positions and velocities, but they will potentially be updated here
		checkCollision(entityID); //does the collision detection

		////////2.5nd do mob movement
		if(entity::superEntityManager.getType(entity::entity(entityID)) == entity::MOB){ //if it's a mob, do mob stuff with it
			mobSystem::getInstance()->mobMovement(entityID);
		}

		////////3rd we use the velocity which may or may not have been updated in the previous step to get the new coordinates/chunkCoordinates, and update the coordinates themselves
		physicsalStruct->coordinates.x += physicsalStruct->velocity.x; //update the x position
		physicsalStruct->coordinates.y += physicsalStruct->velocity.y; //update the y position
		sf::Vector2f newCoordinates(physicsalStruct->coordinates.x, physicsalStruct->coordinates.y);
		coordinatesStruct newChunkCoords( //get what the new chunk coordinates would be in the world using simple mod math
			chunkCoordHelperX(newCoordinates.x, chunkPixelSize_x), 
			chunkCoordHelperY(newCoordinates.y, chunkPixelSize_y)
		);

		////////4th we update the information held in the chunks if the new chunk coordinate calculated is different from the current one
		if(entity::superEntityManager.getType(entity::entity(entityID)) == entity::MOB){
			if(!mobSystem::getInstance()->mobMovementRestrictions(entityID, newChunkCoords)){ //so that they can't go into cities
				chunkMovementManager(newChunkCoords, currentChunkCoords, entityID);
			}
		} else if (newChunkCoords.coordinates.first != currentChunkCoords.coordinates.first || newChunkCoords.coordinates.second != currentChunkCoords.coordinates.second){
			chunkMovementManager(newChunkCoords, currentChunkCoords, entityID);
		}
	}
}

void physics::chunkMovementManager(coordinatesStruct newChunkCoords, coordinatesStruct currentChunkCoords, int entityID){
	for (int i = 0; i < chunks[currentChunkCoords].second.size(); i++){ //get the coordinate retrieved above, and get the chunk at that coordinate, and loop through the vector storing all the entities at that coordinate
		if (chunks[currentChunkCoords].second[i].id == entityID) { //if the entityID is equal to the one we retrieved, it's the user we're looking for
			if(entity::superEntityManager.getType(entity::entity(entityID)) == entity::USER){ //if it's a user
				chunks[currentChunkCoords].first.userCount--; //decrements the number of users in this chunk
				chunks[newChunkCoords].first.userCount++; //increments the number of users in this chunk
				//std::cout << "moved " << users.compVec[users.entityToVectorMap(entityID)].username << " to chunk: " << newChunkCoords.coordinates.first << ", " << newChunkCoords.coordinates.second << std::endl;
			} else if(entity::superEntityManager.getType(entity::entity(entityID)) == entity::ITEM_THROWN){ //if it's a thrown item
				chunks[currentChunkCoords].first.itemCount--;
				chunks[newChunkCoords].first.itemCount++; //increments the number of users in this chunk
			} else if(entity::superEntityManager.getType(entity::entity(entityID)) == entity::MOB){ //if it's a mob
				chunks[currentChunkCoords].first.mobCount--;
				chunks[newChunkCoords].first.mobCount++; //increments the number of users in this chunk
			}
			chunks[currentChunkCoords].second.erase(chunks[currentChunkCoords].second.begin() + i); //remove the user from the vector
			chunks[newChunkCoords].second.push_back(entity::entity(entityID)); //and pushes to the vector in the new chunk
			break;
		}
	}
}

void physics::updateEntitiesInRange(){
	entitiesInRange.clear();
	for(int i = 0; i < mpHpObjects.compVec.size(); i++){
		int mpObj_entityID = mpHpObjects.vectorToEntityMap(i);
		sf::Vector2f entityCoords = physicsObjects.compVec[physicsObjects.entityToVectorMap(mpObj_entityID)].coordinates;
		float width = abs(physicsObjects.compVec[physicsObjects.entityToVectorMap(mpObj_entityID)].boxCorners[0].x - physicsObjects.compVec[physicsObjects.entityToVectorMap(mpObj_entityID)].boxCorners[3].x);
		float height = abs(physicsObjects.compVec[physicsObjects.entityToVectorMap(mpObj_entityID)].boxCorners[0].y - physicsObjects.compVec[physicsObjects.entityToVectorMap(mpObj_entityID)].boxCorners[3].y);
		double radialMagnitude = sqrt(pow(width, 2) + pow(height, 2));

		sf::Vector2f entityAttackBoxOrigin;
		if(drawables.compVec[drawables.entityToVectorMap(mpObj_entityID)].direction.x == -1){
			entityAttackBoxOrigin = {entityCoords.x - (0.5f*width), entityCoords.y + width/2.0f}; //so the origin of the attack box is in front of them
		}else{
			entityAttackBoxOrigin = {entityCoords.x + (1.5f*width), entityCoords.y + width/2.0f}; //so the origin of the attack box is in front of them
		}

		entityCoords.x = chunkCoordHelperX(entityCoords.x, chunkPixelSize_x);
		entityCoords.y = chunkCoordHelperY(entityCoords.y, chunkPixelSize_y);
		for(int i = -1; i <= 1; i++){
			coordinatesStruct currentChunk(entityCoords.x + i, entityCoords.y); //will mean that the adjacent 2 chunks are looped over
			for(auto &entity : chunks[currentChunk].second){
				if(users.entityToVectorMap(entity.id) != -1 || mobs.entityToVectorMap(entity.id) != -1){
					if(entity.id == mpObj_entityID){
						continue;
					}
					if(findDistance(physicsObjects.compVec[physicsObjects.entityToVectorMap(entity.id)].coordinates, entityAttackBoxOrigin) < radialMagnitude){ 
						//if the entity in the outer loop and inner loop are less than a certain distance apart (radialMagntiude), add the inner loop entity to the unordered_set
						//of entities that the outer loop entity can attack (are in range)
						entitiesInRange[mpObj_entityID].insert(entity.id);
					}
				}
			}
		}
	}
}