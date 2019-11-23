#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#define CURL_STATICLIB

#include <curl/curl.h>
#include <network.h>
#include <string>
#include <iostream>
#include <gui.h>
#include <SFML/Network.hpp>
#include <header.h>
#include "../xorFunction.h"

void networking::getMessagesFromDB() {
	//the code below gets a json object from my website of the last 50 messages and parses them into a c++ json object from the json library
	//and also decrypts the message contents with the simple xor function

	CURL* curl = curl_easy_init(); //we can set options for this to make it control how a transfer/transfers will be made
	std::string readBuffer; //string for the returning data

	curl_easy_setopt(curl, CURLOPT_URL, std::string(websiteUrl+"/game/serverMessages.php?save=false&roomGuild=" + roomGuild).c_str());
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); //the callback
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); //will write data to the string, so the fourth param of the last callback is stored here

	curl_easy_perform(curl);
	delete curl;

	messages.clear(); //clears the contents of the buffer for a new request
	messages = json::parse(readBuffer);

	for (int i = 0; i < messages.size(); i++) {
		messages[i]["msg"] = xorFunction(messages[i]["msg"]);
	}
}

bool networking::login(std::string username) {
	if (active == true) { //if the user is already logged in, ignore any input and just return true
		return true;
	}

	std::string msg = "";

	while (true) {
		if (socket->connect(ip.c_str(), port) == sf::Socket::Done) { //if the socket successfully connects to the specified IP address at the specified port...
			//the IP address has to be a C style array as well

			//it sends the username that the launcher gave to the server for verification
			sf::Packet logonPacket;
			msg = "SERVER::LOGON::USERNAME::" + username;

			logonPacket << msg.c_str(); //packets expect to deal with const char* arrays rather than strings

			socket->send(logonPacket); //sends the username to the server for authentication

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
	std::string sessionIDFlag = "USER::SESSION_ID::";
	std::string userIDFlag = "USER::ID::";

	// std::cout << receiveString << std::endl;

	if (receiveString.find(removeFlag) == 0) { //if the flag is at the beginning of the string
		receiveString.erase(receiveString.begin(), receiveString.begin() + removeFlag.length());

		if (receiveString.find("false") == 0) {
			active = false;
		}
		else {
			std::string server_sessionID = receiveString;
			server_sessionID.erase(0, server_sessionID.find(sessionIDFlag) + sessionIDFlag.length());
			server_sessionID.erase(server_sessionID.find(userIDFlag), server_sessionID.length());

			//the below will check if the user ID has been sent from the server, as a response to the logon username we sent for the verification bit
			std::string server_userID = receiveString;
			server_userID.erase(server_userID.begin(), server_userID.begin() + server_userID.find(userIDFlag) + std::string(userIDFlag).length());

			receiveString.erase(receiveString.find(sessionIDFlag), receiveString.length());

			if (receiveString == "true") { //if the verification was successful
				userID = std::stoi(server_userID); //set user ID
				active = true; //verification was successful, so the rest of the program can proceed
				usernameReal = username; //sets the username to be the one that the launcher provided us with
				sessionID = server_sessionID; //sets the unique session ID thing, to send with our data for basic authentication

				curl_global_init(CURL_GLOBAL_ALL); //initialise libcurl functionality globally, as it'll be used now that login was successful

				getMessagesFromDB(); //this will get messages from the database as soon as login was successful
			}
			else {
				active = false;
			}
		}
	}

	if (active) {
		CURL* curl = curl_easy_init(); //we can set options for this to make it control how a transfer/transfers will be made
		std::string readBuffer; //string for the returning data

		curl_easy_setopt(curl, CURLOPT_URL, std::string(websiteUrl + "/game/getSettings.php?username=" + username).c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); //the callback
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); //will write data to the string, so the fourth param of the last callback is stored here

		curl_easy_perform(curl);
		delete curl;

		settings = json::parse(readBuffer);

		delete receiveCharArray; //then deletes the char array
	}

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

		if (receiveString == "die") {
			break;
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

		if (chatBoxActive == true) { //checks if the chatbox has been set to active, in which case decode received messages and add them to the chatbox
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

	msg = "USER::USERNAME::" + usernameReal + "USER::ROOMGUILD::" + roomGuild + "USER::MESSAGE::" + msg; //puts USER::USERNAME:: before appending the chosen username and USER::MESSAGE:: before appending the message
	//this will help with decoding the data on the server side

	sendPacket << msg.c_str(); //converts the string into a C style array, and puts it into the packet which will be sent

	socket->send(sendPacket); //sends the packet
}

networking::~networking() {
	delete socket; //destructor just deletes the socket
}