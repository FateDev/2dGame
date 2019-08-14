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

tgui::Panel::Ptr msgMaker(std::string usernameText, std::string messageText, int percent, std::string imgLocation) {
	float imgRatio = 0;
	float percentOfScreenX = (float(sf::VideoMode::getDesktopMode().width) / 100) * percent; //max set by params
	float halfScreen = (float(sf::VideoMode::getDesktopMode().height) / 100) * 50; //max anything should be, set right here

	auto username = tgui::Label::create(usernameText);
	username->setSize({ percentOfScreenX, 20 });
	username->setPosition({ 10, 10 });
	username->setVerticalAlignment(tgui::Label::VerticalAlignment::Center);

	auto msg = tgui::Label::create(messageText);
	msg->setPosition(30, 30);
	msg->setMaximumTextWidth(percentOfScreenX - 40);

	auto img = tgui::Picture::create(imgLocation);
	img->setPosition({ 10, 30 + msg->getSize().y + 5 });
	imgRatio = img->getSize().y / img->getSize().x; //gets y/x of image

	if (img->getSize().y > halfScreen) {
		img->setSize(percentOfScreenX - 20, (percentOfScreenX - 20) * imgRatio); //if the height is greater than half the screen, then resize the width instead
	}
	else {
		img->setSize(1/imgRatio * halfScreen, halfScreen); //otherwise resize the height
	}

	auto msgContainer = tgui::Panel::create({ msg->getSize().x + 40, msg->getSize().y + username->getSize().y + img->getSize().y + 25 });

	msgContainer->add(username);
	msgContainer->add(msg);
	msgContainer->add(img);

	return msgContainer;
}

tgui::Panel::Ptr msgMaker(std::string usernameText, std::string messageText, int percent) {
	float percentOfScreenX = (float(sf::VideoMode::getDesktopMode().width) / 100) * percent;

	auto username = tgui::Label::create(usernameText);
	username->setSize({ percentOfScreenX, 20 });
	username->setPosition({ 10, 10 });
	username->setVerticalAlignment(tgui::Label::VerticalAlignment::Center);

	auto msg = tgui::Label::create(messageText);
	msg->setPosition(30, 30);
	msg->setMaximumTextWidth(percentOfScreenX - 40);

	auto msgContainer = tgui::Panel::create({ msg->getSize().x + 40, msg->getSize().y + username->getSize().y + 25 });

	msgContainer->add(username);
	msgContainer->add(msg);

	return msgContainer;
}

bool launcherBit(networking* networkObject, sf::Thread* pingThread, sf::Thread* receiveThread, sf::Thread* getInputThread, sf::Clock* globalClock) {
	sf::RenderWindow *launcherWindow = new sf::RenderWindow(sf::VideoMode(800, 600), "window", sf::Style::Close); //window, the one used for the launcher
	launcher* launcherObject = new launcher(networkObject, launcherWindow, pingThread, receiveThread, getInputThread, globalClock); //launcher

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

void gameBit(sf::Clock* globalClock){
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	sf::RenderWindow *gameWindow = new sf::RenderWindow(sf::VideoMode(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height), "window", sf::Style::Fullscreen, settings); 
	//fullscreen window for the game

	tgui::Gui gui(*gameWindow); //the main gui for the entire game bit

	loadingScreen *loadingBit = new loadingScreen(gameWindow, globalClock); //the loading screen object
	sf::Time loadingScreenRemove = sf::milliseconds(globalClock->getElapsedTime().asMilliseconds() + 1500); //so it sets the duration of the loading screen as 2 seconds

	mainScreen mainGameScreen(gameWindow, gui);
	mainGameScreen.setActive(true);

	tgui::Panel::Ptr msg1 = msgMaker("Username", "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Praesent venenatis ac ligula et laoreet. Nunc at facilisis orci. Nullam urna massa, scelerisque quis libero a, pretium gravida erat. Sed porta libero ex. Aenean pharetra nibh et tincidunt interdum. Pellentesque leo dui, fermentum nec ultrices rutrum, fermentum sit amet arcu. Duis sollicitudin sapien ut lacus mollis, ac dictum ligula dignissim.", 23);
	tgui::Panel::Ptr msg2 = msgMaker("Username", "epic message dude", 23, "resources/snow.jpg");
	msg1->setPosition({ 0,  100 });
	msg2->setPosition({ 0,  msg1->getFullSize().y + 100 });

	gui.add(msg1);
	gui.add(msg2);
	


	while (gameWindow->isOpen())
	{
		sf::Event event;
		while (gameWindow->pollEvent(event)) //check for events
		{
			if (event.type == sf::Event::Closed)
				gameWindow->close(); //close the window when you find the close event basically
			
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
		}*/
		gui.draw();

		gameWindow->display(); //the contents of the screen are shown
		sf::sleep(sf::milliseconds(15)); //so the program doesnt just fry your CPU
	}

	delete gameWindow;
}

void clearResources(networking* networkObject, sf::Thread* pingThread, sf::Thread* receiveThread, sf::Thread* getInputThread, sf::Clock* globalClock){
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

	if (getInputThread) {
		getInputThread->wait();
		delete getInputThread;
	}

	//when delete is called, the destructor is called before deallocating the memory
	delete networkObject;

	delete globalClock; //deletes the clock
}
