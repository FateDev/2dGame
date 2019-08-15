//you need to define this to make sure <windows.h> doesn't conflict with any min()/max() functions, 
//as this header file has min() and max() functions as macros, so would otherwise cause conflix
#define NOMINMAX

#include <Windows.h>
#include <header.h>
#include <TGUI/TGUI.hpp>

void helpFunction() { //the function to make the help button open the help setting in browser
	ShellExecute(0, 0, L"https://erewhon.xyz/game/help/", 0, 0, SW_SHOW);
	//opens URL in browser,
	//1st param is handle to parent windows, but we're using SFML rather than win32 windows so this is NULL or 0
	//2nd param is the action, which isnt necessary here
	//3rd is the thing to 'do' (the URL), so the default browser would be used on windows
	//4th would be parameters to pass to the file, but not necessary for opening a URL
	//5th is the working directory of the action, none specified so current one used
	//6th is how to show the application once opened, so it shows it
}

bool launcherBit(networking* networkObject, sf::Thread* pingThread, sf::Thread* receiveThread, sf::Clock* globalClock) {
	sf::RenderWindow *launcherWindow = new sf::RenderWindow(sf::VideoMode(800, 600), "window", sf::Style::Close); //window, the one used for the launcher
	launcher* launcherObject = new launcher(networkObject, launcherWindow, pingThread, receiveThread, globalClock); //launcher

	while (launcherWindow->isOpen())
	{
		sf::Event event;
		while (launcherWindow->pollEvent(event)) //check for events
		{
			if (event.type == sf::Event::Closed)
				launcherWindow->close(); //close the window when you find the close event basically

			launcherObject->gui->handleEvent(event); // Pass the event to the widgets
		}

		if (networkObject->active == true) { //this means the login was successful
			launcherWindow->close(); //close the window afterwards
		}

		launcherWindow->clear(); //clears the previous contents of the screen off

		launcherObject->liveUpdate(); //does the entire notifications bit as well as drawing widgets

		launcherWindow->display(); //the contents of the screen are shown

		sf::sleep(sf::milliseconds(50)); //so the program doesnt just fry your CPU
	}

	delete launcherWindow;
	delete launcherObject;

	if (networkObject->active == true) {
		return true;
	}
	else {
		return false;
	}
}

void gameBit(sf::Clock* globalClock, networking* networkObject){
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	sf::RenderWindow *gameWindow = new sf::RenderWindow(sf::VideoMode(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height), "window", sf::Style::Fullscreen, settings); 
	//fullscreen window for the game

	tgui::Gui gui(*gameWindow); //the main gui for the entire game bit

	loadingScreen *loadingBit = new loadingScreen(gameWindow, globalClock); //the loading screen object
	sf::Time loadingScreenRemove = sf::milliseconds(globalClock->getElapsedTime().asMilliseconds() + 1500); //so it sets the duration of the loading screen as 2 seconds

	mainScreen mainGameScreen(gameWindow, gui);
	mainGameScreen.setActive(true);

	chat chatBox(25, 50);

	gui.add(chatBox.chatBoxContainer);

	networkObject->chatBoxObject = &chatBox;
	networkObject->chatBoxActive = true;

	while (gameWindow->isOpen())
	{
		sf::Event event;
		while (gameWindow->pollEvent(event)) //check for events
		{
			if (event.type == sf::Event::Closed) {
				gameWindow->close(); //close the window when you find the close event basically
			}

			gui.handleEvent(event); // Pass the event to the widgets
		}

		gameWindow->clear(); //clears the previous contents of the screen off

		//stuff to draw here
		/*
		if (loadingScreenRemove.asSeconds() - globalClock->getElapsedTime().asSeconds() <= 0 && loadingBit != NULL) {
			delete loadingBit;
			loadingBit = NULL;
			mainGameScreen.setActive(true);
		}
		else if (loadingBit != NULL) {
			loadingBit->liveUpdate();
		}
		*/
		if (chatBox.active == true && chatBox.active == true) {
			chatBox.liveUpdate(networkObject);
		}

		gui.draw();

		gameWindow->display(); //the contents of the screen are shown
		sf::sleep(sf::milliseconds(15)); //so the program doesnt just fry your CPU
	}

	delete gameWindow;
}

void clearResources(networking* networkObject, sf::Thread* pingThread, sf::Thread* receiveThread, sf::Clock* globalClock){
	//From here on, the TCP connection is severed, the threads are waited on and then destroyed, and then the network and launcher objects are deleted, and 0 is returned
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

	//when delete is called, the destructor is called before deallocating the memory
	delete networkObject;

	delete globalClock; //deletes the clock
}
