#ifndef MAIN_HEADER
#define MAIN_HEADER

#include <network.h>
#include <gui.h>
#include <game.h>
#include <TGUI/TGUI.hpp>

/*
This file will contain the main functions for the main stages of the game, the classes and stuff will be in other files
*/

void helpFunction(); 
//this will open the help function, it's for tgui buttons, it is used as a friend function by both the launcher and mainScreen classes of them (check in the gui.h file for more)

void gameBit(sf::Clock* globalClock, networking* networkObject, gameNetwork* gameConnection);
//this is the main game bit

void clearResources(networking* networkObject, sf::Thread* pingThread, sf::Thread* receiveThread, sf::Thread* tcpGameThread, sf::Clock* globalClock);
//this will delete stuff, like objects/threads so no memory leaks

void chatBoxBulkAdd(networking* networkObject, chat* chatBox);
//this will be a friend function to both the social and main screen classes, and will update the contents of the chat box

size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp);
//this will be a friend function to the network and social tab classes

#endif