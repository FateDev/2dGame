#ifndef GAME_HEADER
#define GAME_HEADER

#include <iostream>
#include <atomic>
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
	bool exiting = false; //this will make it exit
	std::mutex exitMutex; //this is for exiting the program properly
	bool success = false;
	gameNetwork(std::string ip, unsigned short port, std::string sessionID);
	std::mutex drawMutex; //this is for drawing to the screen properly
	game* gameReference = 0; //reference to the game itself
	json responseData;
	void sendData(json payload); //this will send the payload to the server, the server accepts key presses and such
	void listenData();
};

class game {
private:
	sf::RenderWindow* gameWindow = NULL;
	sf::View* gameView = NULL;
	bool changeInButtonState = false; //used to indicate whether or not any buttons have been pressed or released
	sf::Font font;
public:
	json itemsFromFile; //items loaded from file
	std::unordered_map<std::string, sf::Texture> textures; //to avoid having to repeatedly load in textures
	std::unordered_map<sf::Keyboard::Key, std::string> sfKeyToAbstractKeyMap;
	std::unordered_map<std::string, sf::Keyboard::Key> abstractKeyTosfKeyMap;
	sf::Texture playerTexture; //temp
	sf::Texture opponentTexture; //temp

	stats* statsObj = NULL;
	networking* networkObj = NULL;
	gameNetwork* gameNetworkObj = NULL;
	
	std::vector<sf::RectangleShape> backgroundRectanglesToDraw;
	std::vector<sf::RectangleShape> rectanglesToDraw;
	std::vector<sf::RectangleShape> spriteAccessories;
	std::vector<sf::CircleShape> circlesToDraw;
	std::vector<sf::Text> textToDraw;
	std::vector<sf::Sprite> spritesToDraw;
	std::vector<sf::Texture> texturesToUse;

	json keysObject;
	json gameData;
	game(networking* networkObject, gameNetwork* gameConnection, sf::RenderWindow* window, sf::View* view);
	void listenForKeys(sf::Event event);
	void draw(); //will draw to the game screen
	void live(inventory* inventoryBit); //will do anything required in the main loop, takes inventory as a parameter because we need to stop detecting keypress while inventory is open
};

#endif