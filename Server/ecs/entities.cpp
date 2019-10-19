#include "../header/ecs.h"
using namespace ecs::entity;

unsigned int entityManager::create(std::initializer_list<ecs::component::components> initialiseWithStructs){
    nextEntity.id = 0;
    while(alive(nextEntity)){
        nextEntity.id++;
    }

    entities.insert(nextEntity);

    for(int i = 0;i < initialiseWithStructs.size(); i++){
        if(*(initialiseWithStructs.begin()+i) == ecs::component::components::USER){
            ecs::component::user userTemp;
            ecs::component::users.addComponent(userTemp, nextEntity.id);
        }
        
        if(*(initialiseWithStructs.begin()+i) == ecs::component::components::LOCATION){
            ecs::component::location location;
            ecs::component::locationStructs.addComponent(location, nextEntity.id);
        }
    }
    
    return nextEntity.id;
}

bool entityManager::alive(entity entityStruct){
    if(entities.find(entityStruct) != entities.end()){
        return true;
    }else{
        return false;
    }
}

void entityManager::destroy(unsigned int entityID){
    ecs::component::users.removeComponent(entityID);
    ecs::component::locationStructs.removeComponent(entityID);
    //add in any other component objects
    nextEntity.id = entityID;

    entities.erase(nextEntity);

    nextEntity.id = 0;
}