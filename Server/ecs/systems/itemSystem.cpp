#include "../../header/ecs.h"
#include "../../header/helper.h"

using namespace ecs::system;
using namespace ecs::component;

void itemSystem::throwItem(json keysAndID, int entityID, int direction_x, sf::Vector2f userCoordinates){
	if(keysAndID["throwItem"].get<bool>()){ //sends a message to the user to update their inventory and redraw it to include the fact that their item is now gone
		mutexs::mainUserLockMutex.lock(); //need to use this lock
		ecs::component::user* userPtr = &users.compVec[users.entityToVectorMap(entityID)]; //gets the user object
		int thrownItemID = userInventories[userPtr->userID][0][userPtr->currentItemSelection]; //need this for the texture of the item

		if(thrownItemID != 0){ //if it's not an empty inventory index
			userInventories[userPtr->userID][0][userPtr->currentItemSelection] = 0; //sets the item they've selected to 0 as they've thrown it out
			userPtr->currentItem = 0; //the current item they're holding is also now a 0

			sf::Packet packet;	 //a packet to hold a string
			std::string sendString = "USER::SELECTED::" + std::to_string(userPtr->currentItemSelection) + "USER::INVENTORY::" + userInventories[userPtr->userID].dump();
			packet << sendString; //puts json data into the packet

			userPtr->socket->send(packet); //sends the packet to the user currently being looped over

			int thrownEntityID = ecs::entity::superEntityManager.create(ecs::entity::ITEM_THROWN);
			ecs::component::physical* physicalObj = &physicsObjects.compVec[physicsObjects.entityToVectorMap(thrownEntityID)];
			physicalObj->coordinates = sf::Vector2f(userCoordinates.x, userCoordinates.y - 5); //sets thrown item's coordinates as this (around player's waist)
			physicalObj->velocity = sf::Vector2f(direction_x, 0); //sets thrown item's velocity as this
			physicalObj->objType = ITEM; //special type of collision/collider object
            physicalObj->onFloor = false; //not on the floor
			thrown_items.compVec[thrown_items.entityToVectorMap(thrownEntityID)].item_id = thrownItemID; //sets the id of the fallent item
			drawables.compVec[drawables.entityToVectorMap(thrownEntityID)].texture = itemsFromFile[thrownItemID]["resourceLocation"].get<std::string>();

			ecs::system::coordinatesStruct thrownItemChunk = coordinatesStruct(chunkCoordHelperX(physicalObj->coordinates.x, chunkPixelSize_x), chunkCoordHelperY(physicalObj->coordinates.y, chunkPixelSize_y));
			chunks[thrownItemChunk].second.push_back(entity::entity(thrownEntityID));
		}
		mutexs::mainUserLockMutex.unlock();
	}	
}

void itemSystem::pickupItem(entity::entity colliderEntity, entity::entity collisionEntity){ //the collider and collision could both be ITEMs, and this accounts for it
    if(users.entityToVectorMap(colliderEntity.id) != -1){ //this means the collider is a user
        if(physicsObjects.compVec[physicsObjects.entityToVectorMap(collisionEntity.id)].objType == ITEM && physicsObjects.compVec[physicsObjects.entityToVectorMap(collisionEntity.id)].onFloor == true &&  physicsObjects.compVec[physicsObjects.entityToVectorMap(collisionEntity.id)].velocity.x == 0){
            //std::cout << "a user collides with an item" << "\n";
            
            int entityID = collisionEntity.id;
            int itemTypeID = thrown_items.compVec[thrown_items.entityToVectorMap(entityID)].item_id; //the item ID
            
            ecs::component::user* userPtr = &users.compVec[users.entityToVectorMap(colliderEntity.id)]; //the user which will now receive the item
            bool pickedUpItem = false;
            for(int i = 0; i < 6; i++){
                for(int j = 0; j < 6; j++){
                    if(userInventories[userPtr->userID][i][j].get<int>() == 0){
                        userInventories[userPtr->userID][i][j] = itemTypeID;

                        if(i == 0 && j == userPtr->currentItemSelection){
                            userPtr->currentItem = itemTypeID;
                        }

                        sf::Packet packet;	 //a packet to hold a string
                        std::string sendString = "USER::SELECTED::" + std::to_string(userPtr->currentItemSelection) + "USER::INVENTORY::" + userInventories[userPtr->userID].dump();
                        packet << sendString; //puts json data into the packet

                        userPtr->socket->send(packet); //sends the packet to the user currently being looped over
                        
                        pickedUpItem = true;
                        break;
                    }
                }
                if(pickedUpItem){
                    break;
                }
            }

            sf::Vector2f coordinates = physicsObjects.compVec[physicsObjects.entityToVectorMap(entityID)].coordinates;
            ecs::system::coordinatesStruct removeItemCoordinates(chunkCoordHelperX(int(coordinates.x), chunkPixelSize_x), chunkCoordHelperY(int(coordinates.y), chunkPixelSize_y)); //get what the coordinates would be in the world using simple mod math

            for(int i = 0; i < chunks[removeItemCoordinates].second.size(); i++){
                if(chunks[removeItemCoordinates].second[i].id == entityID){
                    chunks[removeItemCoordinates].second.erase(chunks[removeItemCoordinates].second.begin() + i);
                    break;
                }
            }

            ecs::entity::superEntityManager.destroy(entity::entity(entityID));
        }
    }
    if(physicsObjects.compVec[physicsObjects.entityToVectorMap(colliderEntity.id)].objType == ITEM && physicsObjects.compVec[physicsObjects.entityToVectorMap(colliderEntity.id)].onFloor == true){
        //above covers the other case where the item is a collider instead (still need to stop it)
        physicsObjects.compVec[physicsObjects.entityToVectorMap(colliderEntity.id)].velocity.x = 0; //once an item has fallen to the ground it should stop moving
    }
}

bool itemSystem::userItemCollision(physical* collisionObject, physical* colliderObject){
	if(collisionObject->objType == ITEM){ //if the item 'collided with' (nearby) is near a user do this
		float diffX = colliderObject->coordinates.x - collisionObject->coordinates.x;
		float diffY = colliderObject->coordinates.y - collisionObject->coordinates.y;
		if(sqrt(pow(diffX, 2) + pow(diffY, 2)) < 5){ //if the distance between the coordinates is less than 5 units count it as a 'collision' (being nearby, so can be picked up)
			return true;
		}
	}
    return false;
}