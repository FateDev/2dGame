#ifndef MAIN_HEADER
#define MAIN_HEADER

#include <network.h>
#include <gui.h>
#include <TGUI/TGUI.hpp>

/*
This file will contain the main functions for the main stages of the game, the classes and stuff will be in other files
*/

void helpFunction(); 
//this will open the help function, it's for tgui buttons, it is used as a friend function by both the launcher and mainScreen classes of them (check in the gui.h file for more)

bool launcherBit(networking* networkObject, sf::Thread* pingThread, sf::Thread* receiveThread, sf::Clock* globalClock);
//will show the launcher and everything

void gameBit(sf::Clock* globalClock, networking* networkObject); 
//this is the main game bit

void clearResources(networking* networkObject, sf::Thread* pingThread, sf::Thread* receiveThread, sf::Clock* globalClock); 
//this will delete stuff, like objects/threads so no memory leaks

void chatBoxBulkAdd(networking* networkObject, chat* chatBox);
//this will be a friend function to both the social and main screen classes, and will update the contents of the chat box

#endif