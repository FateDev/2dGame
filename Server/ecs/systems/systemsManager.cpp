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
	processNetwork = new std::thread(&network::process, network::getInstance()); //makes the thread

	listenNetwork = new sf::Thread(&network::server, network::getInstance()); //launches the server to listen on that specific port
	listenNetwork->launch();														//launches it

	gameListen = new sf::Thread(&gameBroadcast::listenToUsers, gameBroadcast::getInstance());
	gameListen->launch();

	gameConnectServer = new sf::Thread(&gameBroadcast::server, gameBroadcast::getInstance());
	gameConnectServer->launch();

	mainGame = new std::thread(&game::runGame, game::getInstance());

	chunksStuff = new std::thread(&game::chunksUpdateLoop, game::getInstance());
	updateInRangeThread = new std::thread(&game::updateInRangeLoop, game::getInstance());
	damageThread = new std::thread(&game::damageLoop, game::getInstance());

	//below is just initialisation stuff
	std::ifstream itemsJSONFile("items.json");
	std::stringstream jsonContents;
	jsonContents << itemsJSONFile.rdbuf(); //reads into read buffer
	itemsFromFile = json::parse(jsonContents.str()); //puts into object
	
	updateActiveChunkData::getInstance()->initWorld(coordinatesStruct(0, 0)); //generates the first few chunks in the world
}

void systemsManager::systemEnd()
{
	if (processNetwork){//if a thread exists
		processNetwork->join(); //wait until it's completed this bit of code
		delete processNetwork;  //delete it, by the way as sever(...) has an infinite while loop inside, this is only really called when quitting the program
	}

	if (listenNetwork){//if a thread exists
		listenNetwork->wait(); //wait until it's completed this bit of code
		delete listenNetwork;  //delete it, by the way as sever(...) has an infinite while loop inside, this is only really called when quitting the program
	}

	if (gameListen){//if a thread exists
		gameListen->wait(); //wait until it's completed this bit of code
		delete gameListen;  //delete it, by the way as sever(...) has an infinite while loop inside, this is only really called when quitting the program
	}

	if (gameConnectServer){//if a thread exists
		gameConnectServer->wait(); //wait until it's completed this bit of code
		delete gameConnectServer;  //delete it, by the way as sever(...) has an infinite while loop inside, this is only really called when quitting the program
	}

	if (mainGame){//if a thread exists
		mainGame->join(); //wait until it's completed this bit of code
		delete mainGame;  //delete it, by the way as sever(...) has an infinite while loop inside, this is only really called when quitting the program
	}

	if (chunksStuff){
		chunksStuff->join();
		delete chunksStuff;
	}

	if (updateInRangeThread){
		updateInRangeThread->join();
		delete updateInRangeThread;
	}
	
	if(damageThread){
		damageThread->join();
		delete damageThread;
	}
}