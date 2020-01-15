#include "../../header/ecs.h"
#include "../../header/helper.h"

#include <cmath>
#include <functional>
#include <SFML/Network.hpp>
#include <curl/curl.h>
#include <chrono>
#include <fstream>
#include <sstream>

using namespace ecs::system;
using namespace ecs::component;

void systemsManager::systemStart()
{
	processNetwork = new sf::Thread(&network::process, network::getInstance()); //makes the thread
	processNetwork->launch();										 //launches it

	listenNetwork = new sf::Thread(&network::server, network::getInstance()); //launches the server to listen on that specific port
	listenNetwork->launch();														//launches it

	gameListen = new sf::Thread(&gameBroadcast::listenToUsers, gameBroadcast::getInstance());
	gameListen->launch();

	gameConnectServer = new sf::Thread(&gameBroadcast::server, gameBroadcast::getInstance());
	gameConnectServer->launch();

	mainGame = new sf::Thread(&game::runGame, game::getInstance());
	mainGame->launch();

	//below is just initialisation stuff
	std::ifstream itemsJSONFile("items.json");
	std::stringstream jsonContents;
	jsonContents << itemsJSONFile.rdbuf(); //reads into read buffer
	itemsFromFile = json::parse(jsonContents.str()); //puts into object

	//this just makes the first chunk, if any updates are made in the chunk gen section make sure to add them here
	coordinatesStruct startCoord(0, 0);
	chunks[startCoord].first.settingID = 1; //sets the first chunk's setting ID to 1, which is a city
	chunks[startCoord].first.permanent = true; //it should never be deleted
	unsigned int entityID = ecs::entity::superEntityManager.create({components::DRAWABLE, components::PHYSICAL}); //a new object with those attributes is made
	physicsObjects.compVec[physicsObjects.entityToVectorMap(entityID)].boxCorners = {
		sf::Vector2f(startCoord.coordinates.first * chunkPixelSize_x, (startCoord.coordinates.second * chunkPixelSize_y) - 5), 
		sf::Vector2f((startCoord.coordinates.first * chunkPixelSize_x) + chunkPixelSize_x, (startCoord.coordinates.second * chunkPixelSize_y)  - 5),
		sf::Vector2f(startCoord.coordinates.first * chunkPixelSize_x, (startCoord.coordinates.second * chunkPixelSize_y)), 
		sf::Vector2f((startCoord.coordinates.first * chunkPixelSize_x) + chunkPixelSize_x, (startCoord.coordinates.second * chunkPixelSize_y))
	};  //sets the corners of these boxes
	physicsObjects.compVec[physicsObjects.entityToVectorMap(entityID)].coordinates = sf::Vector2f(startCoord.coordinates.first * chunkPixelSize_x, startCoord.coordinates.second * chunkPixelSize_y);
	physicsObjects.compVec[physicsObjects.entityToVectorMap(entityID)].objType = COLLISION; //sets the object type
	chunks[startCoord].second.push_back(entityID); //pushes the floor entity to the chunks object
}

void systemsManager::systemEnd()
{
	if (processNetwork)
	{							//if a thread exists
		processNetwork->wait(); //wait until it's completed this bit of code
		delete processNetwork;  //delete it, by the way as sever(...) has an infinite while loop inside, this is only really called when quitting the program
	}

	if (listenNetwork)
	{						   //if a thread exists
		listenNetwork->wait(); //wait until it's completed this bit of code
		delete listenNetwork;  //delete it, by the way as sever(...) has an infinite while loop inside, this is only really called when quitting the program
	}
}