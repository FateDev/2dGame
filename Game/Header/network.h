#ifndef NETWORK_HEADER
#define NETWORK_HEADER

#include <iostream>
#include <string>
#include "SFML/Network.hpp"

class networking {
private:
	std::string usernameReal = ""; //this global variable would store the username
	std::string msg;

	sf::Clock* keepAliveTimer = NULL;
	sf::Time pingTime;
public:
	bool active = false; //this single global variable allows us to indicate whether or not the server connection is active
	//we make the above public so that we can see in the GUI bit whether or not there is a connection

	sf::TcpSocket *socket = new sf::TcpSocket; //makes a new socket
	std::string ip; //the IP address to connect to
	unsigned short port; //the port to connect to

	bool login(std::string username, std::string password);
	//this will loop repeatedly until either the user crosses it off or gets the correct login details

	void getInput();
	//once the user has got the correct login details, this is run on the main thread, so that they can type, and then send messages by pressing enter

	void getResponses();
	//this will get any responses from the server, decode them, and output them, and it's run on a separate thread

	void stayAlive();
	//this will ping the server periodically so that the server knows that this client is still acutally connected, as there's no other real way of checking, and it's run on another thread

	networking(std::string IPADDRESS, int PORT, sf::Clock * globalClock) : ip(IPADDRESS), port(PORT), keepAliveTimer(globalClock) {};
	//the constructor to store all the necessary data and references to start the networking bit

	~networking(); //destructor to delete socket
};

#endif