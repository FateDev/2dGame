#include <iostream>
#include <vector>
#include <ctime>
#include <fstream>
#include <SFML/Network.hpp>
#include <curl/curl.h>

struct user{ //struct to hold user's sockets, usernames, and the time left until their socket expires
	sf::TcpSocket* socket;
	int userID;
	std::string username;
	sf::Time timeOfExpiry;
	bool loggedIn;
	std::string roomGuild = "main.alpha";
	std::string tempNewestMsgID = "";
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
bool extractInformation(std::string &msg);
void process();
size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp);
void saveMsgDB(std::string msg, int userNum, int time);
bool checkLeave(std::string &msg);
void forwardToAllUsers(std::string msg, int userNum);
bool login(std::string input, user* userPtr);

int main(){
	curl_global_init(CURL_GLOBAL_ALL); //initialise libcurl functionality globally
	
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

void saveMsgDB(std::string msg, int userNum, int time){
	CURL* curl = curl_easy_init(); //we can set options for this to make it control how a transfer/transfers will be made
	std::string readBuffer; //string for the returning data
	
	int id = users[userNum].userID;
	msg = curl_easy_escape(curl, msg.c_str(), msg.length());
	std::string roomGuild = users[userNum].roomGuild;
	
	curl_easy_setopt(curl, CURLOPT_URL, ("https://erewhon.xyz/game/serverMessages.php?save=true&id="+std::to_string(id)+"&msg="+msg+"&roomGuild="+roomGuild+"&time="+std::to_string(time)).c_str());
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); //the callback
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); //will write data to the string, so the fourth param of the last callback is stored here

	curl_easy_perform(curl);

	users[userNum].tempNewestMsgID = readBuffer;
}

bool processString(std::string msg){ //for checking if the server has been pinged by a socket
	if(msg.find("SERVER::PING::3SEC") == 0){
		return true;
	}else{
		return false;
	}
	std::cout << msg << std::endl;
}

bool checkLeave(std::string &msg){
	std::string leaveToken = "USER::LEAVE";

	if(msg.find(leaveToken) == 0){
		return true; //user sent the leave token
	}else{
		return false; //user didn't send the leave token
	}
}

bool extractInformation(std::string &msg){ //extracts username and the message
	std::string usernameToken = "USER::USERNAME::";
	std::string messageToken = "USER::MESSAGE::";

	if(msg.find(usernameToken) != 0 || msg.find(messageToken) == std::string::npos){
		return false; //if USER::USERNAME:: and USER::MESSAGE:: are not found don't process this
	}else{
		msg.erase(msg.begin(), msg.begin() + (msg.find(messageToken) + messageToken.length())); //erase from the beginning of the string to the end of USER::MESSAGE::
		
		return true;
	}
}

void forwardToAllUsers(std::string msg, int userNum){ //forwards to every user
	for(int i = 0; i < users.size(); i++){
		if(users[i].loggedIn != true){
			continue;
		}

		if(users[i].roomGuild == users[userNum].roomGuild){
			sf::Packet packet; //a packet to hold a string
			packet << msg.c_str(); //putting the c style string into the packet

			users[i].socket->send(packet); //sending the packet
		}
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

						if(checkLeave(receiveString)){
							sf::Packet packet; //a packet to hold a string
							packet << std::string("SERVER::DIE").c_str(); //putting the c style string into the packet
							users[i].socket->send(packet); //sending the packet
							
							users.erase(users.begin() + i); //removes them from the array if it is

							std::string msgString = "SERVER: A USER HAS LEFT\nSERVER: CLIENTS ONLINE: " + std::to_string(users.size()) + "\n"; //outputs some server info to indicate that the user is gone
							std::cout << msgString; //outputs this string
						}

						if(receiveString.find("USER::CHANGEROOMGUILD::") == 0){
							receiveString.erase(receiveString.begin(), receiveString.begin() + (receiveString.find("USER::CHANGEROOMGUILD::") + std::string("USER::CHANGEROOMGUILD::").length()));

							users[i].roomGuild = receiveString;
							std::cout << receiveString << std::endl;
							continue;
						}

						if(!extractInformation(receiveString)){ //if username and message could not be extracted...
							continue; //then continue
						}

						saveMsgDB(receiveString, i, t);

						std::string time = std::to_string(t);

						sendString += "USER::ID::" + users[i].tempNewestMsgID; //USER::ID is actually the message ID
						sendString += "USER::USERNAME::" + users[i].username;
						sendString += "USER::TIME::" + time;
						sendString += "USER::MSG::" + receiveString;

						std::cout << sendString << std::endl;

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
			}
		}
		
		sf::sleep(sf::milliseconds(50)); //slows down the process loop so less intensive on my poor laptop
	}
}

size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp) //first one is pointer to data, second is to the size of one data item,
{ //third is how many there are, so size * nmemb is data size, and fourth is pointer to the data you get back, which you actually access using CURLOPT_WRITEDATA and a reference
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

bool login(std::string input, user* userPtr) {
	std::string usernameToken = "SERVER::LOGON::USERNAME::";
	std::string passwordToken = "SERVER::LOGON::PASSWORD::";
	std::string username = input;
	std::string password = input;

	if (input.find(usernameToken) == 0 && input.find(passwordToken) != std::string::npos) {
		username.erase(username.begin(), (username.begin() + usernameToken.size()));
		username.erase(username.find(passwordToken), username.size());
		
		password.erase(password.begin(), password.begin() + password.find(passwordToken) + passwordToken.size());
		//the above basically extracts the username and password

		//std::cout << "Username: " << username << " - Password: " << password << std::endl;
		
		CURL* curl = curl_easy_init(); //we can set options for this to make it control how a transfer/transfers will be made
		std::string readBuffer; //string for the returning data
		
		username = curl_easy_escape(curl, username.c_str(), username.length());
		password = curl_easy_escape(curl, password.c_str(), password.length());
		
		curl_easy_setopt(curl, CURLOPT_URL, ("http://erewhon.xyz/game/serverLogin.php?username="+username+"&password="+password).c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); //the callback
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); //will write data to the string, so the fourth param of the last callback is stored here

		curl_easy_perform(curl);
		
		if(readBuffer.find("USER::ID::") != 0 || readBuffer.find("USER::ID::") == std::string::npos){
			return false; //login failed if no ID found
		} else {
			readBuffer.erase(readBuffer.begin(), readBuffer.begin() + std::string("USER::ID::").length());
			userPtr->userID = std::stoi(readBuffer);
			userPtr->username = username;

			return true;
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
			//std::cout << "NEW USER" << std::endl;
			sf::Packet packet;

			socket->receive(packet);
			
			std::string receiveString;
			packet >> receiveString;

			sf::Packet sendPacket;

			if(!login(std::string(receiveString), userPtr)){
				sendPacket << "SERVER::LOGON::RESPONSE::false";
				socket->send(sendPacket);

				delete userPtr;
				delete socket;
				continue;
			}else{
				sendPacket << "SERVER::LOGON::RESPONSE::true";
				socket->send(sendPacket);

				outputString = "SERVER: NEW CONNECTION @ " + socket->getRemoteAddress().toString() + "\n"; //make a string which includes their IP address...

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