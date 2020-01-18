#include "../../header/ecs.h"
#include "../../header/helper.h"

#include <cmath>

using namespace ecs::system;
using namespace ecs::component;

void mobSystem::mobGeneration(){
    for(auto &chunk : chunks){ //loops through every chunk
        if(chunk.second.first.mobCount < maxMobsPerChunk){
            int entityID = ecs::entity::superEntityManager.create(ecs::entity::MOB);
            physicsObjects.compVec[physicsObjects.entityToVectorMap(entityID)].boxCorners = {sf::Vector2f(0, -7.5), sf::Vector2f(6, -7.5), sf::Vector2f(0, 0), sf::Vector2f(6, 0)}; //same box as users
            mob* newMob = &mobs.compVec[mobs.entityToVectorMap(entityID)];

            newMob->attackDamage = int((rand() % 25));
            newMob->mob_type = mobType((rand() % mobType::FINAL_ENUM_MOBTYPE)); //gets a random mob type, make sure you don't assign values to the enums yourself else this breaks
            //the below sets the drop items as random items
            newMob->dropItems.x = itemsFromFile[int(rand() % itemsFromFile.size())]["itemID"].get<int>();
            newMob->dropItems.y = itemsFromFile[int(rand() % itemsFromFile.size())]["itemID"].get<int>();
            newMob->dropItems.z = itemsFromFile[int(rand() % itemsFromFile.size())]["itemID"].get<int>();

            chunk.second.second.push_back(entity::entity(entityID)); //puts the mob into the chunk

            chunk.second.first.mobCount++;
        }
    }
}