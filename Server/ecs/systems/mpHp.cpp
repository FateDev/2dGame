#include "../../header/ecs.h"
using namespace ecs::system;
using namespace ecs::component;

void mpHpSystem::damage(){
    mutexs::attackVectorMutex.lock();

    std::vector<int> entitiesKilled;

    for(auto &entityIDdamagePair : attacks){
        for(auto &entityID : entitiesInRange[entityIDdamagePair.first]){
            int compVecIndex = mpHpObjects.entityToVectorMap(entityID);
            if(compVecIndex != -1){
                if(entity::superEntityManager.getType(entity::entity(entityID)) == ecs::entity::MOB){
                    mpHpObjects.compVec[mpHpObjects.entityToVectorMap(entityID)].hp -= entityIDdamagePair.second;
                    if(mpHpObjects.compVec[mpHpObjects.entityToVectorMap(entityID)].hp < 0){
                        mobSystem::getInstance()->dropItems(entityID);
                        entitiesKilled.push_back(entityID);
                    }
                }
            }
        }
    }

    for(auto &entityID : entitiesKilled){
        entity::superEntityManager.destroy(entity::entity(entityID)); //destroys the entity
        //coordinatesStruct chunkCoord()
    }

    entitiesKilled.clear();
    attacks.clear();

    mutexs::attackVectorMutex.unlock();
}