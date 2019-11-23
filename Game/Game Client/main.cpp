#include <iostream>
#include <string>
#include <functional>
#include <thread>
#include <SFML/Network.hpp>
#include <TGUI/TGUI.hpp>
#include <json.hpp>

//user defined header files
#include <network.h>
#include <gui.h>
#include <header.h>
#include <game.h>

//tgui lib selection, and libcurl lib selection
#ifdef _DEBUG
#	pragma comment(lib, "tgui-d.lib")
#else
#	pragma comment(lib, "tgui.lib")
#	pragma comment(lib, "libcurl_a.lib")
#endif

const unsigned short REMOTE_TCP_PORT = 5000;
const unsigned short LOCAL_GAME_PORT = 5001;
const std::string REMOTE_IP = "192.168.0.15";
const std::string WEBSITE_URL = "http://erewhon.xyz";

int main(int argc, char **argv) { //will accept parameters from command line, the launcher will send parameters using this
	SetCurrentDirectory(L"Game"); //sets the current working directory, so it doesn't conflict with the stuff in the parent directory

	//extracts the variables from the parameters passed from the launcher
	/*
	std::string launcherKey(argv[1]);
	std::string username(argv[2]);
	*/
	///>>>below are set temporarily for debugging and development
	std::string launcherKey = "Hg+Lb9B6Q/zrMduYEvPmuQ==";
	std::string username = "test";

	//std::thread udpSendThread(testSocket);
	//std::thread udpReceiveThread(testListen);

	if (launcherKey == "Hg+Lb9B6Q/zrMduYEvPmuQ==") { //really weak check to verify that the launcher opened the program
		sf::Clock* globalClock = new sf::Clock; //the clock which is used to check when to ping the server
		networking* networkObject = new networking(REMOTE_IP, REMOTE_TCP_PORT, WEBSITE_URL, globalClock); //initialises the network object

		if (networkObject->login(username)) { //actually do the login procedure to get the sessionID
			gameNetwork* gameNetworkObj = new gameNetwork(REMOTE_IP, LOCAL_GAME_PORT, networkObject->sessionID);

			if (gameNetworkObj->success) { //if it successfully connected to the game TCP server
				sf::Thread* receiveThread = new sf::Thread(&networking::getResponses, networkObject); //make the getResponses() function run on this thread
				sf::Thread* pingThread = new sf::Thread(&networking::stayAlive, networkObject); //makes the stayAlive() function run on this thread
				sf::Thread* udpGameThread = new sf::Thread(&gameNetwork::listenData, gameNetworkObj); //this will listen for incoming data

				//launcher window bit, everything for the launcher is draw and the login is done and stuff
				if (networkObject->login(username)) {
					udpGameThread->launch();
					pingThread->launch(); //will maintain the connection by pinging every so often
					receiveThread->launch(); //will receive any server orders
					gameBit(globalClock, networkObject, gameNetworkObj);
				}
			}
		}

		//clearResources(networkObject, pingThread, receiveThread, globalClock); //deletes objects and stuff
	}

	return 0;
}