#ifndef NETWORK_HEADER
#define NETWORK_HEADER

#include <iostream>
#include <string>
#include "gui.h"
#include <curl/curl.h>
#include <SFML/Network.hpp>
#include <json.hpp>
#include <mutex>

using json = nlohmann::json;

class networking {
private:
	std::string msg;
	sf::Clock* keepAliveTimer = NULL;
	sf::Time pingTime;
	friend size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp); //friend function to get data from curl, this is forward declaring the function as we can't include the header file here, as this file is included in it
public:
	chat* chatBoxObject = NULL;
	inventory* inventoryObject = NULL;
	std::mutex messagesMutex; //will lock for retrieving messages
	sf::TcpSocket* socket = NULL; //makes a new socket
	std::string websiteUrl = ""; //the website url will be stored here
	std::string roomGuild = "main.alpha"; //public because we need to update this a lot, stores current room/guild (this is the one you start in)
	std::string usernameReal = ""; //this global variable would store the username
	std::string sessionID = ""; //will hold the session ID for the UDP socket
	std::string ip; //the IP address to connect to
	unsigned short port; //the port to connect to
	unsigned short userID; //will hold the user ID
	bool chatBoxActive = false; //is the small chat box active or not
	bool active = false; //this single global variable allows us to indicate whether or not the server connection is active
	bool msgBoxFocused = false; //this is used to indicate whether or not the message box is focused (used to decide if to send server keypress information) (we make this public so that we can see in the GUI bit whether or not there is a connection)
	json settings; //will contain the user defined settings
	json messages; //messages from DB container

	//this will update the internal messages buffer
	void getMessagesFromDB(); 
	//this will loop repeatedly until either the user crosses it off or gets the correct login details
	bool login(std::string username);
	//sends any message passed to it, to the server
	void sendMessage(std::string msg);
	//this will get any responses from the server, decode them, and output them, and it's run on a separate thread
	void getResponses();
	//this will ping the server periodically so that the server knows that this client is still acutally connected, as there's no other real way of checking, and it's run on another thread
	void stayAlive();
	//the constructor to store all the necessary data and references to start the networking bit
	networking(std::string IPADDRESS, int PORT, std::string WEBSITE_URL, sf::Clock * globalClock) : ip(IPADDRESS), port(PORT), websiteUrl(WEBSITE_URL), keepAliveTimer(globalClock) {};
	//will get the user's inventory from the server
	void getUserInventory();
	//destructor to delete socket
	~networking();
};

#endif