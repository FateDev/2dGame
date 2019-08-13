#include <iostream>
#include <string>
#include <functional>

#include <SFML/Network.hpp>
#include <TGUI/TGUI.hpp>

//user defined header files
#include <network.h>
#include <gui.h>
#include <header.h>

//tgui lib selection
#ifdef _DEBUG
#	pragma comment(lib, "tgui-d.lib")
#else
#	pragma comment(lib, "tgui.lib")
#endif

int main() {
	sf::Clock* globalClock = new sf::Clock; //the clock which is used to check when to ping the server
	networking* networkObject = new networking("192.168.0.15", 5000, globalClock); //initialises the network object

	sf::Thread* receiveThread = new sf::Thread(&networking::getResponses, networkObject); //make the getResponses() function run on this thread
	sf::Thread* pingThread = new sf::Thread(&networking::stayAlive, networkObject); //makes the stayAlive() function run on this thread
	sf::Thread* getInputThread = new sf::Thread(&networking::getInput, networkObject); //thread for getting user input

	//launcher window bit, everything for the launcher is draw and the login is done and stuff
	if (launcherBit(networkObject, pingThread, receiveThread, getInputThread, globalClock)) {
		gameBit();
	}

	clearResources(networkObject, pingThread, receiveThread, getInputThread, globalClock);

	return 0;
}