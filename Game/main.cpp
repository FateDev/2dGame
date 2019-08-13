#include <iostream>
#include <string>
#include <functional>

#include <SFML/Network.hpp>
#include <TGUI/TGUI.hpp>

//user defined header files
#include <network.h>
#include <gui.h>

//tgui lib selection
#ifdef _DEBUG
#	pragma comment(lib, "tgui-d.lib")
#else
#	pragma comment(lib, "tgui.lib")
#endif

int main() {
	sf::Clock* globalClock = new sf::Clock; //the clock which is used to check when to ping the server
	networking* networkObject = new networking("192.168.0.15", 5000, globalClock);

	sf::Thread* receiveThread = new sf::Thread(&networking::getResponses, networkObject); //make the getResponses() function run on this thread
	sf::Thread* pingThread = new sf::Thread(&networking::stayAlive, networkObject); //makes the stayAlive() function run on this thread
	sf::Thread* getInputThread = new sf::Thread(&networking::getInput, networkObject); //thread for getting user input

	//window bit
	sf::RenderWindow *window = new sf::RenderWindow(sf::VideoMode(800, 600), "window", sf::Style::Close); //window
	launcher* launcherObject = new launcher(networkObject, window, pingThread, receiveThread, getInputThread, globalClock); //launcher

	while (window->isOpen())
	{
		sf::Event event;
		while (window->pollEvent(event)) //check for events
		{
			if (event.type == sf::Event::Closed)
				window->close(); //close the window when you find the close event basically

			launcherObject->gui->handleEvent(event); // Pass the event to the widgets
		}

		window->clear(); //clears the previous contents of the screen off

		launcherObject->liveUpdate(); //does the entire notifications bit as well as drawing widgets

		window->display(); //the contents of the screen are shown

		sf::sleep(sf::milliseconds(50)); //so the program doesnt just fry your CPU
	}

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
	delete launcherObject;
	delete networkObject;

	return 0;
}