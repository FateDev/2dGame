#ifndef GUI_HEADER
#define GUI_HEADER

//#include "network.h"
#include <SFML/Window.hpp>
#include <TGUI/TGUI.hpp>
#include <json.hpp>

using json = nlohmann::json;

class networking; //forward declaration of the networking class, so when we use it here, the compiler knows it exists
//this is so we don't incude network.h in gui.h, and vice versa which would be recursively including files

class loadingScreen { //this is for the loading screen
private:
	sf::Clock* loadingScreenClock = NULL; //need the clock for the loading animation
	sf::RenderWindow *gameWindow = NULL; //holds window
	tgui::Gui *gui = NULL; //holds gui
	sf::CircleShape *shape; //holds shape
public:
	loadingScreen(sf::RenderWindow *window, sf::Clock *clock); //initialises the loading screen
	void liveUpdate(); //the function to do stuff during the main loop
	~loadingScreen(); //called when the loading screen is deleted
};

class chat { //this would be the chat box on the main screen (maybe on other screens too later)
private:
	tgui::Panel::Ptr msgMaker(int time, std::string usernameText, std::string messageText, std::string imgLocation, bool continueFromLast);
	//function above is one that returns a tgui::Panel object with time, message, image (if specified) and username formatted nicely in a box thing

	tgui::EditBox::Ptr enterMessage = tgui::EditBox::create(); //where you enter a message
	tgui::ScrollablePanel::Ptr chatBox = tgui::ScrollablePanel::create(); //a tgui::Panel but has a scrollbar that appears if stuff overflows

	float percentWidth = 0; //the user defined maximum width of the chatbox
	float percentHeight = 0; //user defined maximum height of the chatbox
	float currentMaxHeight = 0; 
	//above is internal float to get the lowest y coord currently being used, so that we can give this to new messages for their y coord, 
	//and to jump down to this value for new messages

	int lastMsgTime = 0; //this is used internally with SFML to check if it has been 1 second since the user has messages (to prevent spam_

	std::string lastUsername = ""; //the username of the person who sent the last message
	int lastTimeInSeconds = 0; //the time of the last message

public:
	void addMessages(int time, std::string usernameText, std::string messageText, std::string imgLocation, int msgID); //this will directly allow for messages to be added to the chat box

	void flushMessages(); //removes all messages from the container

	tgui::ChildWindow::Ptr chatBoxContainer = tgui::ChildWindow::create("Chat"); //this would be the final container holding the chat box (a draggable window from tgui)
	chat(float percentWidth, float percentHeight, float posPercentX, float posPercentY); //the constructor, first 2 are the size of the box, last 2 are the starting position
	void liveUpdate(networking* networkObject, sf::Clock* clock); 
	//the above would send whatever message a user enters when they press enter (uses network object to send through the network obviously)
};

class mainScreen { //this is the first screen to be displayed after the loading screen disappears
private:
	std::vector<tgui::BitmapButton::Ptr> smallInventoryButtons; //the buttons on the top left for the small inventory task bar thing or whatever

	tgui::Group::Ptr mainScreenGroup = tgui::Group::create({ sf::VideoMode::getDesktopMode().width , sf::VideoMode::getDesktopMode().height });
	//the above would be an invisible container from tgui which holds everything that gets drawn for this screen

	sf::RenderWindow *window = NULL; //would hold the window object
	networking* networkObject = NULL; //this will hold the network object for use in the chat
	friend void chatBoxBulkAdd(networking* networkObject, chat* chatBox); //this will parse messages in the network object's buffer and add them
public:
	chat* chatBox = NULL; //this will hold the chat box

	bool active = false; //we can use this to decide whether or not we should have the liveUpdate() function execute
	void setActive(bool active); //this would make the above boolean active, and also would make the main screen group visible
	mainScreen(tgui::Gui &gui, networking* networkObject); //the constructor, has networking object because needs to pass it to the chat
	void liveUpdate(sf::Clock* globalClock); //simply calls the chat's live update function
};

class socialTabClass {
private:
	tgui::Group::Ptr socialTabGroup = tgui::Group::create({ sf::VideoMode::getDesktopMode().width , sf::VideoMode::getDesktopMode().height });
	//the above would be an invisible container from tgui which holds everything that gets drawn for this screen

	sf::RenderWindow *window = NULL; //would hold the window object
	networking* networkObject = NULL; //this will hold the network object for use in the chat
	chat* chatBox = NULL; //this will hold the chat box
	friend void chatBoxBulkAdd(networking* networkObject, chat* chatBox); //this will parse messages in the network object's buffer and add them

	tgui::ScrollablePanel::Ptr roomGuildSelectBox; //this will be the box on the left of the screen which will contain buttons for selecting room/guild
	
	float currentMaxHeightRoomGuildSelect = 0; //the max height of the room guild select box

	void addButtonToPanel(tgui::ScrollablePanel::Ptr panel, std::string text, float percentWidth);
	void populateRoomGuildSelectBox(); //will fill up the room guild select box

	void changeRoomGuild(std::string buttonText); //function which changes the room/guild in the rooms bit

	//the buttons for the various tabs
	tgui::Button::Ptr roomsBtn;
	tgui::Button::Ptr areaChatBtn;
	tgui::Button::Ptr guildSelectBtn;
	tgui::Button::Ptr privateMessagingBtn;

	float chatBoxContainerHeight = 0;
	float chatBoxContainerYCoord = 0;

	json roomGuildList; //this will contain the room guild stuff so a user can select and switch and stuff

	std::string activeTab = "rooms";

	void switchTabs(std::string buttonText);

	friend size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp); //friend function to get data from curl, this is forward declaring the function as we can't include the header file here, as this file is included in it
public:
	bool active = false; //we can use this to decide whether or not we should have the liveUpdate() function execute
	void setActive(bool active); //this would make the above boolean active, and also would make the main screen group visible
	socialTabClass(tgui::Gui &gui, networking* networkObject); //the constructor, has networking object because needs to pass it to the chat
	void liveUpdate(sf::Clock* globalClock); //simply calls the chat's live update function
};

class toolbar { //this contains the buttons in the top left of the screen
private:
	sf::RenderWindow *window = NULL; //would hold the window object
	mainScreen* main_screen = NULL; //holds the main screen (for switching using buttons
	socialTabClass* socialTabBit = NULL; //holds the social tab bit

	std::vector<tgui::BitmapButton::Ptr> buttons; //will contain all of the buttons needed
	std::string icons[11] = { "resources/home.png", "resources/commerce.png", "resources/social.png", "resources/map.png", "resources/sound_on.png", "resources/settings.png", "resources/help.png", "resources/exit.png", "resources/return.png", "resources/more.png", "resources/sound_off.png" };
	//the resource locations for the images used in the bitmap buttons

	friend void helpFunction(); //function prototype, for function to open the help page in the default browser
	void exitFunction(); //function prototype, for function to exit the game
	void socialTab(); //this will open the social tab/panel
	void returnToMain(); //this will go back to the main game screen
public:
	toolbar(sf::RenderWindow *window, mainScreen* main_screen, socialTabClass* socialTabBit, tgui::Gui &gui); //would initialise the toolbar and construct it
	tgui::Group::Ptr toolbarGroup = tgui::Group::create({ sf::VideoMode::getDesktopMode().width , sf::VideoMode::getDesktopMode().height });
	//the above would be an invisible container from tgui which holds everything that gets drawn for the toolbar, it's public so we can set it visible/invisible
};

#endif // !GUI_HEADER