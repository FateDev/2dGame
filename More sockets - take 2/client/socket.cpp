#include <iostream>
#include <string>
#include "SFML/Network.hpp"

void client(std::string IPADDRESS, unsigned short PORT);
void getInput(std::string &msg);
void stayAlive();

sf::Clock keepAliveTimer;
sf::Time pingTime;
sf::TcpSocket* socket = new sf::TcpSocket;
std::string username;

sf::Mutex globalMutex;

int main(){
	const std::string IPADDRESS("erewhon.xyz");
	const unsigned short PORT = 5000;

	std::string msg;

	sf::Thread* pingThread = 0;

	client(IPADDRESS, PORT);

	pingThread = new sf::Thread(&stayAlive);
	pingThread->launch();

	std::cout << "Set your username: ";
	getline(std::cin, username);
	
	getInput(msg);

	if(pingThread){
		pingThread->wait();
		delete pingThread;
	}
}

void stayAlive(){
	while(true){
		sf::Packet sendPacket;

		if(keepAliveTimer.getElapsedTime().asSeconds() >= pingTime.asSeconds()){ //pings every ~3 seconds
			pingTime = sf::seconds(keepAliveTimer.getElapsedTime().asSeconds() + 3);

			sendPacket << "SERVER::PING::3SEC";
			socket->send(sendPacket);
		}
	}
}

void getInput(std::string &msg){
	std::cout << "Enter a message: " << std::endl;

	while(true){
		sf::Packet sendPacket;

		getline(std::cin, msg);
		
		if(msg != ""){
			msg = "USER::USERNAME::" + username + "USER::MESSAGE::" + msg;
			sendPacket << msg.c_str();

			socket->send(sendPacket);
		}

		if(msg == "DISCONNECT"){
			socket->disconnect();
			break;
		}
	}
}

void client(std::string IPADDRESS, unsigned short PORT){
	while(true){
		if(socket->connect(IPADDRESS.c_str(), PORT) == sf::Socket::Done){
			std::cout << "Connected to " << IPADDRESS << ":" << PORT << std::endl;
			break;
		}
		std::cout << socket->connect(IPADDRESS.c_str(), PORT) << std::endl;
	}
}