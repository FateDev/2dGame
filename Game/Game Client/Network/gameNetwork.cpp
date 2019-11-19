#include <game.h>

void gameNetwork::listenData() {
	sf::Packet packet;
	sf::IpAddress sender;
	unsigned short port;
	
	while (true) {
		socket.receive(packet, sender, port);
		std::string output = "";
		packet >> output;

		json jsonObj = json::parse(output);

		if(unixMicrosecondsOfLastPacket == 0){
			unixMicrosecondsOfLastPacket = jsonObj["time"].get<long long>();
		}

		if (gameReference != 0) {
			jsonMutex.lock(); //will lock the gameData resource
			unixMicrosecondsOfLastPacket = jsonObj["time"].get<long long>();
			gameReference->gameData = json::parse(output);
			jsonMutex.unlock(); //deconstructs the lock gaurd, thereby unlocking the resource
		}
	}

	/*
	//std::cout << sender.toString() << " said: " << output << std::endl;

	json jsonObj = json::parse(output);

	if (jsonObj.is_null()) {
		std::cout << "null" << std::endl;
	}

	json chunkCenter = json::parse(jsonObj["center-center"].get<std::string>());
	std::cout << chunkCenter["entities"][0]["name"].get<std::string>() << ": " << chunkCenter["entities"][0]["location"]["x"].get<float>() << " -- " << chunkCenter["entities"][0]["location"]["y"].get<float>() << std::endl;
	//std::cout << jsonObj["center-center"].dump() << std::endl;
	*/
}

void gameNetwork::sendData(json payload) {
	std::string payloadDump = payload.dump();
	socket.send(payloadDump.c_str(), payloadDump.size() + 1, remoteIP, sendPort); //makes the json into a string, then 
}