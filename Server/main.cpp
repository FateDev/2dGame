#include <iostream>
#include <vector>
#include <ctime>
#include <cmath>
#include <fstream>

#include "header/helper.h"
#include "header/ecs.h"

#include <SFML/Network.hpp>
#include <curl/curl.h>

//these are the definitions that the externs in ecs.h were referring to, this basically means that anywhere that ecs.h is included, the below variables are global
ecs::component::ecsComponentStructure<ecs::component::user> ecs::component::users;
ecs::component::ecsComponentStructure<ecs::component::drawable> ecs::component::drawables;
ecs::component::ecsComponentStructure<ecs::component::physical> ecs::component::physicsObjects;
ecs::component::ecsComponentStructure<ecs::component::mp_hp> ecs::component::mpHpObjects;
ecs::component::ecsComponentStructure<ecs::component::mob> ecs::component::mobs;
ecs::component::ecsComponentStructure<ecs::component::npc> ecs::component::npcs;
ecs::component::ecsComponentStructure<ecs::component::mission> ecs::component::missions;
ecs::component::ecsComponentStructure<ecs::component::thrown_item> ecs::component::thrown_items;
json ecs::system::itemsFromFile; //items loaded in from items.json
ecs::entity::entityManager ecs::entity::superEntityManager;
std::unordered_map<ecs::system::coordinatesStruct, std::pair<ecs::system::chunkData, std::vector<ecs::entity::entity>>, ecs::system::Hash> ecs::system::chunks;
std::unordered_map<ecs::system::coordinatesStruct, json, ecs::system::Hash> ecs::system::gameData;
std::unordered_map<std::string, unsigned int>  ecs::system::sessionIDToEntityID;
std::unordered_map<unsigned int, std::unordered_set<unsigned int>> ecs::system::entitiesInRange; //maps entity id to a set of other ids
std::unordered_map<int, json> ecs::system::userInventories;

int main(){
	curl_global_init(CURL_GLOBAL_ALL); //initialise libcurl functionality globally
	srand(time(NULL)); //pseudo random number generator seed

	ecs::system::systemsManager systems = ecs::system::systemsManager(5000); //will make a system manager object with the port set to be 5000
	systems.systemStart(); //this will launch the threads for system processes

	while(true){ //this is basically a convenience, will always await user input and run forever, it should be changed
		std::cin.get();
	}

	systems.systemEnd(); //stops threads and cleans up resources

	return 0;
}