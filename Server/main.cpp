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
ecs::component::ecsComponentStructure<ecs::component::location> ecs::component::locationStructs;
ecs::component::ecsComponentStructure<ecs::component::drawable> ecs::component::drawables;
ecs::component::ecsComponentStructure<ecs::component::chunkData> ecs::component::chunkDataStructs;
ecs::entity::entityManager ecs::entity::superEntityManager;
std::unordered_map<ecs::system::coordinatesStruct, std::pair<int, std::vector<ecs::entity::entity>>, ecs::system::Hash> ecs::system::chunks;
std::unordered_map<ecs::system::coordinatesStruct, json, ecs::system::Hash> ecs::system::gameData;
std::unordered_map<std::string, unsigned int>  ecs::system::sessionIDToEntityID;

int main(){
	curl_global_init(CURL_GLOBAL_ALL); //initialise libcurl functionality globally

	ecs::system::systemsManager systems = ecs::system::systemsManager(5000); //will make a system manager object with the port set to be 5000
	systems.systemStart(); //this will launch the threads for system processes

	while(true){ //this is basically a convenience, will always await user input and run forever, it should be changed
		std::cin.get();
	}

	systems.systemEnd(); //stops threads and cleans up resources

	return 0;
}