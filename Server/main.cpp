#include <iostream>
#include <vector>
#include <ctime>
#include <cmath>
#include <fstream>

#include "header/helper.h"
#include "header/ecs.h"

#include <SFML/Network.hpp>
#include <curl/curl.h>

ecs::component::ecsComponentStructure<ecs::component::user> ecs::component::users;
ecs::component::ecsComponentStructure<ecs::component::location> ecs::component::locationStructs;
ecs::entity::entityManager ecs::entity::superEntityManager;

int main(){
	curl_global_init(CURL_GLOBAL_ALL); //initialise libcurl functionality globally

	ecs::system::systemsManager systems = ecs::system::systemsManager(5000);
	systems.systemStart();

	while(true){
		std::cin.get();
	}

	return 0;
}