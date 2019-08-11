#include <network.h>
#include <string>
#include <iostream>
#include "SFML/Network.hpp"

void loginLoop(std::string IPADDRESS, int PORT, std::string username, std::string password, sf::TcpSocket *socket, bool &loggedIn) {
	while (true) {
		std::cout << "Enter username: "; //asks for username
		getline(std::cin, username); //and makes it the value of the global variable defined earlier

		std::cout << "Enter password: "; //asks for password
		getline(std::cin, password); //and makes it the value of the global variable defined earlier

		while (true) {
			if (socket->connect(IPADDRESS.c_str(), PORT) == sf::Socket::Done) { //if the socket successfully connects to the specified IP address at the specified port...
				//the IP address has to be a C style array as well
				//std::cout << "Connected to " << IPADDRESS << ":" << PORT << std::endl; //output that it has connected to that IP address and that port

				sf::Packet logonPacket;
				std::string msg = "SERVER::LOGON::USERNAME::" + username + "SERVER::LOGON::PASSWORD::" + password;

				logonPacket << msg.c_str();

				//std::cout << msg << std::endl;

				socket->send(logonPacket); //sends the username and password to the server for authentication

				break; //and break the loop
			}
			//otherwise it will loop endlessly until it does connect
		}

		sf::Packet receivePacket; //a packet variable to hold the incoming packet
		char* receiveCharArray = NULL; //a char pointer to hold what will be a dynamically allocated char array

		socket->receive(receivePacket); //receives the packet

		receiveCharArray = new char[receivePacket.getDataSize()]; //makes a new char array the exact size of the data from the just received packet
		receivePacket >> receiveCharArray; //puts the packet's data into the char array variable we just put
		//need to do this as all of the data transmitted from the server was string with .c_str() applied to it, so was a C style string (char array)

		std::string receiveString(receiveCharArray);
		std::string removeFlag = "SERVER::LOGON::RESPONSE::";

		if (receiveString.find(removeFlag) == 0) {
			receiveString.erase(receiveString.begin(), receiveString.begin() + removeFlag.length());

			if (receiveString == "true") {
				loggedIn = true;
				break;
			}
			else {
				std::cout << "Incorrect details." << std::endl;
			}
		}

		delete receiveCharArray; //then deletes the char array
	}
}

void getResponses(sf::TcpSocket *socket) { //retrieves incoming message
	while (true) { //loop endlessly
		sf::Packet receivePacket; //a packet variable to hold the incoming packet
		char* receiveCharArray = NULL; //a char pointer to hold what will be a dynamically allocated char array

		socket->receive(receivePacket); //receives the packet

		receiveCharArray = new char[receivePacket.getDataSize()]; //makes a new char array the exact size of the data from the just received packet
		receivePacket >> receiveCharArray; //puts the packet's data into the char array variable we just put
		//need to do this as all of the data transmitted from the server was string with .c_str() applied to it, so was a C style string (char array)

		std::string receiveString(receiveCharArray);

		std::cout << receiveCharArray; //outputs the message which was received
	}
}

void stayAlive(sf::Clock keepAliveTimer, sf::Time pingTime, sf::TcpSocket *socket) { //for pinging the server
	while (true) { //loop endlessly
		sf::Packet sendPacket; //makes a packet which will contain the ping flag

		if (keepAliveTimer.getElapsedTime().asSeconds() >= pingTime.asSeconds()) { //pings every ~3 seconds
			pingTime = sf::seconds(keepAliveTimer.getElapsedTime().asSeconds() + 3); //then updates the time of the next ping to be in 3 seconds

			sendPacket << "SERVER::PING::3SEC"; //puts this string into the packet
			socket->send(sendPacket); //then sends it off to the server
		}
	}
}

void getInput(std::string &msg, bool &loggedIn, std::string username, sf::TcpSocket *socket) { //gets input from the user
	std::cout << "Enter a message: " << std::endl; //prompted to enter a message once

	while (loggedIn == true) { //loop endlessly
		sf::Packet sendPacket; //the packet which will contain the data to send

		getline(std::cin, msg); //gets the string to send and puts it in the msg variable declared in int main()

		if (msg != "") { //if the message isn't empty
			msg = "USER::USERNAME::" + username + "USER::MESSAGE::" + msg; //puts USER::USERNAME:: before appending the chosen username and USER::MESSAGE:: before appending the message
			//this will help with decoding the data on the server side

			sendPacket << msg.c_str(); //converts the string into a C style array, and puts it into the packet which will be sent

			socket->send(sendPacket); //sends the packet
		}
	}
}