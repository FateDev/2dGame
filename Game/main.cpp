#include <iostream>
#include <string>
#include "SFML/Network.hpp"
#include <functional>

//user defined header files
#include <network.h>

sf::Clock keepAliveTimer; //the clock which is used to check when to ping the server
sf::Time pingTime; //the time at which a ping should occur
sf::TcpSocket *socket = new sf::TcpSocket; //makes a new socket
std::string username; //the name of this user
std::string password; //password
bool loggedIn = false; //whether or not the user successfully logged in

int main() {
	const std::string IPADDRESS("192.168.0.15"); //the IP address to connect to
	const unsigned short PORT = 5000; //the port to connect to

	std::string msg; //the string used in getInput(...)

	sf::Thread* pingThread = 0; //thread for pinging the server
	sf::Thread* receiveThread = 0; //thread for waiting for incoming messages

	loginLoop(IPADDRESS, PORT, username, password, socket, loggedIn); //this will ask the user to login over and over

	receiveThread = new sf::Thread(&getResponses, socket); //make the getResponses() function run on this thread
	receiveThread->launch(); //launches the thread

	pingThread = new sf::Thread(std::bind(&stayAlive, keepAliveTimer, pingTime, socket)); //makes the stayAlive() function run on this thread
	pingThread->launch(); //launches the thread

	getInput(msg, loggedIn, username, socket); //runs the getInput(...) function endlessly

	if (pingThread) { //if the ping thread exists...
		pingThread->wait(); //wait for it's current bit of code to finish
		delete pingThread; //then delete it
	}

	if (receiveThread) { //if the receiveThread exists...
		receiveThread->wait(); //wait for it's current bit of code to finish
		delete receiveThread; //then delete it
	}

	delete socket;

	return 0;
}