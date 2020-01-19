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