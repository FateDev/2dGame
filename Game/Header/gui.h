#ifndef GUI_HEADER
#define GUI_HEADER

#include <network.h>
#include <TGUI/TGUI.hpp>

class launcher {
private:
	sf::Time notifTime; //the amount of time a notification has been on screen (should be in milliseconds)
	int loginError = 0; //0 = no error; 1 = error; 2 = error displayed, waiting to take it off
	
	sf::RenderWindow* window = NULL; //will hold the window

	//will hold the 3 main threads
	sf::Thread *pingThread = NULL;
	sf::Thread *receiveThread = NULL;
	sf::Thread *input = NULL;

	//makes these 3 tgui widgets here rather than in the constructor as I need these in other functions as well
	tgui::Label::Ptr unsuccessfulLabel = tgui::Label::create();
	tgui::Panel::Ptr loginUnsuccessful = tgui::Panel::create();
	tgui::Button::Ptr playButton = tgui::Button::create();

	networking *networkBit = NULL; //will hold the networking object

	sf::Clock *notifClock; //will hold the global clock

	void loginFunctionFromWindow(tgui::EditBox::Ptr usernameBox, tgui::EditBox::Ptr passwordBox); //function prototype to check login
	void helpFunction(); //function prototype to open the help page in browser
public:
	tgui::Gui *gui = NULL; //will be initialised later when the window object has been received, to hold the necessary widgets

	void liveUpdate(); //the function to do stuff during the main loop
	launcher(networking *networkObject, sf::RenderWindow* mainWindow, sf::Thread *ping, sf::Thread *receive, sf::Thread *inputThread, sf::Clock *globalClock);
	//the constructor gets the necessary data from outside and stores references inside the object for use whenever

	~launcher(); //the destructor, it deletes the gui which is made in the constructor
};

#endif // !GUI_HEADER
