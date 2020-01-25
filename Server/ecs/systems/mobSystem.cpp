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
        mobPhysical->coordinates = {(float(coordinate.coordinates.first * chunkPixelSize_x) + chunkPixelSize_x/4 + (rand() % (chunkPixelSize_x/2))), float((coordinate.coordinates.second * chunkPixelSize_y) + 2)};
        
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
    if (mobs.compVec[mobs.entityToVectorMap(entityID)].targetEntity == -1) { //if it's not targetting anything, just randomly move around
        int chance = (rand() % 100 > 95) ? 1 : 0; //if the number is greater than 95 put 1, else 0 - 5% chance of changing movement
        if (chance) {
            int selection = rand() % 3; //so 0, 1 or 2 - 0 is stop moving, 1 is move left, 2 is move right
            if (selection == 0) {
                physicsObjects.compVec[physicsObjects.entityToVectorMap(entityID)].velocity.x = 0;
            }
            else if (selection == 1) {
                physicsObjects.compVec[physicsObjects.entityToVectorMap(entityID)].velocity.x = -1;
                drawables.compVec[drawables.entityToVectorMap(entityID)].direction.x = -1;
            }
            else if (selection == 2) {
                physicsObjects.compVec[physicsObjects.entityToVectorMap(entityID)].velocity.x = 1;
                drawables.compVec[drawables.entityToVectorMap(entityID)].direction.x = 1;
            }
        }
    }
    else {
        moveTowardsTarget(entityID); //otherwise move towards the target
    }
}

bool mobSystem::mobMovementRestrictions(unsigned int entityID, coordinatesStruct newChunkCoords){
    bool cityDetected = false;
    physical* physicalStruct = &physicsObjects.compVec[physicsObjects.entityToVectorMap(entityID)];
    drawable* drawableStruct = &drawables.compVec[drawables.entityToVectorMap(entityID)];
    if(chunks[newChunkCoords].first.settingID == 4 || chunks[newChunkCoords].first.settingID == 5){
        if(chunks[coordinatesStruct(newChunkCoords.coordinates.first, newChunkCoords.coordinates.second+1)].first.settingID == 1){
            physicalStruct->coordinates.x -= physicalStruct->velocity.x; //update the x position
            physicalStruct->coordinates.y -= physicalStruct->velocity.y; //update the y position
            physicalStruct->velocity.x *= -1; //reverses direction
            drawableStruct->direction *= -1; //reverses direction for drawing too
            cityDetected = true;
        }
    } else if (chunks[newChunkCoords].first.settingID == 1){
        physicalStruct->coordinates.x -= physicalStruct->velocity.x; //update the x position
        physicalStruct->coordinates.y -= physicalStruct->velocity.y; //update the y position
        physicalStruct->velocity.x *= -1; //reverses direction
        drawableStruct->direction *= -1; //reverses direction for drawing too
        cityDetected = true;
    }
    return cityDetected;
}

void mobSystem::targetEntity(int fromEntityID, int targetEntityID){
    mobs.compVec[mobs.entityToVectorMap(fromEntityID)].targetEntity = targetEntityID;
}

void mobSystem::moveTowardsTarget(int mobEntityID) {
    auto *mobMobsObj = &mobs.compVec[mobs.entityToVectorMap(mobEntityID)];

    if (!entity::superEntityManager.alive(mobMobsObj->targetEntity)) { //if the entity that they are targetting is no longer active, set their target to -1
        mobMobsObj->targetEntity = -1;
    }else {
        auto* mobPhysicsObj = &physicsObjects.compVec[physicsObjects.entityToVectorMap(mobEntityID)];
        auto* mobDrawingObj = &drawables.compVec[drawables.entityToVectorMap(mobEntityID)];
        sf::Vector2f targetCoordinates = physicsObjects.compVec[physicsObjects.entityToVectorMap(mobMobsObj->targetEntity)].coordinates;
        if (targetCoordinates.x < mobPhysicsObj->coordinates.x) { //if the target is about to the left of the mob
            mobPhysicsObj->velocity.x = -1;
            mobDrawingObj->direction.x = -1;
        }else if(targetCoordinates.x > mobPhysicsObj->coordinates.x) {
            mobPhysicsObj->velocity.x = 1;
            mobDrawingObj->direction.x = 1;
        }
        else { //this pretty much only happens when 2 mobs are trying to attac each other
            mobPhysicsObj->velocity.x = 0;
            mobDrawingObj->direction.x = 1;
        }

        if (findDistance(targetCoordinates, mobPhysicsObj->coordinates) < 5 && mobMobsObj->nextAttackTime < clock.getElapsedTime().asMilliseconds()) { //if the entity is in range, and the mob isn't cooling down from an attack, attack
            mutexs::attackVectorMutex.lock();
            attacks.push_back({ mobEntityID, 5 }); //for now just setting damage as 5
            mutexs::attackVectorMutex.unlock();
            mobMobsObj->nextAttackTime = clock.getElapsedTime().asMilliseconds() + 500;
        }
    }
}