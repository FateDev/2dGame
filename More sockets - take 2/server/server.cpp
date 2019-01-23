#include <iostream>
#include <vector>
#include <ctime>
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
void extractInformation(std::string &msg);
void process();

int main(){
	sf::Thread* otherThread = 0;

	const unsigned short PORT = 5000;
	bool state = true; //if true, is running, else if false, isn't running

	otherThread = new sf::Thread(&process);
	otherThread->launch();
	
	server(PORT, state);

	if(otherThread){
		otherThread->wait();
		delete otherThread;
	}

	return 0;
}

bool processString(std::string msg){
	if(msg.find("SERVER::PING::3SEC") == 0){
		return true;
	}else{
		return false;
	}
}

bool extractInformation(std::string &msg, int clientNum){
	std::string usernameToken = "USER::USERNAME::";
	std::string messageToken = "USER::MESSAGE::";
	std::string username;

	if(msg.find(usernameToken) != 0 || msg.find(messageToken) == std::string::npos){
		return false; //if USER::USERNAME:: and USER::MESSAGE:: are not found don't process this

	}else if(msg.find(messageToken) != std::string::npos){
		username = msg;
		username.erase(username.begin(), username.begin() + usernameToken.length());
		username.erase(username.find(messageToken), username.length());

		users[clientNum].username = username;

		msg.erase(msg.begin(), msg.begin() + (msg.find(messageToken) + messageToken.length()));
		
		return true;
	}
}

void process(){
	std::time_t t; //current time
	std::tm* now; //object which processes into year, month, day, etc

	while(true){
		if(selector.wait(sf::seconds(1))){
			for(int i = 0;i < users.size(); i++){
				if(selector.isReady(*(users[i].socket))){
					sf::Packet receivePacket;
					sf::Packet sendPacket;
					std::string receiveString = "";
					std::string currentTime;
					std::string sendString;

					t = std::time(0);
					now = std::localtime(&t);

					users[i].socket->receive(receivePacket);

					if((receivePacket >> receiveString) && receiveString != ""){
						if(processString(receiveString)){
							users[i].timeOfExpiry = sf::seconds(expiryTimer.getElapsedTime().asSeconds() + 5);
							continue;
						}

						if(!extractInformation(receiveString, i)){
							continue;
						}

						if(receiveString == "DISCONNECT"){
							users.erase(users.begin() + i);
							std::cout << "SERVER: USER LEFT" << std::endl;
							std::cout << "SERVER: CLIENTS ONLINE: " << users.size() << std::endl;
							continue;
						}else{
							if(now->tm_hour < 10){currentTime += "0";}

							currentTime += std::to_string(now->tm_hour) + ":";

							if(now->tm_min < 10){currentTime += "0";}

							currentTime += std::to_string(now->tm_min) + ":";

							if(now->tm_sec < 10){currentTime += "0";}
							
							currentTime += std::to_string(now->tm_sec);

							sendString = "[" + currentTime + "] " + users[i].username + ": " + receiveString + "\n";

							std::cout << sendString;

							sendPacket << sendString;

							for(int j = 0; j < users.size(); j++){
								if(j == i){
									continue;
								}

								users[i].socket->send(sendPacket);
							}

							currentTime = "";
						}
					}
				}
			}
		}

		for(int i = 0;i < users.size();i++){
			if(users[i].timeOfExpiry.asSeconds() <= expiryTimer.getElapsedTime().asSeconds()){
				std::string msgString = "SERVER: USER LEFT\nSERVER: CLIENTS ONLINE: " + std::to_string(users.size()) + "\n";
				users.erase(users.begin() + i);
				std::cout << msgString;

				sf::Packet sendPacket;
				sendPacket << msgString;

				users[i].socket->send(sendPacket);
			}
		}
		
		sf::sleep(sf::milliseconds(50)); //slows down the process loop so less intensive on my poor laptop
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
		
		sf::sleep(sf::milliseconds(50)); //slows down the listener loop so less intensive on my poor laptop
	}
}