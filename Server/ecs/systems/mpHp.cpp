#include "../../header/ecs.h"
using namespace ecs::system;
using namespace ecs::component;

void mpHpSystem::damage(){
    mutexs::attackVectorMutex.lock(); //need to lock it as we add attacks for processing from another thread (physics.cpp)

    std::vector<int> entitiesKilled = {};

    for(auto &entityIDdamagePair : attacks){ //the pair is the entity which initiated the attack, and the strength of the attack
        for(auto &entityID : entitiesInRange[entityIDdamagePair.first]){ //loops through all entities in range of this entity, and applies damage to them
            int compVecIndex = mpHpObjects.entityToVectorMap(entityID);
            if(compVecIndex != -1){ //if they have an mpHpObject entry do further processing
                if(entity::superEntityManager.getType(entity::entity(entityID)) == ecs::entity::MOB){ //if they are a mob specifically do this bit
                    mpHpObjects.compVec[mpHpObjects.entityToVectorMap(entityID)].hp -= entityIDdamagePair.second;

                    mobSystem::getInstance()->targetEntity(entityID, entityIDdamagePair.first); //first param is the entity which will target the other entity, and the second parameter is the other entity

                    if(mpHpObjects.compVec[mpHpObjects.entityToVectorMap(entityID)].hp < 0){
                        mobSystem::getInstance()->targetEntity(entityID, -1); //the entiy they are targetting is now dead, so set their target to -1
                        entitiesKilled.push_back(entityID);
                    }
                }
                else {
                    mpHpObjects.compVec[mpHpObjects.entityToVectorMap(entityID)].hp -= entityIDdamagePair.second;
                    if (mpHpObjects.compVec[mpHpObjects.entityToVectorMap(entityID)].hp < 0) {
                        entitiesKilled.push_back(entityID);
                    }
                }
            }
        }
    }

    for(auto &entityID : entitiesKilled){
        if (physicsObjects.entityToVectorMap(entityID) != -1) {
            sf::Vector2f coordinates = physicsObjects.compVec[physicsObjects.entityToVectorMap(entityID)].coordinates;
            coordinatesStruct chunkCoord(chunkCoordHelperX(coordinates.x, chunkPixelSize_x), chunkCoordHelperY(coordinates.y, chunkPixelSize_y));

            if (entity::superEntityManager.getType(entityID) == ecs::entity::MOB) { //if it's a mob, do these things
                auto dropItems = mobs.compVec[mobs.entityToVectorMap(entityID)].dropItems;

                //drop up to 3 items, no velocity
                itemSystem::getInstance()->dropItem(coordinates, dropItems.x, { 0, 0 });
                itemSystem::getInstance()->dropItem(coordinates, dropItems.y, { 0, 0 });
                itemSystem::getInstance()->dropItem(coordinates, dropItems.z, { 0, 0 });

                for (int i = 0; i < chunks[chunkCoord].second.size(); i++) {
                    if (chunks[chunkCoord].second[i].id == entityID) {
                        chunks[chunkCoord].second.erase(chunks[chunkCoord].second.begin() + i); //erases the entity from the vector
                        break;
                    }
                }

                chunks[chunkCoord].first.mobCount--; //decrement the mob count because we just killed one
                entity::superEntityManager.destroy(entity::entity(entityID)); //destroys the entity
            }
            else if (entity::superEntityManager.getType(entityID) == ecs::entity::USER) {
                for (int i = 0; i < chunks[chunkCoord].second.size(); i++) {
                    if (chunks[chunkCoord].second[i].id == entityID) {
                        chunks[chunkCoord].second.erase(chunks[chunkCoord].second.begin() + i); //erases the entity from the vector
                        break;
                    }
                }

                chunks[coordinatesStruct(0, 0)].second.push_back(entity::entity(entityID));
                physicsObjects.compVec[physicsObjects.entityToVectorMap(entityID)].coordinates = { 0, -5 };
                auto* userMpHpObj = &mpHpObjects.compVec[mpHpObjects.entityToVectorMap(entityID)];
                userMpHpObj->hp = userMpHpObj->max_hp;
                userMpHpObj->mp = userMpHpObj->max_mp;

                updateActiveChunkData::getInstance()->updateActiveChunks();
            }
        }
    }

    entitiesKilled.clear();
    attacks.clear();

    mutexs::attackVectorMutex.unlock();
}