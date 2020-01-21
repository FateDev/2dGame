#include "../../header/ecs.h"
#include "../../header/helper.h"

#include <cmath>

using namespace ecs::system;
using namespace ecs::component;

void mobSystem::generateMobsAt(coordinatesStruct coordinate){
    int mobGenNumber = rand() % (maxMobsPerChunk+1); //between 0 and maxMobsPerChunk mobs are generated for a new chunk

    for(int i = 0; i <= mobGenNumber; i++){
        int entityID = ecs::entity::superEntityManager.create(ecs::entity::MOB);
        physical* mobPhysical = &physicsObjects.compVec[physicsObjects.entityToVectorMap(entityID)];
        mobPhysical->objType = ecs::component::objectType::COLLIDER;
        mobPhysical->boxCorners = {sf::Vector2f(0, -7.5), sf::Vector2f(6, -7.5), sf::Vector2f(0, 0), sf::Vector2f(6, 0)}; //same box as users
        mobPhysical->coordinates = {(coordinate.coordinates.first * chunkPixelSize_x) + chunkPixelSize_x/4 + (rand() % (chunkPixelSize_x/2)), (coordinate.coordinates.second * chunkPixelSize_y) + 2};
        
        mob* newMob = &mobs.compVec[mobs.entityToVectorMap(entityID)];

        newMob->attackDamage = int((rand() % 25));
        newMob->mob_type = mobType((rand() % mobType::FINAL_ENUM_MOBTYPE)); //gets a random mob type, make sure you don't assign values to the enums yourself else this breaks
        //the below sets the drop items as random items
        newMob->dropItems.x = itemsFromFile[int(rand() % itemsFromFile.size())]["itemID"].get<int>();
        newMob->dropItems.y = itemsFromFile[int(rand() % itemsFromFile.size())]["itemID"].get<int>();
        newMob->dropItems.z = itemsFromFile[int(rand() % itemsFromFile.size())]["itemID"].get<int>();

        chunks[coordinate].second.push_back(entity::entity(entityID)); //puts the mob into the chunk
        chunks[coordinate].first.mobCount++; //increments that chunks mob count
    }
}

void mobSystem::mobMovement(int entityID){
    int chance = (rand() % 100 > 95) ? 1 : 0; //if the number is greater than 95 put 1, else 0 - 5% chance of changing movement
    if(chance){
        int selection = rand() % 3; //so 0, 1 or 2 - 0 is stop moving, 1 is move left, 2 is move right
        if(selection == 0){
            physicsObjects.compVec[physicsObjects.entityToVectorMap(entityID)].velocity.x = 0;
        }else if(selection == 1){
            physicsObjects.compVec[physicsObjects.entityToVectorMap(entityID)].velocity.x = -1;
            drawables.compVec[drawables.entityToVectorMap(entityID)].direction.x = -1;
        }else if(selection == 2){
            physicsObjects.compVec[physicsObjects.entityToVectorMap(entityID)].velocity.x = 1;
            drawables.compVec[drawables.entityToVectorMap(entityID)].direction.x = 1;
        }
    }
}