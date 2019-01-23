#include <iostream>
#include <vector>
#include <SFML/Network.hpp>

const unsigned int PORT = 5000;
const std::string reserved = "#";
const std::string delmiter = "####";
const std::string hash = "{hash}";

std::vector<sf::TcpSocket*> clients;
sf::TcpListener listener;

std::string msg;
std::string userName;
sf::TcpSocket* activeSocket;

void extractInfo(std::string input){
	size_t pos = input.find(delmiter); //finds the first #
	std::string tempHolder;

	tempHolder = input.substr(0, pos); //gets the text up to the first #, that is the username

	userName = tempHolder;

	input.erase(0, pos+4); //erase username and the # from the input string

	pos = userName.find(hash); //find {hash} inside of the username

	while(pos != std::string::npos){ //so long as it is found
		userName.replace(pos, hash.length(), reserved); //replace it with #
		pos = userName.find(hash); //find the next occurence
	}

	msg = input; //what is left of the input is now stored in the msg variable

	pos = msg.find(hash); //find if the contents have {hash}

	while(pos != std::string::npos){ //so long as an occurence of {hash} is found
		msg.replace(pos, hash.length(), reserved); //replace {hash} with #
		pos = msg.find(hash); //find the next occurence of {hash}
	}

	//if no occurence are found when using find(), std::string::npos is returned, which is basically -1 (not found)
}

void processLoop(void){
	while(true){
		std::cout << clients.size() << std::endl;
		
		activeSocket = new sf::TcpSocket;

		if(listener.accept(*activeSocket) == sf::Socket::Done){
			clients.push_back(activeSocket);

			std::cout <<  "Server: New client connected: " << activeSocket->getRemoteAddress() << std::endl;
		}else{
			delete activeSocket;
		}

		for(int i = 0;i < clients.size();i++){
			/*
			if(clients[i]->getRemoteAddress() == sf::IpAddress::None){
				std::cout << "Server: A user has disconnected." << std::endl;

				clients.erase(clients.begin() + i); //erases the disconnected client
				
				continue;
			}
			*/

			sf::Packet packetReceive;
			std::string receiveMsg;
			clients[i]->receive(packetReceive);

			if((packetReceive >> receiveMsg) && !receiveMsg.empty()){
				extractInfo(receiveMsg);

				std::cout << userName << ": " << msg << std::endl;
			}
		}
	}
}

void server(void){
	listener.listen(PORT);

	activeSocket = new sf::TcpSocket;

	if(listener.accept(*activeSocket) == sf::Socket::Done){
		clients.push_back(activeSocket);

		std::cout <<  "Server: New client connected: " << activeSocket->getRemoteAddress() << std::endl;
	}else{
		delete activeSocket;
	}
}

int main(){
	std::vector<sf::TcpSocket*> clients;

	std::cout << "Server: Server Started" << std::endl;

	server();

	processLoop();
}
