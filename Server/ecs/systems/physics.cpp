#include "../../header/ecs.h"
#include "../../header/helper.h"

using namespace ecs::system;
using namespace ecs::component;

void physics::userInput(json keysAndID)
{
	std::lock_guard<std::mutex> lock(mutexs::userLocationsMutex); //will block attempts to lock this mutex again, thereby allowing us to prevent accidentally accessing shared data at the wrong time, released at the end of scope

	unsigned short entityID = sessionIDToEntityID[keysAndID["sessionID"]];
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
		direction->x = 0;
	}
	
	if(userVelocity->y > 0){
		direction->y = 1;
	}else if(userVelocity->y < 0){
		direction->y = -1;
	}else{
		direction->y = 0;
	}	
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

		if (!physicsalStruct->onFloor){
			physicsalStruct->velocity.y += deceleration.y; //if it's not on ground it should accelerate downwards
		}

		sf::Vector2f newCoordinates(physicsalStruct->coordinates.x + physicsalStruct->velocity.x, physicsalStruct->coordinates.y + physicsalStruct->velocity.y);

		coordinatesStruct newChunkCoords(chunkCoordHelperX(int(newCoordinates.x), chunkPixelSize_x), chunkCoordHelperY(int(newCoordinates.y), chunkPixelSize_y));
		coordinatesStruct currentChunkCoords(chunkCoordHelperX(int(physicsalStruct->coordinates.x), chunkPixelSize_x), chunkCoordHelperY(int(physicsalStruct->coordinates.y), chunkPixelSize_y)); //get what their coordinates would be in the world using simple mod math

		//the below moves an entity out of its old chunk and into the one it is now in
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
				
			updateActiveChunkData::getInstance()->updateActiveChunks(); //updates the active chunks
		}


		if (physicsalStruct->coordinates.y + physicsalStruct->velocity.y > -5)
		{ //this means below ground level, so manually set to ground
			physicsalStruct->velocity.y = 0;
			physicsalStruct->coordinates.y = -5;
			physicsalStruct->onFloor = true;
		}

		physicsalStruct->coordinates.x += physicsalStruct->velocity.x;
		physicsalStruct->coordinates.y += physicsalStruct->velocity.y;
	}
}