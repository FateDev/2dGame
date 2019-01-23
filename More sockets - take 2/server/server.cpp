#include <iostream>
#include <vector>
#include <SFML/Network.hpp>

struct user{
	sf::TcpSocket* socket;
	std::string username;
	sf::Time timeOfExpiry;
};

std::vector<user> users;

std::vector<sf::TcpSocket*> clients;
sf::SocketSelector selector;
sf::Clock expiryTimer;

void server(unsigned short PORT, bool state);
bool processString(std::string msg);
void process();

int main(){
	sf::Thread* otherThread = 0;

	const unsigned short PORT = 5000;
	bool state = true; //if true, is running, else if false, isn't running

	otherThread = new sf::Thread(&process);
	otherThread->launch();
	
	server(PORT, state);
}

bool processString(std::string msg){
	if(msg.find("SERVER::PING::3SEC") == 0){
		std::cout << "YES" << std::endl;
		return true;
	}else{
		return false;
	}
}

void process(){
	while(true){
		if(selector.wait(sf::seconds(1))){
			for(int i = 0;i < users.size(); i++){
				if(selector.isReady(*(users[i].socket))){
					sf::Packet receivePacket;
					std::string receiveString = "";

					users[i].socket->receive(receivePacket);

					if((receivePacket >> receiveString) && receiveString != ""){
						if(processString(receiveString)){
							users[i].timeOfExpiry = sf::seconds(expiryTimer.getElapsedTime().asSeconds() + 5);
						}

						if(receiveString == "DISCONNECT"){
							users.erase(users.begin() + i);
							std::cout << "SERVER: USER LEFT" << std::endl;
							std::cout << "SERVER: CLIENTS ONLINE: " << users.size() << std::endl;
							continue;
						}else{
							std::cout << "USER: " << receiveString << std::endl;
						}
					}
				}
			}
		}

		for(int i = 0;i < users.size();i++){
			if(users[i].timeOfExpiry.asSeconds() <= expiryTimer.getElapsedTime().asSeconds()){
				users.erase(users.begin() + i);
				std::cout << "SERVER: USER LEFT" << std::endl;
				std::cout << "SERVER: CLIENTS ONLINE: " << users.size() << std::endl;
			}
		}
	}
}

void server(unsigned short PORT, bool state){
	sf::TcpListener listener;

	listener.listen(PORT);

	sf::TcpSocket* socket;

	user* userPtr;

	std::cout << "SERVER: SERVER STARTED" << std::endl;

	while(state){
		socket = new sf::TcpSocket;
		userPtr = new user;

		if(listener.accept(*socket) == sf::Socket::Done){
			std::cout << "SERVER: New connection @ " << socket->getRemoteAddress() << std::endl;

			userPtr->socket = socket;
			userPtr->timeOfExpiry = sf::seconds(expiryTimer.getElapsedTime().asSeconds() + 5);

			users.push_back(*userPtr);
			//clients.push_back(socket);
			selector.add(*socket);
		}else{
			delete userPtr;
			delete socket;
		}
	}
}