#ifndef GAME_HEADER
#define GAME_HEADER

#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <json.hpp>

#include <network.h>

using namespace nlohmann;

class gameNetwork {
private:
	std::string sessionID = ""; //will contain the session ID for the UDP connection
	unsigned short localListenPort; //will be the local listen port, will be bound to by the UDP socket
	unsigned short sendPort; //the remote port 
	std::string remoteIP = "";
	sf::UdpSocket socket;
public:
	json responseData;
	void setData(std::string sessID, unsigned short lisPort, unsigned short senPort, std::string remIP) { sessionID = sessID; localListenPort = lisPort; sendPort = senPort; remoteIP = remIP; } //will just assign those values
	void sendData(json payload); //this will send the payload to the server, the server accepts key presses and such
	void listenData();
};

class game {
private:
	networking* networkObj = NULL;
	gameNetwork* gameNetworkObj = NULL;
	std::unordered_map<sf::Keyboard::Key, std::string> sfKeyToAbstractKeyMap;
	std::unordered_map<std::string, sf::Keyboard::Key> abstractKeyTosfKeyMap;
	bool changeInButtonState = false; //used to indicate whether or not any buttons have been pressed or released
public:
	json keysObject;
	game(networking* networkObject, gameNetwork* gameConnection);
	void listenForKeys(sf::Event event);
	void live(); //will do anything required in the main loop
};

#endif