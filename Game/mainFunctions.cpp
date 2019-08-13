#include <header.h>
#include <TGUI/TGUI.hpp>

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

void gameBit(){
	sf::RenderWindow *gameWindow = new sf::RenderWindow(sf::VideoMode(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height), "window", sf::Style::Fullscreen); //window, the one used for the launcher

	while (gameWindow->isOpen())
	{
		sf::Event event;
		while (gameWindow->pollEvent(event)) //check for events
		{
			if (event.type == sf::Event::Closed)
				gameWindow->close(); //close the window when you find the close event basically
		}

		gameWindow->clear(); //clears the previous contents of the screen off

		//stuff to draw here

		gameWindow->display(); //the contents of the screen are shown
		sf::sleep(sf::milliseconds(50)); //so the program doesnt just fry your CPU
	}
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
