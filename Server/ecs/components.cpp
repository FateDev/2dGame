#include "../header/ecs.h"
using namespace ecs::component;

template <class T>
void ecsComponentStructure<T>::addComponent(T component, unsigned int entityID){
    compVec.push_back(component); //adds to the component vector
    entityVectorMap.insert(entityID, (compVec.size()-1)); //inserts an entry in the map, entityID as the key, the index in the component vector as the value
}

template <class T>
void ecsComponentStructure<T>::removeComponent(unsigned int entityID){
    unsigned int index = entityVectorMap.getBfromA(entityID);
    if(entityVectorMap.countInA(entityID)){ //if the ID exists in the map
        if(compVec.size() - 1 == index){ //if this is the last element in the vector
            entityVectorMap.eraseFromA(entityID); //remove the entry for the entity
            compVec.pop_back(); //and remove the final element
        }else{
            unsigned int lastEntityID = entityVectorMap.getAfromB(compVec.size() - 1);

            compVec[index] = compVec[compVec.size() - 1]; //replaces the value at the index of the element to remove with the values of the last element in the vector
            compVec.pop_back(); //removes the final entry in the component vector

            entityVectorMap.setBfromA(lastEntityID, index); //sets the component vector index of the final element to now be equal to the index that we wanted to remove, as we have made the value of the index equal to the final elements value
            entityVectorMap.eraseFromA(entityID); //and erase the entry for the entity who's component we removed
        }
    }
}

template <class T>
unsigned int ecsComponentStructure<T>::entityToVectorMap(unsigned int entityID){ //returns the component vector index associated with some entity ID
    return entityVectorMap.getBfromA(entityID);
}

template <class T>
unsigned int ecsComponentStructure<T>::vectorToEntityMap(unsigned int componentIndex){ //returns the entity ID that is associated with this component vector index
    return entityVectorMap.getAfromB(componentIndex); //uses my custom bimap class
}

//these are explicit instantiations for every type that this object will be used for, so it needs to be updated for any new components
template class ecsComponentStructure<ecs::component::location>;
template class ecsComponentStructure<ecs::component::user>;