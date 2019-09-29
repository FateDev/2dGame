//you need to define this to make sure <windows.h> doesn't conflict with any min()/max() functions, 
//as this header file has min() and max() functions as macros, so would otherwise cause conflix
#define NOMINMAX

#include <Windows.h>
#include <iostream>
#include <curl/curl.h>
#include <SFML/Graphics.hpp>
#include "headerFuncs.h"

int main() {
	curl_global_init(CURL_GLOBAL_ALL); //initialise libcurl functionality globally, as it'll be used now that login was successful

	sf::Clock* globalClock = new sf::Clock; //the clock which is used to check when to ping the server
	networking* networkObject = new networking("192.168.0.15", 5000); //initialises the network object

	sf::RenderWindow *launcherWindow = new sf::RenderWindow(sf::VideoMode(800, 600), "One More Time", sf::Style::Close); //window, the one used for the launcher
	launcher* launcherObject = new launcher(networkObject, launcherWindow, globalClock); //launcher

	while (launcherWindow->isOpen())
	{
		sf::Event event;
		while (launcherWindow->pollEvent(event)) //check for events
		{
			if (event.type == sf::Event::Closed)
				launcherWindow->close(); //close the window when you find the close event basically

			launcherObject->gui->handleEvent(event); // Pass the event to the widgets
		}

		if (networkObject->loggedIn == true) { //this means the login was successful
			launcherWindow->close(); //close the window afterwards
		}

		launcherWindow->clear(); //clears the previous contents of the screen off

		launcherObject->liveUpdate(); //does the entire notifications bit as well as drawing widgets

		launcherWindow->display(); //the contents of the screen are shown

		sf::sleep(sf::milliseconds(50)); //so the program doesnt just fry your CPU
	}

	delete launcherWindow;
	delete launcherObject;

	if (networkObject->loggedIn == true) {
		std::string arguments = "";

		//the below are passed as arguments for the game program to use
		arguments += "Hg+Lb9B6Q/zrMduYEvPmuQ==";
		arguments += " " + networkObject->loggedinToken;
		arguments += " " + networkObject->confirmedUsername;

		ShellExecute(0, 0, "Game\\Game.exe", arguments.c_str(), NULL, SW_SHOW);
		//1st param is handle to parent windows, but we're using SFML rather than win32 windows so this is NULL or 0
		//2nd param is the action, which isnt necessary here
		//3rd is the thing to 'do' (the program address)
		//4th would be parameters to pass to the file, so we pass arguments from the string, separated by spaces
		//5th is the working directory of the action, none specified so current one used
		//6th is how to show the application once opened, so it shows it
	}

	return 0;
}