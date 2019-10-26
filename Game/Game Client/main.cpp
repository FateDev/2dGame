#include <iostream>
#include <string>
#include <functional>
#include <SFML/Network.hpp>
#include <TGUI/TGUI.hpp>

//user defined header files
#include <network.h>
#include <gui.h>
#include <header.h>

//tgui lib selection, and libcurl lib selection
#ifdef _DEBUG
#	pragma comment(lib, "tgui-d.lib")
#else
#	pragma comment(lib, "tgui.lib")
#	pragma comment(lib, "libcurl_a.lib")
#endif

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

	if (launcherKey == "Hg+Lb9B6Q/zrMduYEvPmuQ==") { //really weak check to verify that the launcher opened the program
		sf::Clock* globalClock = new sf::Clock; //the clock which is used to check when to ping the server
		networking* networkObject = new networking("192.168.0.15", 5000, globalClock); //initialises the network object

		sf::Thread* receiveThread = new sf::Thread(&networking::getResponses, networkObject); //make the getResponses() function run on this thread
		sf::Thread* pingThread = new sf::Thread(&networking::stayAlive, networkObject); //makes the stayAlive() function run on this thread

		//launcher window bit, everything for the launcher is draw and the login is done and stuff
		if (networkObject->login(username)) {
			pingThread->launch(); //will maintain the connection by pinging every so often
			receiveThread->launch(); //will receive any server orders
			gameBit(globalClock, networkObject);
		}

		clearResources(networkObject, pingThread, receiveThread, globalClock); //deletes objects and stuff
	}

	return 0;
}