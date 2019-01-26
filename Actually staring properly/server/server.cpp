#include <iostream>
#include <vector>
#include <ctime>
#include <fstream>
#include <SFML/Network.hpp>

struct user{ //struct to hold user's sockets, usernames, and the time left until their socket expires
	sf::TcpSocket* socket;
	std::string username;
	sf::Time timeOfExpiry;
	bool loggedIn;
};

std::vector<user> users; //vector to hold users

//sockets are normally in blocking mode, i.e, waits until a process is complete, so for say socket->receive(packet), it will wait until at least some data
//has been passed to it, so essentially freezes the program, so it'd be good if this blocking occurred, but for every single socket at the same time,
//i.e wait for any sort of activity and process it
//this is achieved using a selector
sf::SocketSelector selector; //a selector, which, once a socket of any type is added to it, listens and waits for activity from any of them, and let's them proceed
sf::Clock expiryTimer; //this is the global clock against which long polling is employed
//basically is used to check how long it's been since the server was last pinged

//prototypes
void server(unsigned short PORT);
bool processString(std::string msg);
void extractInformation(std::string &msg);
void process();

int main(){
	sf::Thread* otherThread = 0; //the thread for running the main receiving and sending processes

	const unsigned short PORT = 5000; //server port

	otherThread = new sf::Thread(&process); //makes the thread
	otherThread->launch(); //launches it
	
	server(PORT); //launches the server to listen on that specific port

	if(otherThread){ //if a thread exists
		otherThread->wait(); //wait until it's completed this bit of code
		delete otherThread; //delete it, by the way as sever(...) has an infinite while loop inside, this is only really called when quitting the program
	}

	return 0;
}

bool processString(std::string msg){ //for checking if the server has been pinged by a socket
	if(msg.find("SERVER::PING::3SEC") == 0){
		return true;
	}else{
		return false;
	}
	std::cout << msg << std::endl;
}

bool extractInformation(std::string &msg, int clientNum){ //extracts username and the message
	std::string usernameToken = "USER::USERNAME::";
	std::string messageToken = "USER::MESSAGE::";
	std::string username;

	if(msg.find(usernameToken) != 0 || msg.find(messageToken) == std::string::npos){
		return false; //if USER::USERNAME:: and USER::MESSAGE:: are not found don't process this
	}else{
		username = msg;
		username.erase(username.begin(), username.begin() + usernameToken.length()); //erase everything from the beginning to the end of USER::USERNAME::
		username.erase(username.find(messageToken), username.length()); //erase everything from the beginning of USER::MESSAGE:: to the end of the string

		users[clientNum].username = username; //what is left is the username of the client

		msg.erase(msg.begin(), msg.begin() + (msg.find(messageToken) + messageToken.length())); //erase from the beginning of the string to the end of USER::MESSAGE::
		
		return true;
	}
}

void forwardToAllUsers(std::string msg, int userNum){ //forwards to every user, other than the one specified by userNum
	for(int i = 0; i < users.size(); i++){
		if(i == userNum){ //userNum can be something like -1 so that the message is sent to every user
			continue;
		}

		if(users[i].loggedIn != true){
			continue;
		}

		sf::Packet packet; //a packet to hold a string
		packet << msg.c_str(); //putting the c style string into the packet

		users[i].socket->send(packet); //sending the packet
	}
}

void process(){
	std::time_t t; //current time
	std::tm* now; //object which processes into year, month, day, etc

	while(true){ //loop endlessly
		if(selector.wait(sf::seconds(1))){ //waits 1 second for any activity from any of the sockets
			for(int i = 0;i < users.size(); i++){ //loops through every connected socket
				if(selector.isReady(*(users[i].socket)) && users[i].loggedIn == true){ //checks through every socket in the selector, checking if it is ready for action (all sockets are in the selector so this is valid)
					sf::Packet receivePacket; //packet for message to be received
					std::string receiveString = ""; //the string for the packet
					std::string currentTime; //currentTime string
					std::string sendString; //string to send to users

					t = std::time(0); //gets current time
					now = std::localtime(&t); //makes it into an object which we can extract time from

					users[i].socket->receive(receivePacket); //receives packet

					if((receivePacket >> receiveString) && receiveString != ""){ //if the packet data can be extracted, and it is not empty
						if(processString(receiveString)){ //if this contains the ping flag which is to be received every few seconds, update the socket expiry time
							users[i].timeOfExpiry = sf::seconds(expiryTimer.getElapsedTime().asSeconds() + 5); //updates the socket expiry time
							continue; //then continue
						}

						if(!extractInformation(receiveString, i)){ //if username and message could not be extracted...
							continue; //then continue
						}

						//below basically formats the time, adding a 0 if a digit is less than 10 so it's in a standard form
						if(now->tm_hour < 10){currentTime += "0";}
						currentTime += std::to_string(now->tm_hour) + ":";

						if(now->tm_min < 10){currentTime += "0";}
						currentTime += std::to_string(now->tm_min) + ":";

						if(now->tm_sec < 10){currentTime += "0";}
						currentTime += std::to_string(now->tm_sec);

						sendString = "[" + currentTime + "] " + users[i].username + ": " + receiveString + "\n"; //the string with timestamp, username and message

						std::cout << sendString; //outputs the string

						forwardToAllUsers(sendString, i); //also sends to all users but the one which just sent this

						currentTime = ""; //empties this string
					}
				}
			}
		}

		for(int i = 0;i < users.size();i++){ //loops through every user...
			if(users[i].timeOfExpiry.asSeconds() <= expiryTimer.getElapsedTime().asSeconds()){ //checks if their socket's time of expiry is now or passed...
				users.erase(users.begin() + i); //removes them from the array if it is

				std::string msgString = "SERVER: A USER HAS LEFT\nSERVER: CLIENTS ONLINE: " + std::to_string(users.size()) + "\n"; //outputs some server info to indicate that the user is gone
				std::cout << msgString; //outputs this string

				forwardToAllUsers(msgString, -1); //sends it to all connected users, as the one this is about has now left
			}
		}
		
		sf::sleep(sf::milliseconds(50)); //slows down the process loop so less intensive on my poor laptop
	}
}

bool login(std::string input) {
	std::string usernameToken = "SERVER::LOGON::USERNAME::";
	std::string passwordToken = "SERVER::LOGON::PASSWORD::";
	std::string username = input;
	std::string password = input;

	if (input.find(usernameToken) == 0 && input.find(passwordToken) != std::string::npos) {
		std::fstream file;
		file.open("../html/erewhon.xyz/public_html/game/data.csv", std::ios::in);

		username.erase(username.begin(), (username.begin() + usernameToken.size()));
		username.erase(username.find(passwordToken), username.size());
		
		password.erase(password.begin(), password.begin() + password.find(passwordToken) + passwordToken.size());

		std::string line;
		while(std::getline(file, line)){
			if(line.find(username) != std::string::npos && line.find("unconfirmed") == std::string::npos){
				size_t posPass = line.find(password, line.find(username)+username.size()+1);
				line.erase(line.begin(), line.begin() + posPass);

				if(line == password){
					return true; //if username and password are found, return true
				}
			}
		}
	}
	
	return false; //otherwise return false
}

void server(unsigned short PORT){ //the function for server initialisation
	sf::TcpListener listener; //makes a tcpListener

	listener.listen(PORT); //listens on the provided port

	sf::TcpSocket* socket; //makes a socket pointer

	user* userPtr; //makes a user pointer

	std::cout << "SERVER: SERVER STARTED" << std::endl; //outputs that the server has started

	while(true){ //loop endlessly
		std::string outputString; //the string to alert connected users that another user has joined
		socket = new sf::TcpSocket; //a new socket in case a user joins
		userPtr = new user; //a new user struct in case a user joins

		if(listener.accept(*socket) == sf::Socket::Done){ //if a new user has joined...
			sf::Packet packet;

			socket->receive(packet);
			
			std::string receiveString;
			packet >> receiveString;

			sf::Packet sendPacket;

			if(!login(std::string(receiveString))){
				sendPacket << "SERVER::LOGON::RESPONSE:false";
				socket->send(packet);

				delete userPtr;
				delete socket;
				continue;
			}else{
				sendPacket << "SERVER::LOGON::RESPONSE::true";
				socket->send(sendPacket);

				outputString = "SERVER: NEW CONNECTION @ " + socket->getRemoteAddress().toString() + "\n"; //make a string which includes their IP address...
				forwardToAllUsers(outputString, -1); //forwards to all users, as the one just connected isn't in the users vector yet

				userPtr->socket = socket; //make the new user object contain their socket
				userPtr->timeOfExpiry = sf::seconds(expiryTimer.getElapsedTime().asSeconds() + 5); //set the expiry time for their socket
				userPtr->loggedIn = true;

				users.push_back(*userPtr); //add this user to the users vector

				selector.add(*socket); //add this vector to the selector

				std::cout << outputString;
			}
		}else{ //if a new user hasn't joined...
			delete userPtr; //delete the user struct
			delete socket; //and delete the socket
		}

		sf::sleep(sf::milliseconds(50)); //slows down the listener loop so less intensive on my poor laptop
	}
}
