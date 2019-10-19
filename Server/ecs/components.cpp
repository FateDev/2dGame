#include "../header/ecs.h"
using namespace ecs::component;

template <class T>
void ecsComponentStructure<T>::addComponent(T component, unsigned int entityID){
    compVec.push_back(component);
    entityVectorMap.insert({entityID, compVec.size()-1});
}

template <class T>
void ecsComponentStructure<T>::removeComponent(unsigned int entityID){
    unsigned int index = entityVectorMap[entityID];
    if(entityVectorMap.count(entityID)){ //if the ID exists in the map
        if(compVec.size() - 1 == index){
            entityVectorMap.erase(entityID);
            compVec.pop_back();
        }else{
            unsigned int lastEntityID = 0;
            for (std::pair<unsigned int, unsigned int> entry : entityVectorMap){
                if(entry.second == (compVec.size() - 1)){
                    lastEntityID = entry.first;
                    break;
                }
            }

            compVec[index] = compVec[compVec.size() - 1];
            compVec.pop_back();

            entityVectorMap[lastEntityID] = index;
            entityVectorMap.erase(entityID);
        }
    }
}

template <class T>
unsigned int ecsComponentStructure<T>::entityToVectorMap(unsigned int entityID){
    return entityVectorMap[entityID];
}

template <class T>
unsigned int ecsComponentStructure<T>::vectorToEntityMap(unsigned int componentIndex){
    for (std::pair<unsigned int, unsigned int> entry : entityVectorMap){
        if(entry.second == componentIndex){
            return entry.first;
        }
    }
}

template class ecsComponentStructure<ecs::component::location>;
template class ecsComponentStructure<ecs::component::user>;