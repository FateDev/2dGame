#include <game.h>

void gameNetwork::listenData() {
	sf::Packet packet;
	sf::IpAddress sender;
	unsigned short port;

	while (true) {
		socket.receive(packet);
		std::string output = "";
		packet >> output;

		json jsonObj = json::parse(output);

		if (gameReference != 0) {
			//jsonMutex.lock(); //will lock the gameData resource
			std::cout << jsonObj["chunks"][4].get<std::string>() << std::endl;
			//std::cout << "x: " << chunkToDraw["entities"][0]["location"]["x"].get<float>() * 10 << " -- y: " << chunkToDraw["entities"][0]["location"]["y"].get<float>() * 10 << std::endl << std::endl;
			gameReference->gameData = json::parse(output);
			gameReference->draw();
			//jsonMutex.unlock(); //deconstructs the lock gaurd, thereby unlocking the resource
		}
	}
}

void gameNetwork::sendData(json payload) {
	std::string payloadDump = payload.dump();
	sf::Packet packet;
	packet << payloadDump;
	socket.send(packet); //makes the json into a string, then 
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