#include <iostream>
#include <string>
#include "SFML/Network.hpp"

const unsigned int PORT = 5000;
const std::string IPADDRESS = "erewhon.xyz";
const std::string reserved1 = "#";
const std::string reserved2 = "{";
const std::string reserved3 = "}";
const std::string hash = "{hash}";

sf::Mutex globalMutex;
sf::TcpSocket socket;
bool quit = false;
std::string msg;
std::string userName;

void sanitiseString(std::string &input){ //removes #, { and } from the string
	//also just removes curly braces
	size_t pos = input.find(reserved2);
	
	while(pos != std::string::npos){
		input.replace(pos, reserved2.length(), "");
		pos = input.find(reserved2);
	}

	pos = input.find(reserved3);
	
	while(pos != std::string::npos){
		input.replace(pos, reserved3.length(), "");
		pos = input.find(reserved3);
	}
	
	pos = input.find(reserved1); //this is a special type for holding size and the return value from find() in this application
	
	while(pos != std::string::npos){ //this is basically saying so long as the value returned from find() isn't empty
		input.replace(pos, reserved1.length(), "{hash}"); //replaces a found value with {hash}
		pos = input.find(reserved1); //looks for the value
	}
}

void processLoop(void){
	while(!quit){
		sf::Packet packetReceive;
		
		if(msg == ""){
			continue;
		}

		std::string tempMsgHolder;

		globalMutex.lock();

		sanitiseString(msg);
		tempMsgHolder = userName + "####" + msg;
		packetReceive <<  tempMsgHolder.c_str();
		msg = "";

		globalMutex.unlock();

		socket.send(packetReceive);
	}
}

void getInput(void){
	std::string temp1;
	std::string newMsg;
	std::cout << "Enter 'quit' to quit or a message to send: ";
	getline(std::cin, newMsg);

	if(newMsg == "quit")
		quit = true;
	
	globalMutex.lock();
	msg = newMsg;
	globalMutex.unlock();
}

void client(){
	while (true) {
		if (socket.connect(IPADDRESS.c_str(), PORT) == sf::Socket::Done) {
			std::cout << "Successfully connected" << std::endl;
			break;
		}
		std::cout << ".";
	}
}

int main(){
	sf::Thread* thread = 0;

	client();

	while(true){
		std::cout << "Enter a username: ";
		getline(std::cin, userName);
		
		if(userName == ""){
			std::cout << std::endl << "Invalid." << std::endl;
		}else{
			sanitiseString(userName);

			std::cout << std::endl;
			break;
		}
	}

	thread = new sf::Thread(&processLoop);
	thread->launch();

	while(!quit){
		getInput();
	}

	if(thread){
		thread->wait();
		delete thread;
	}

	return 0;
}
