#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#define CURL_STATICLIB

#include <curl/curl.h>
#include <network.h>
#include <string>
#include <iostream>
#include <gui.h>
#include <SFML/Network.hpp>
#include "../xorFunction.h"

size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp) //first one is pointer to data, second is to the size of one data item,
{ //third is how many there are, so size * nmemb is data size, and fourth is pointer to the data you get back, which you actually access using CURLOPT_WRITEDATA and a reference
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

bool networking::login(std::string username, std::string password) {
	if (active == true) { //if the user is already logged in, ignore any input and just return true
		return true;
	}

	std::string msg = "";

	while (true) {
		if (socket->connect(ip.c_str(), port) == sf::Socket::Done) { //if the socket successfully connects to the specified IP address at the specified port...
			//the IP address has to be a C style array as well

			sf::Packet logonPacket;
			msg = "SERVER::LOGON::USERNAME::" + username + "SERVER::LOGON::PASSWORD::" + password;

			logonPacket << msg.c_str();

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

	if (receiveString.find(removeFlag) == 0) { //if the flag is at the beginning of the string
		receiveString.erase(receiveString.begin(), receiveString.begin() + removeFlag.length());

		if (receiveString == "true") {
			active = true;
			usernameReal = username;

			//the code below gets a json object from my website of the last 50 messages and parses them into a c++ json object from the json library
			//and also decrypts the message contents with the simple xor function

			curl_global_init(CURL_GLOBAL_ALL); //initialise libcurl functionality globally

			CURL* curl = curl_easy_init(); //we can set options for this to make it control how a transfer/transfers will be made
			std::string readBuffer; //string for the returning data

			curl_easy_setopt(curl, CURLOPT_URL, "http://erewhon.xyz/game/serverMessages.php?save=false&roomGuild=main.alpha");
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); //the callback
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); //will write data to the string, so the fourth param of the last callback is stored here

			curl_easy_perform(curl);
			delete curl;

			messages = json::parse(readBuffer);

			for (int i = 0; i < messages.size(); i++) {
				messages[i]["msg"] = xorFunction(messages[i]["msg"]);
			}
		}
		else {
			active = false;
		}
	}

	delete receiveCharArray; //then deletes the char array
	return active; //return whether or not it's active
}

void networking::getResponses() { //retrieves incoming message
	while (active) { //loop while active is true
		sf::Packet receivePacket; //a packet variable to hold the incoming packet
		char* receiveCharArray = NULL; //a char pointer to hold what will be a dynamically allocated char array

		socket->receive(receivePacket); //receives the packet

		receiveCharArray = new char[receivePacket.getDataSize()]; //makes a new char array the exact size of the data from the just received packet
		receivePacket >> receiveCharArray; //puts the packet's data into the char array variable we just put
		//need to do this as all of the data transmitted from the server was string with .c_str() applied to it, so was a C style string (char array)

		std::string receiveString(receiveCharArray);

		if (receiveString == "SERVER::DIE") { //when the server sends a message for it to shut down, it shuts down
			active = false;
			std::cout << "Press the enter key to exit.";
			continue;
		}
		
		//below basically gets the received string and splits it into username, message and time of message
		std::string msgID = receiveString;
		std::string username = receiveString;
		std::string msgContent = receiveString;
		std::string imgLocation = "";
		int time = 0;

		if (msgID.find("USER::ID::") == 0) {
			msgID.erase(msgID.begin(), msgID.begin() + std::string("USER::ID::").length());
			msgID.erase(msgID.find("USER::USERNAME"), msgID.size());

			username.erase(username.begin(), username.begin() + username.find("USER::USERNAME::") + std::string("USER::USERNAME::").length());
			username.erase(username.find("USER::TIME::"), username.size());

			receiveString.erase(receiveString.begin(), receiveString.begin() + receiveString.find("USER::TIME::") + std::string("USER::TIME::").length());
			receiveString.erase(receiveString.find("USER::MSG::"), receiveString.size());
			time = std::stoi(receiveString);

			msgContent.erase(msgContent.begin(), msgContent.begin() + msgContent.find("USER::MSG::") + std::string("USER::MSG::").length());
		}

		if (chatBoxActive == true) {
			msgContent = xorFunction(msgContent);
			chatBoxObject->addMessages(time, username, msgContent, imgLocation, std::stoi(msgID)); //sends a message with the extracted data
		}
		else {
			//if the chat box isn't active yet, then add the new message to the json object so the user doesn't miss any messages
			json messageObject;
			messageObject["imgURL"] = imgLocation;
			messageObject["msgID"] = msgID;
			messageObject["time"] = time;
			messageObject["from"] = username;
			messageObject["msg"] = msgContent;

			messages.push_back(messageObject);
		}
	}
}

void networking::stayAlive() { //for pinging the server
	while (active) { //loop while active is true
		sf::Packet sendPacket; //makes a packet which will contain the ping flag

		if (keepAliveTimer->getElapsedTime().asSeconds() >= pingTime.asSeconds()) { //pings every ~3 seconds
			pingTime = sf::seconds(keepAliveTimer->getElapsedTime().asSeconds() + 3); //then updates the time of the next ping to be in 3 seconds

			sendPacket << "SERVER::PING::3SEC"; //puts this string into the packet
			socket->send(sendPacket); //then sends it off to the server
		}
	}
}

void networking::sendMessage(std::string msg) {
	sf::Packet sendPacket; //the packet which will contain the data to send

	msg = xorFunction(msg);

	msg = "USER::USERNAME::" + usernameReal + "USER::MESSAGE::" + msg; //puts USER::USERNAME:: before appending the chosen username and USER::MESSAGE:: before appending the message
	//this will help with decoding the data on the server side

	sendPacket << msg.c_str(); //converts the string into a C style array, and puts it into the packet which will be sent

	socket->send(sendPacket); //sends the packet
}

networking::~networking() {
	delete socket; //destructor just deletes the socket
}