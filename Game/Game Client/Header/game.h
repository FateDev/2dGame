#ifndef GAME_HEADER
#define GAME_HEADER

#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <json.hpp>
#include <mutex>

#include <network.h>

using namespace nlohmann;

class game; //forward declaration of the game class, as it's used in the gameNetwork class

class gameNetwork {
private:
	std::string remoteIP = ""; 
	sf::TcpSocket socket;
public:
	bool success = false;
	gameNetwork(std::string ip, unsigned short port, std::string sessionID);
	std::mutex jsonMutex;
	game* gameReference = 0; //reference to the game itself
	json responseData;
	void sendData(json payload); //this will send the payload to the server, the server accepts key presses and such
	void listenData();
};

class game {
private:
	networking* networkObj = NULL;
	sf::RenderWindow* gameWindow = NULL;
	sf::View* gameView = NULL;
	std::unordered_map<sf::Keyboard::Key, std::string> sfKeyToAbstractKeyMap;
	std::unordered_map<std::string, sf::Keyboard::Key> abstractKeyTosfKeyMap;
	bool changeInButtonState = false; //used to indicate whether or not any buttons have been pressed or released
public:
	gameNetwork* gameNetworkObj = NULL;
	json keysObject;
	json gameData;
	game(networking* networkObject, gameNetwork* gameConnection, sf::RenderWindow* window, sf::View* view);
	void listenForKeys(sf::Event event);
	void draw(); //will draw to the game screen
	void live(); //will do anything required in the main loop
};

#endif