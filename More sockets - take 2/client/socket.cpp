#include <iostream>
#include <string>
#include "SFML/Network.hpp"

void client(std::string IPADDRESS, unsigned short PORT);
void getInput(std::string &msg);
void getResponses();
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
	sf::Thread* receiveThread = 0;

	client(IPADDRESS, PORT);
	
	receiveThread = new sf::Thread(&getResponses);
	receiveThread->launch();

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

void getResponses(){
	while(true){
		sf::Packet receivePacket;
		std::string receiveString;

		globalMutex.lock();
		socket->receive(receivePacket);
		globalMutex.unlock();

		if(receivePacket >> receiveString){
			std::cout << receiveString;
		}

	}
}

void stayAlive(){
	while(true){
		sf::Packet sendPacket;

		if(keepAliveTimer.getElapsedTime().asSeconds() >= pingTime.asSeconds()){ //pings every ~3 seconds
			pingTime = sf::seconds(keepAliveTimer.getElapsedTime().asSeconds() + 3);

			sendPacket << "SERVER::PING::3SEC";

			globalMutex.lock();
			socket->send(sendPacket);
			globalMutex.unlock();
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

			globalMutex.lock();
			socket->send(sendPacket);
			globalMutex.unlock();
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