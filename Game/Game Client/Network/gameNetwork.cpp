#include <game.h>

void gameNetwork::listenData() {
	sf::Packet packet;
	sf::IpAddress sender;
	unsigned short port;

	while (!exiting) {
		socket.receive(packet);
		std::string output = "";
		packet >> output;

		if (output == "die" && gameReference != nullptr) {
			gameReference->networkObj->active = false; //will break out of all loops and die
			break;
		}

		if (gameReference != 0 && gameReference != nullptr && output.size() > 0) {
			exitMutex.lock();
			//std::cout << jsonObj["chunks"][4].get<std::string>() << std::endl;
			gameReference->gameData = json::parse(output);
			gameReference->draw();
			exitMutex.unlock();
		}
	}
}

void gameNetwork::sendData(json payload) {
	std::string payloadDump = payload.dump();
	sf::Packet packet;
	packet << payloadDump;
	socket.send(packet);
}

gameNetwork::gameNetwork(std::string ip, unsigned short port, std::string sessionID) {
	if(socket.connect(ip.c_str(), port) == sf::Socket::Done){
		sf::Packet packet;
		packet << sessionID;
		socket.send(packet);

		std::string output;

		sf::Packet receivePacket;


		//std::cout << packet.getData() << " -- " << sessionID << std::endl;
		socket.receive(receivePacket);

		receivePacket >> output;

		if (output != "true") {
			success = false;
		}
		else {
			success = true;
		}
	}
}