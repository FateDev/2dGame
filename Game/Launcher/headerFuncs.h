#ifndef HEADER
#define HEADER

#include <SFML/Window.hpp>
#include <SFML/Network.hpp>
#include <TGUI/TGUI.hpp>
#include <iostream>
#include <vector>
#include <fstream>

//function prototypes
void helpFunction();
void GETftpFile(std::string fileLocation, std::string remoteFileURL);
void GETresponse(std::string &readBuffer, std::string urlAndParams);
void delimitString(std::string delimiter, std::string stringToSplit, std::vector<std::string> &outputTokens);
bool doesFileExist(std::string fileName);

class networking {
private:
	std::string ip; //the IP address to connect to
	unsigned short port; //the port to connect to

	bool updateFiles();
	void getChecksumTokens(std::vector<std::pair<std::string, std::string>> &tokens, bool local, std::string location);
	void compareTokenList(std::vector<std::pair<std::string, std::string>> &localTokens, std::vector<std::pair<std::string, std::string>> &serverTokens, std::vector<std::pair<std::string, std::string>> &returnTokens, std::vector<std::pair<std::string, std::string>> &deleteTokens);
	void parseReturnTokens(std::vector<std::pair<std::string, std::string>> &returnTokens);
	void parseDeleteTokens(std::vector<std::pair<std::string, std::string>> &parseDeleteTokens);
public:
	std::string loggedinToken;

	bool loggedIn = false; //this will close the window when true

	std::string confirmedUsername = ""; //this is the username to pass on as a shell argument

	bool login(std::string username, std::string password);
	//this will loop repeatedly until either the user crosses it off or gets the correct login details

	networking(std::string IPADDRESS, int PORT) : ip(IPADDRESS), port(PORT) {};
	//the constructor to store all the necessary data and references to start the networking bit

	~networking(); //destructor to delete socket
};

class launcher { //this would be the 'login' window, the launcher
private:
	sf::Time notifTime; //the amount of time a notification has been on screen (should be in milliseconds)
	int loginError = 0; //0 = no error; 1 = error; 2 = error displayed, waiting to take it off

	sf::RenderWindow* window = NULL; //will hold the window

	//makes these 3 tgui widgets here rather than in the constructor as I need these in other functions as well
	tgui::Label::Ptr unsuccessfulLabel = tgui::Label::create();
	tgui::Panel::Ptr loginUnsuccessful = tgui::Panel::create();
	tgui::Button::Ptr playButton = tgui::Button::create();

	networking *networkBit = NULL; //will hold the networking object

	sf::Clock *notifClock; //will hold the global clock

	void loginFunctionFromWindow(tgui::EditBox::Ptr usernameBox, tgui::EditBox::Ptr passwordBox); //function prototype to check login
	friend void helpFunction(); //function prototype, for function to open the help page in the default browser
public:
	tgui::Gui *gui = NULL; //will be initialised later when the window object has been received, to hold the necessary widgets

	void liveUpdate(); //the function to do stuff during the main loop
	launcher(networking *networkObject, sf::RenderWindow* mainWindow, sf::Clock* globalClock);
	//the constructor gets the necessary data from outside and stores references inside the object for use whenever

	~launcher(); //the destructor, it deletes the gui which is made in the constructor
};

#endif // !HEADER
