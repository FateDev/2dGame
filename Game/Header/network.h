#ifndef NETWORK_HEADER
#define NETWORK_HEADER

#include <iostream>
#include <string>
#include "SFML/Network.hpp"

void loginLoop(std::string IPADDRESS, int PORT, std::string username, std::string password, sf::TcpSocket *socket, bool &loggedIn);
//this will loop repeatedly until either the user crosses it off or gets the correct login details

void getInput(std::string &msg, bool &loggedIn, std::string username, sf::TcpSocket *socket);
//once the user has got the correct login details, this is run on the main thread, so that they can type, and then send messages by pressing enter

void getResponses(sf::TcpSocket *socket);
//this will get any responses from the server, decode them, and output them, and it's run on a separate thread

void stayAlive(sf::Clock keepAliveTimer, sf::Time pingTime, sf::TcpSocket *socket);
//this will ping the server periodically so that the server knows that this client is still acutally connected, as there's no other real way of checking, and it's run on another thread

#endif