#include <iostream>
#include <string>
#include "SFML/Network.hpp"

//prototypes
void client(std::string IPADDRESS, unsigned short PORT);
void getInput(std::string &msg);
void getResponses();
void stayAlive();

sf::Clock keepAliveTimer; //the clock which is used to check when to ping the server
sf::Time pingTime; //the time at which a ping should occur
sf::TcpSocket socket; //makes a new socket
std::string username; //the name of this user
std::string password;
bool loggedIn = false;
bool serverResponse = false;

int main() {
	const std::string IPADDRESS("erewhon.xyz"); //the IP address to connect to
	const unsigned short PORT = 5000; //the port to connect to

	std::string msg; //the string used in getInput(...)

	sf::Thread* pingThread = 0; //thread for pinging the server
	sf::Thread* receiveThread = 0; //thread for waiting for incoming messages

	std::cout << "Enter username: "; //asks for username
	getline(std::cin, username); //and makes it the value of the global variable defined earlier

	std::cout << "Enter password: "; //asks for password
	getline(std::cin, password); //and makes it the value of the global variable defined earlier

	client(IPADDRESS, PORT); //initialises the client

	receiveThread = new sf::Thread(&getResponses); //make the getResponses() function run on this thread
	receiveThread->launch(); //launches the thread

	pingThread = new sf::Thread(&stayAlive); //makes the stayAlive() function run on this thread
	pingThread->launch(); //launches the thread

	std::cout << "Loading...";
	while (!serverResponse) {
		sf::sleep(sf::seconds(1));
		std::cout << ".";
	}
	std::cout << std::endl;

	getInput(msg); //runs the getInput(...) function endlessly

	if (pingThread) { //if the ping thread exists...
		pingThread->wait(); //wait for it's current bit of code to finish
		delete pingThread; //then delete it
	}

	if (receiveThread) { //if the receiveThread exists...
		receiveThread->wait(); //wait for it's current bit of code to finish
		delete receiveThread; //then delete it
	}

	return 0;
}

void getResponses() { //retrieves incoming message
	while (true) { //loop endlessly
		sf::Packet receivePacket; //a packet variable to hold the incoming packet
		char* receiveCharArray = NULL; //a char pointer to hold what will be a dynamically allocated char array

		socket.receive(receivePacket); //receives the packet

		receiveCharArray = new char[receivePacket.getDataSize()]; //makes a new char array the exact size of the data from the just received packet
		receivePacket >> receiveCharArray; //puts the packet's data into the char array variable we just put
		//need to do this as all of the data transmitted from the server was string with .c_str() applied to it, so was a C style string (char array)

		std::string receiveString(receiveCharArray);
		std::string removeFlag = "SERVER::LOGON::RESPONSE::";

		if (receiveString.find(removeFlag) == 0) {
			receiveString.erase(receiveString.begin(), receiveString.begin() + removeFlag.length());

			if (receiveString == "true") {
				serverResponse = true;
				loggedIn = true;
			}
			else {
				loggedIn = false;
				serverResponse = true;

				socket.disconnect();

				break;
			}
		}
		else {
			std::cout << receiveCharArray; //outputs the message which was received
		}

		delete receiveCharArray; //then deletes the char array
	}
}

void stayAlive() { //for pinging the server
	while (true) { //loop endlessly
		sf::Packet sendPacket; //makes a packet which will contain the ping flag

		if (keepAliveTimer.getElapsedTime().asSeconds() >= pingTime.asSeconds()) { //pings every ~3 seconds
			pingTime = sf::seconds(keepAliveTimer.getElapsedTime().asSeconds() + 3); //then updates the time of the next ping to be in 3 seconds

			sendPacket << "SERVER::PING::3SEC"; //puts this string into the packet
			socket.send(sendPacket); //then sends it off to the server
		}
	}
}

void getInput(std::string &msg) { //gets input from the user
	std::cout << "Enter a message: " << std::endl; //prompted to enter a message once

	while (loggedIn == true) { //loop endlessly
		sf::Packet sendPacket; //the packet which will contain the data to send

		getline(std::cin, msg); //gets the string to send and puts it in the msg variable declared in int main()

		if (msg != "") { //if the message isn't empty
			msg = "USER::USERNAME::" + username + "USER::MESSAGE::" + msg; //puts USER::USERNAME:: before appending the chosen username and USER::MESSAGE:: before appending the message
			//this will help with decoding the data on the server side

			sendPacket << msg.c_str(); //converts the string into a C style array, and puts it into the packet which will be sent

			socket.send(sendPacket); //sends the packet
		}
	}
}

void client(std::string IPADDRESS, unsigned short PORT) { //for connecting to the server
	while (true) {
		if (socket.connect(IPADDRESS.c_str(), PORT) == sf::Socket::Done) { //if the socket successfully connects to the specified IP address at the specified port...
			//the IP address has to be a C style array as well
			std::cout << "Connected to " << IPADDRESS << ":" << PORT << std::endl; //output that it has connected to that IP address and that port
			
			sf::Packet logonPacket;
			std::string msg = "SERVER::LOGON::USERNAME::" + username + "SERVER::LOGON::PASSWORD::" + password;
			
			logonPacket << msg.c_str();

			socket.send(logonPacket); //sends the username and password to the server for authentication

			break; //and break the loop
		}
		//otherwise it will loop endlessly until it does connect
	}
}
