#include <iostream>
#include <SFML/Network.hpp>

const unsigned int PORT = 5000;
const std::string reserved = "#";
const std::string delmiter = "####";
const std::string hash = "{hash}";

std::string msg;
std::string userName;
sf::TcpSocket socket;

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
		sf::Packet packetReceive;
		std::string receiveMsg;

		socket.receive(packetReceive);

		if((packetReceive >> receiveMsg) && !receiveMsg.empty()){
			extractInfo(receiveMsg);
			
			std::cout << userName << ": " << msg << std::endl;
		}
	}
}

void server(void){
	sf::TcpListener listener;
	listener.listen(PORT);
	listener.accept(socket);
	std::cout <<  "New client connected: " << socket.getRemoteAddress() << std::endl;
}

int main(){
	std::cout << " ** SERVER STARTED ** " << std::endl;
	server();

	processLoop();
}