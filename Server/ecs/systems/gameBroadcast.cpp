#include "../../header/ecs.h"
#include "../../header/helper.h"

using namespace ecs::system;
using namespace ecs::component;

void gameBroadcast::broadcastGameState()
{ //this broadcasts stuff on port 5001
	std::lock_guard<std::mutex> mutex(mutexs::removeUserMutex);	
	for (auto &chunkEntityVector : chunks)
	{
		for (auto &user : chunkEntityVector.second.second)
		{
			json jsonObj = json::object();
			int userCompVecIndex = users.entityToVectorMap(user.id);			   //user ID in this case is the entity ID
			int physicsCompVecIndex = physicsObjects.entityToVectorMap(user.id); //user ID in this case is the entity ID
			
			if(userCompVecIndex == -1){ //-1 indicates that it wasn't found
				continue;
			}

			if (users.compVec[userCompVecIndex].gameConnected == true)
			{
				if (selector.isReady(*(users.compVec[userCompVecIndex].gameSocket)))
				{
					sf::Vector2f currentChunk = {chunkCoordHelperX(int(physicsObjects.compVec[physicsCompVecIndex].coordinates.x), chunkPixelSize_x), chunkCoordHelperY(int(physicsObjects.compVec[physicsCompVecIndex].coordinates.y), chunkPixelSize_y)};
					sf::IpAddress userIP = users.compVec[userCompVecIndex].socket->getRemoteAddress();
					double size = gameData.size();
					jsonObj["chunks"][0] = gameData[coordinatesStruct(currentChunk.x - 1, currentChunk.y - 1)].dump();
					jsonObj["chunks"][1] = gameData[coordinatesStruct(currentChunk.x - 1, currentChunk.y)].dump();
					jsonObj["chunks"][2] = gameData[coordinatesStruct(currentChunk.x - 1, currentChunk.y + 1)].dump();
					jsonObj["chunks"][3] = gameData[coordinatesStruct(currentChunk.x, currentChunk.y - 1)].dump();
					jsonObj["chunks"][4] = gameData[coordinatesStruct(currentChunk.x, currentChunk.y)].dump();
					jsonObj["chunks"][5] = gameData[coordinatesStruct(currentChunk.x, currentChunk.y + 1)].dump();
					jsonObj["chunks"][6] = gameData[coordinatesStruct(currentChunk.x + 1, currentChunk.y - 1)].dump();
					jsonObj["chunks"][7] = gameData[coordinatesStruct(currentChunk.x + 1, currentChunk.y)].dump();
					jsonObj["chunks"][8] = gameData[coordinatesStruct(currentChunk.x + 1, currentChunk.y + 1)].dump();
					jsonObj["time"] = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); //attatches microsecond time

					sf::Packet packet;
					packet << jsonObj.dump();

					users.compVec[userCompVecIndex].gameSocket->send(packet);
				}
			}
		}
	}
}

void gameBroadcast::listenToUsers()
{ //this listens to stuff on port 5001
	while (true)
	{
		if (selector.wait(sf::seconds(1)))
		{
			for (user &userStruct : users.compVec)
			{
				if (userStruct.gameConnected && selector.isReady(*(userStruct.gameSocket)))
				{
					sf::Packet packet;
					std::string buffer;

					userStruct.gameSocket->receive(packet); //receive the data

					packet >> buffer;

					if (buffer.size() > 1)
					{
						json jsonObj = json::parse(buffer); //make into json object

						if (!jsonObj["sessionID"].is_null())
						{ //does the sessionID field exist in this json object?
							if (sessionIDToEntityID.count(jsonObj["sessionID"]))
							{ //does the sessionID belong to any active users?
								physics::getInstance()->userInput(jsonObj);
							}
						}
					}
				}
			}
		}
	}
}

void gameBroadcast::server()
{ //the function for server initialisation
	sf::TcpListener listenSocket;
	listenSocket.listen(5001);

	sf::TcpSocket *socket; //makes a socket pointer

	sf::Clock clock;

	while (true)
	{								//loop endlessly
		std::string outputString;   //the string to alert connected users that another user has joined
		socket = new sf::TcpSocket; //a new socket in case a user joins

		if (listenSocket.accept(*socket) == sf::Socket::Done)
		{ //if a new user has joined...
			sf::Packet packet;
			socket->receive(packet);

			std::string receiveString;
			packet >> receiveString;

			sf::Packet sendPacket;
			std::cout << "NEW USER: " << socket->getRemoteAddress() << " -- " << receiveString << std::endl;

			if (receiveString != "")
			{
				if (int(sessionIDToEntityID[receiveString]) > -1)
				{
					users.compVec[users.entityToVectorMap(sessionIDToEntityID[receiveString])].gameSocket = socket;
					selector.add(*socket); //add this vector to the selector

					sendPacket << "true"; //successful login
					socket->send(sendPacket);
					users.compVec[users.entityToVectorMap(sessionIDToEntityID[receiveString])].gameConnected = true;
				}
				else
				{
					sendPacket << "false"; //unsuccessful login
					socket->send(sendPacket);
				}
			}
		}
		else
		{
			delete socket;
		}
	}
}