#ifndef MAIN_HEADER
#define MAIN_HEADER

#include <network.h>
#include <gui.h>
#include <TGUI/TGUI.hpp>

/*
This file will contain the main functions for the main stages of the game, the classes and stuff will be in other files
*/

//will show the launcher and everything
bool launcherBit(networking* networkObject, sf::Thread* pingThread, sf::Thread* receiveThread, sf::Thread* getInputThread, sf::Clock* globalClock);

void gameBit();

void clearResources(networking* networkObject, sf::Thread* pingThread, sf::Thread* receiveThread, sf::Thread* getInputThread, sf::Clock* globalClock); //this will delete stuff, like objects/threads so no memory leaks

#endif