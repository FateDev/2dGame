//you need to define this to make sure <windows.h> doesn't conflict with any min()/max() functions, 
//as this header file has min() and max() functions as macros, so would otherwise cause conflix
#define NOMINMAX

#include <Windows.h>
#include <header.h>
#include <game.h>
#include <TGUI/TGUI.hpp>

void helpFunction() { //the function to make the help button open the help setting in browser
	ShellExecute(0, 0, L"https://erewhon.xyz/game/help/", 0, 0, SW_SHOW);
	//opens URL in browser
	//1st param is handle to parent windows, but we're using SFML rather than win32 windows so this is NULL or 0
	//2nd param is the action, which isnt necessary here
	//3rd is the thing to 'do' (the URL), so the default browser would be used on windows
	//4th would be parameters to pass to the file, but not necessary for opening a URL
	//5th is the working directory of the action, none specified so current one used
	//6th is how to show the application once opened, so it shows it
}

long cantorPairingFunction(long x, long y) {
	return 0.5 * (x + y) * (x + y + 1) + y;
}

void inverseCantorPairingFunction(long z, long* x, long* y) {
	float w = floor(0.5 * (sqrt(8 * z + 1) - 1));
	float t = 0.5 * (pow(w, 2) + w);
	*y = z - t;
	*x = w - *y;
}

size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp) //first one is pointer to data, second is to the size of one data item,
{ //third is how many there are, so size * nmemb is data size, and fourth is pointer to the data you get back, which you actually access using CURLOPT_WRITEDATA and a reference
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

void chatBoxBulkAdd(networking* networkObject, chat* chatBox) {
	networkObject->messagesMutex.lock();
	//the below for loop loops through the json object from the networking object, and adds all of them to the chat box scrollable panel
	for (int i = networkObject->messages.size() - 1; i >= 0; i--) {
		int time = std::stoi(networkObject->messages[i]["time"].get<std::string>());
		int msgID = std::stoi(networkObject->messages[i]["msgID"].get<std::string>());
		std::string imgLocation = networkObject->messages[i]["imgURL"].get<std::string>();

		std::string from = "";
		if (networkObject->messages[i]["from"].is_null()) { //checks if it's not got a username, then the user no longer exists
			from = "DELETED USER";
		}
		else {
			from = networkObject->messages[i]["from"].get<std::string>();
		}

		std::string msg = networkObject->messages[i]["msg"].get<std::string>();

		if (imgLocation == "EMPTY") {
			imgLocation = "";
		}

		chatBox->addMessages(time, from, msg, imgLocation, msgID);
	}
	networkObject->messagesMutex.unlock();
}

void gameBit(sf::Clock* globalClock, networking* networkObject, gameNetwork* gameConnection){
	//the below is just the settings stuff for it
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8; //so drawn objects don't look too sharp (especially for circles and stuff)
	
	int taskbarHeight = 0;
	RECT rect;
	HWND taskBar = FindWindow(L"Shell_traywnd", NULL);
	if (taskBar && GetWindowRect(taskBar, &rect)) {
		taskbarHeight = rect.bottom - rect.top;
	}

	sf::RenderWindow gameWindow(sf::VideoMode(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height-taskbarHeight), "Descend", sf::Style::None, settings);
	gameWindow.setPosition(sf::Vector2i(0, 0));
	gameWindow.setVerticalSyncEnabled(true);

	sf::Image icon;
	icon.loadFromFile("resources/icon.png");
	gameWindow.setIcon(200, 200, icon.getPixelsPtr());
	//fullscreen window for the game

	tgui::Gui gui(gameWindow); //the main gui for the entire game bit

	/********************************/
	/********************************/
	/********************************/

	sf::Time loadingScreenRemove = sf::milliseconds(globalClock->getElapsedTime().asMilliseconds() + 1000); //so it sets the duration of the loading screen as 1 second
	//the below																									 
	///temp set the above to 1ms
	//the above
	/********************************/
	/********************************/
	/********************************/

	//below makes the loading screen, main screen, and tool bar (buttons in the top left) objects
	loadingScreen *loadingBit = new loadingScreen(&gameWindow, globalClock);  
	//the above is dynamically allocated because we won't need it after and so we can know it's actually gone (as we can set it to NULL)
	mainScreen mainGameScreen(gui, networkObject);

	socialTabClass socialTabBit(gui, networkObject);

	sf::View gameView(sf::FloatRect(0, 0, gameWindow.getSize().x, gameWindow.getSize().y));
	gameView.setCenter(0, 0); //sets 0,0 as the center

	game actualGame(networkObject, gameConnection, &gameWindow, &gameView);
	gameConnection->gameReference = &actualGame;

	toolbar mainToolbar(&gameWindow, &mainGameScreen, &socialTabBit, gui);
	inventory inventoryBit(gui, &gameWindow, &actualGame);

	networkObject->inventoryObject = &inventoryBit; //will set the inventory object
	networkObject->getUserInventory(); //stores the user's inventory in a json object in this object

	stats onScreenStats(gui, networkObject); //makes the status object

	bool isWindowFocused = true;

	actualGame.statsObj = &onScreenStats; //sets the stats object as this, so the data from the draw loop can be used to update it

	while (gameWindow.isOpen()) //so long as the window is open
	{
		sf::Event event; //will store the current event
		while (gameWindow.pollEvent(event)) //check for events
		{
			if (event.type == sf::Event::Closed) 
				gameWindow.close(); //close the window when you find the close event basically
			if (event.type == sf::Event::GainedFocus)
				isWindowFocused = true;
			if (event.type == sf::Event::LostFocus)
				isWindowFocused = false;


			gui.handleEvent(event); //pass the event to the widgets
			actualGame.listenForKeys(event); //pass the event on to the game to listen for keys
			inventoryBit.listenForKeys(event); //pass the event on and check if the inventory should be opened or closed
		}

		gameWindow.clear(sf::Color(26, 25, 30)); //clears the previous contents of the screen off, and replaces it with a nice colour
		//below is the stuff for the loading screen, basically it says that if the expiry time has passed and the loading screen isn't null, delete it, set it to null
		//and then make the main screen active, otherwise draw the loading screen stuff and that little animation too

		if (loadingScreenRemove.asSeconds() - globalClock->getElapsedTime().asSeconds() <= 0 && loadingBit != NULL) {
			delete loadingBit;
			loadingBit = NULL;
			mainGameScreen.setActive(true);
			mainToolbar.toolbarGroup->setVisible(true); //sets the toolbar as visible too
			inventoryBit.displayToolbar();
		} else if (loadingBit != NULL) {
			gameWindow.setView(gameWindow.getDefaultView());
			loadingBit->liveUpdate();
		}

		//the below would check if the main game screen has been made active, and to then call the live update method, 
		//which calls the chat's live update method (for sending messages and stuff)
		if (mainGameScreen.active == true) {
			actualGame.live(&inventoryBit, isWindowFocused); //processes stuff like keys and sends it, and drawing stuff for the game

			gameWindow.setView(gameWindow.getDefaultView());

			if (inventoryBit.isInventoryOpen()) {
				//disable everything except GUI bit while gui is open
				mainGameScreen.chatBox->chatBoxContainer->setEnabled(false);
				mainToolbar.toolbarGroup->setEnabled(false);
			}
			else {
				//re-enable everything if the gui is closed, and also process text and whatnot as normal
				mainGameScreen.chatBox->chatBoxContainer->setEnabled(true);
				mainToolbar.toolbarGroup->setEnabled(true);
				mainGameScreen.liveUpdate(globalClock);
			}
		}
		if (socialTabBit.active == true) {
			socialTabBit.liveUpdate(globalClock);

			try { //it appears there were some inexplicable access violations here, so we'll use this
				socialTabBit.completeThreadWork();
			}catch (...) {
				std::cout << "something went wrong, but we'll pretend it didn't, for the complete thread work bit" << std::endl;
			}
		}
			
		if(socialTabBit.active != true && !inventoryBit.isInventoryOpen() && mainGameScreen.active == true){
			onScreenStats.enableDraw();
			onScreenStats.updateStats();
		}
		else {
			onScreenStats.disableDraw();
		}

		try {
			gui.draw(); //draws everything that's been added to it (hopefully just groups of tgui objects for the different screens)
		}catch (...) {
			std::cout << "something went wrong, but we'll pretend it didn't, for drawing the gui (from tgui)" << std::endl;
		}

		if (mainGameScreen.active) {
			inventoryBit.InventoryLive(&gameView); //does the drag drop bit and user item bit, do this here because the drag drop bit takes priority over basically everything
		}

		try {
			gameWindow.display(); //the contents of the screen are shown
		}catch (...) {
			std::cout << "something went wrong, but we'll pretend it didn't, for displaying the game window" << std::endl;
		}

		sf::sleep(sf::milliseconds(1000/60)); //so the program doesnt just fry your CPU
	}

	gameConnection->exitMutex.lock();
	gameConnection->gameReference = 0;
	gameConnection->exitMutex.unlock();

	gameConnection->exiting = true; //this will end the listen thread's while loop

	socialTabBit.destroyThreads(); //this will end all of the threads
	delete loadingBit; //deletes the loadingBit object
}

void clearResources(networking* networkObject, sf::Thread* pingThread, sf::Thread* receiveThread, sf::Thread* tcpGameThread, sf::Clock* globalClock){
	//From here on, the TCP connection is severed, the threads are waited on and then destroyed, and then the network and launcher objects are deleted, and 0 is returned
	networkObject->active = false; //ensures that the receiveThread and the pingThread return, as they use while(active) { ...code... } in their functions

	std::string msg; //the string used in getInput(...)
	sf::Packet sendPacket; //the packet which will contain the data to send
	msg = "USER::LEAVE";
	//this will help with decoding the data on the server side
	sendPacket << msg.c_str(); //converts the string into a C style array, and puts it into the packet which will be sent
	networkObject->socket->send(sendPacket); //sends the packet

	if (pingThread) { //if the ping thread exists...
		pingThread->wait(); //wait for it's current bit of code to finish
		delete pingThread; //then delete it
	}

	if (receiveThread) { //if the receiveThread exists...
		receiveThread->wait(); //wait for it's current bit of code to finish
		delete receiveThread; //then delete it
	}

	if (tcpGameThread) { //if the udpGameThread exists...
		tcpGameThread->wait(); //wait for it's current bit of code to finish
		delete tcpGameThread; //then delete it
	}

	//when delete is called, the destructor is called before deallocating the memory
	delete networkObject;

	delete globalClock; //deletes the clock
}