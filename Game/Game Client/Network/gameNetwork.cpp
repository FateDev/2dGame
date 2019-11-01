#include <game.h>

void gameNetwork::listenData() {
	sf::UdpSocket socket;
	socket.bind(localListenPort);

	while (true) {
		sf::Packet packet;
		sf::IpAddress sender;
		unsigned short port;

		socket.receive(packet, sender, port);
		std::string output = "";
		packet >> output;

		//std::cout << sender.toString() << " said: " << output << std::endl;
	}

	socket.unbind();
}

void gameNetwork::sendData(json payload) {
	std::string payloadDump = payload.dump();
	socket.send(payloadDump.c_str(), payloadDump.size() + 1, remoteIP, sendPort); //makes the json into a string, then 
}