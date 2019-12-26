#include "../header/ecs.h"
#include "../header/helper.h"

#include <cmath>
#include <functional>
#include <SFML/Network.hpp>
#include <curl/curl.h>
#include <chrono>

using namespace ecs::system;
using namespace ecs::component;

int chunkCoordHelper(float coord, float screenSize){
	return div(coord, screenSize).quot;
}

void systemsManager::systemStart()
{
	processNetwork = new sf::Thread(&network::process, &networkObj); //makes the thread
	processNetwork->launch();										 //launches it

	listenNetwork = new sf::Thread(std::bind(&network::server, &networkObj, port)); //launches the server to listen on that specific port
	listenNetwork->launch();														//launches it

	gameListen = new sf::Thread(&gameBroadcast::listenToUsers, gameBroadcast::getInstance());
	gameListen->launch();

	gameConnectServer = new sf::Thread(&gameBroadcast::server, gameBroadcast::getInstance());
	gameConnectServer->launch();

	mainGame = new sf::Thread(&game::runGame, game::getInstance());
	mainGame->launch();

	coordinatesStruct startCoord(0, 0);
	chunks[startCoord].first.settingID = 1; //sets the first chunk's setting ID to 1, which is a city
	unsigned int entityID = ecs::entity::superEntityManager.create({components::DRAWABLE, components::PHYSICAL}); //a new object with those attributes is made
	physicsObjects.compVec[physicsObjects.entityToVectorMap(entityID)].boxCorners = {
		sf::Vector2f(startCoord.coordinates.first * chunkPixelSize_x, (startCoord.coordinates.second * chunkPixelSize_y) + chunkPixelSize_y - 5), 
		sf::Vector2f((startCoord.coordinates.first * chunkPixelSize_x) + chunkPixelSize_x, (startCoord.coordinates.second * chunkPixelSize_y) + chunkPixelSize_y  - 5),
		sf::Vector2f(startCoord.coordinates.first * chunkPixelSize_x, (startCoord.coordinates.second * chunkPixelSize_y) + chunkPixelSize_y ), 
		sf::Vector2f((startCoord.coordinates.first * chunkPixelSize_x) + chunkPixelSize_x, (startCoord.coordinates.second * chunkPixelSize_y) + chunkPixelSize_y )
	};  //sets the corners of these boxes
	physicsObjects.compVec[physicsObjects.entityToVectorMap(entityID)].objType = COLLISION; //sets the object type
	chunks[startCoord].second.push_back(entityID); //pushes the floor entity to the chunks object
}

void systemsManager::systemEnd()
{
	if (processNetwork)
	{							//if a thread exists
		processNetwork->wait(); //wait until it's completed this bit of code
		delete processNetwork;  //delete it, by the way as sever(...) has an infinite while loop inside, this is only really called when quitting the program
	}

	if (listenNetwork)
	{						   //if a thread exists
		listenNetwork->wait(); //wait until it's completed this bit of code
		delete listenNetwork;  //delete it, by the way as sever(...) has an infinite while loop inside, this is only really called when quitting the program
	}
}

void network::removeUser(unsigned int i)
{											
	std::lock_guard<std::mutex> mutex(mutexs::removeUserMutex);	//function to basically properly log out a user
	
	unsigned int entityID = users.vectorToEntityMap(i);	//get the user entityID
	sf::Vector2f coordinates = physicsObjects.compVec[physicsObjects.entityToVectorMap(entityID)].coordinates; //get the user's coordinates
	ecs::system::coordinatesStruct removeUserCoordinates(chunkCoordHelper(int(coordinates.x), chunkPixelSize_x), chunkCoordHelper(int(coordinates.y), chunkPixelSize_y)); //get what their coordinates would be in the world using simple mod math

	int counter = 0;
	for(int i = 0; i < chunks[removeUserCoordinates].second.size(); i++){
		if(chunks[removeUserCoordinates].second[i].id == entityID){
			chunks[removeUserCoordinates].second.erase(chunks[removeUserCoordinates].second.begin() + i);
			break;
		}
	}
	
	chunks[removeUserCoordinates].first.userCount--; //decrements the number of users in this chunk
	//std::cout << "decrementing user count (removing) \n";

	sf::Packet packet; //a packet to hold a string
	packet << std::string("die").c_str(); //putting the c style string into the packet
	users.compVec[i].socket->send(packet); //sending the packet, this will cause the client to end if it hasn't already
	users.compVec[i].gameSocket->send(packet); //sending the packet, this will cause the client to end if it hasn't already

	sessionIDToEntityID.erase(users.compVec[i].sessionID); //will erase the mapping of the unique session ID to the index in the component vector for this user

	ecs::entity::superEntityManager.destroy(entity::entity(entityID), ecs::entity::USER); //removes them from the array if it is

	updateActiveChunkData::getInstance()->updateActiveChunks(); //updates the active chunks

	std::string msgString = "SERVER: A USER HAS LEFT\nSERVER: CLIENTS ONLINE: " + std::to_string(users.compVec.size()); //outputs some server info to indicate that the user is gone
	std::cout << msgString << "\n";	//outputs this string
}

void network::forwardToAllUsers(std::string msg, int userNum)
{ //forwards to every user
	for (int i = 0; i < users.compVec.size(); i++)
	{
		if (users.compVec[i].loggedIn != true)
		{
			continue;
		}

		if (users.compVec[i].roomGuild == users.compVec[userNum].roomGuild)
		{
			sf::Packet packet;	 //a packet to hold a string
			packet << msg.c_str(); //putting the c style string into the packet

			users.compVec[i].socket->send(packet); //sending the packet
		}
	}
}

void network::broadcastToLocal(std::string msg, int userNum)
{ //this broadcasts to users within 100 units of the player (hence the 100 in the if statement below)
	auto userChunk = coordinatesStruct(physicsObjects.compVec[physicsObjects.entityToVectorMap(users.vectorToEntityMap(userNum))].coordinates.x, physicsObjects.compVec[physicsObjects.entityToVectorMap(users.vectorToEntityMap(userNum))].coordinates.y);
	for(int x = userChunk.coordinates.first-1; x <= userChunk.coordinates.first+1; x++){
		for(int y = userChunk.coordinates.second-1; y <= userChunk.coordinates.second+1; y++){
			auto currentChunk = coordinatesStruct(x, y);
			for(auto user : chunks[currentChunk].second){
				if(users.compVec[users.entityToVectorMap(user.id)].roomGuild == users.compVec[userNum].roomGuild){
					sf::Packet packet;	 //a packet to hold a string
					packet << msg.c_str(); //putting the c style string into the packet

					users.compVec[users.entityToVectorMap(user.id)].socket->send(packet); //sends the packet to the user currently being looped over
					std::cout << "sending" << std::endl;
				}
			}
		}
	}
}

void network::saveMsgDB(std::string msg, int userNum, int time)
{
	CURL *curl = curl_easy_init(); //we can set options for this to make it control how a transfer/transfers will be made
	std::string readBuffer;		   //string for the returning data

	int id = users.compVec[userNum].userID;
	msg = curl_easy_escape(curl, msg.c_str(), msg.length());
	std::string roomGuild = users.compVec[userNum].roomGuild;

	curl_easy_setopt(curl, CURLOPT_URL, ("https://erewhon.xyz/game/serverMessages.php?save=true&id=" + std::to_string(id) + "&msg=" + msg + "&roomGuild=" + roomGuild + "&time=" + std::to_string(time)).c_str());
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); //the callback
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);		  //will write data to the string, so the fourth param of the last callback is stored here

	curl_easy_perform(curl);

	users.compVec[userNum].tempNewestMsgID = readBuffer;
}

void network::process()
{
	std::time_t t; //current time
	std::tm *now;  //object which processes into year, month, day, etc

	while (true)
	{ //loop endlessly
		if (selector.wait(sf::seconds(1)))
		{ //waits 1 second for any activity from any of the sockets
			for (int i = 0; i < users.compVec.size(); i++)
			{ //loops through every connected socket
				if (selector.isReady(*(users.compVec[i].socket)) && users.compVec[i].loggedIn)
				{ //checks through every socket in the selector, checking if it is ready for action (all sockets are in the selector so this is valid)
					sf::Packet receivePacket;		//packet for message to be received
					std::string receiveString = ""; //the string for the packet
					std::string currentTime;		//currentTime string
					std::string sendString;			//string to send to users

					t = std::time(0);		  //gets current time
					now = std::localtime(&t); //makes it into an object which we can extract time from

					users.compVec[i].socket->receive(receivePacket); //receives packet

					if ((receivePacket >> receiveString) && receiveString != "")
					{ //if the packet data can be extracted, and it is not empty
						if (processString(receiveString))
						{ //if this contains the ping flag which is to be received every few seconds, update the socket expiry time
							users.compVec[i].timeOfExpiry = sf::seconds(expiryTimer.getElapsedTime().asSeconds() + 5); //updates the socket expiry time
							continue; //then continue, to the next loop
						}

						if (checkLeave(receiveString))
						{
							users.compVec[i].leave = true;
							break;
							/*
							removeUser(i);
							break; //maybe causing issue (crashing other clients)*/
						}

						if (receiveString.find("USER::CHANGEROOMGUILD::") == 0)
						{
							receiveString.erase(receiveString.begin(), receiveString.begin() + (receiveString.find("USER::CHANGEROOMGUILD::") + std::string("USER::CHANGEROOMGUILD::").length()));

							users.compVec[i].roomGuild = receiveString;

							continue;
						}

						if (!extractInformation(receiveString))
						{			  //if username and message could not be extracted...
							continue; //then continue
						}

						//below is basically saying to set the local message id to 0 if LOCALCHAT, you'd only use it to make sure your messages aren't recorded whatsoever
						if (users.compVec[i].roomGuild != "LOCALCHAT")
						{
							saveMsgDB(receiveString, i, t);
						}
						else
						{
							users.compVec[i].tempNewestMsgID = "-1"; //ID is not 0
						}

						std::string time = std::to_string(t);

						sendString += "MSG::ID::" + users.compVec[i].tempNewestMsgID;
						sendString += "USER::USERNAME::" + users.compVec[i].username;
						sendString += "USER::TIME::" + time;
						sendString += "USER::MSG::" + receiveString;

						std::cout << sendString << "\n";

						if (users.compVec[i].roomGuild != "LOCALCHAT")
						{
							forwardToAllUsers(sendString, i); //also sends to all users but the one which just sent this
						}
						else
						{
							broadcastToLocal(sendString, i);
						}

						currentTime = ""; //empties this string
					}
				}
			}
		}

		for (int i = 0; i < users.compVec.size(); i++)
		{ //loops through every user...
			if (users.compVec[i].timeOfExpiry.asSeconds() <= expiryTimer.getElapsedTime().asSeconds())
			{ //checks if their socket's time of expiry is now or passed...
				//std::cout << "expiry timer issue, for user " << users.compVec[i].username << std::endl;
				users.compVec[i].leave = true;
			}
		}

		for(int i = 0; i < users.compVec.size(); i++){
			if(users.compVec[i].leave){
				//std::cout << "Removing user: " << users.compVec[i].username << std::endl;
				removeUser(i);
			}
		}

		sf::sleep(sf::milliseconds(50)); //slows down the process loop so less intensive on my poor laptop
	}
}

std::string network::login(std::string input, user *userPtr)
{
	std::string usernameToken = "SERVER::LOGON::USERNAME::";
	std::string username = input;

	if (input.find(usernameToken) == 0 != std::string::npos)
	{
		username.erase(username.begin(), (username.begin() + usernameToken.size()));

		CURL *curl = curl_easy_init(); //we can set options for this to make it control how a transfer/transfers will be made
		std::string readBuffer;		   //string for the returning data

		for (int i = 0; i < users.compVec.size(); i++)
		{ //makes sure the same user can't join twice
			if (users.compVec[i].username == username)
			{
				return "failed";
			}
		}

		username = curl_easy_escape(curl, username.c_str(), username.length());

		curl_easy_setopt(curl, CURLOPT_URL, ("http://erewhon.xyz/game/serverLogin.php?username=" + username).c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); //the callback
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);		  //will write data to the string, so the fourth param of the last callback is stored here

		curl_easy_perform(curl);

		if (readBuffer.find("AVATAR::") != 0 || readBuffer.find("AVATAR::") == std::string::npos || readBuffer.find("USER::ID::") == std::string::npos)
		{
			return "failed"; //login failed if no ID found
		}
		else
		{
			//the below will extract the user's avatar's name
			std::string avatar = readBuffer;
			avatar.erase(0, avatar.find("AVATAR::") + std::string("AVATAR::").size());
			avatar.erase(avatar.find("USER::ID::"), avatar.size());

			//the below will erase all irrelavent information and extract the user ID
			readBuffer.erase(0, readBuffer.find("USER::ID::") + std::string("USER::ID::").length());
			userPtr->userID = std::stoi(readBuffer);
			userPtr->username = username;

			return avatar;
		}
	}

	return "failed"; //otherwise return false
}

void network::server(unsigned short PORT)
{							  //the function for server initialisation
	sf::TcpListener listener; //makes a tcpListener

	listener.listen(PORT); //listens on the provided port

	sf::TcpSocket *socket; //makes a socket pointer

	user *userPtr; //makes a user pointer

	std::cout << "SERVER: SERVER STARTED" << std::endl; //outputs that the server has started
	//unsigned int entityID2 = ecs::entity::superEntityManager.create({components::USER, components::LOCATION});
	//std::cout << entityID2 << std::endl;

	sf::Clock clock; //used to generate some 'unique' session ID

	ecs::system::coordinatesStruct startCoord(0, 0); //the starting coordinates
	ecs::entity::entity tempEntity;					 //used to put the users entity inside some chunk

	while (true)
	{								//loop endlessly
		std::string outputString;   //the string to alert connected users that another user has joined
		socket = new sf::TcpSocket; //a new socket in case a user joins
		userPtr = new user;			//a new user struct in case a user joins

		if (listener.accept(*socket) == sf::Socket::Done)
		{ //if a new user has joined...
			//std::cout << "NEW USER" << std::endl;
			sf::Packet packet;

			socket->receive(packet);

			std::string receiveString;
			packet >> receiveString;

			sf::Packet sendPacket;

			std::string response = login(std::string(receiveString), userPtr); //this will either contain the user's avatar's name, or it will be "failed" in the event that it failed to log in
			if (response == "failed")
			{
				sendPacket << "SERVER::LOGON::RESPONSE::falseUSER::ID::-1";
				socket->send(sendPacket);

				delete userPtr;
				delete socket;
				continue;
			}
			else
			{
				userPtr->sessionID = userPtr->username + std::to_string(clock.getElapsedTime().asMicroseconds());

				sendPacket << std::string("SERVER::LOGON::RESPONSE::trueUSER::SESSION_ID::" + userPtr->sessionID + "USER::ID::" + std::to_string(userPtr->userID)).c_str();
				socket->send(sendPacket);

				outputString = "SERVER: NEW CONNECTION @ " + socket->getRemoteAddress().toString() + "\n"; //make a string which includes their IP address...

				unsigned int entityID = ecs::entity::superEntityManager.create({components::USER, components::DRAWABLE, components::PHYSICAL}); //a new object with those attributes is made

				tempEntity.id = entityID; //sets the temp entity to have the correct ID
				//tempEntity.type = ecs::entity::USER; //sets the temp entity to have the correct ID
				ecs::system::chunks[startCoord].second.push_back(tempEntity); //pushes users to the (0, 0) chunk
				chunks[startCoord].first.userCount++; //increments the number of users in this chunk

				updateActiveChunkData::getInstance()->updateActiveChunks(); //updates the active chunks
				
				unsigned int componentVectorIndex = users.entityToVectorMap(entityID);	//gets the component vector index of this entity
				unsigned int physicsCompVecIndex = physicsObjects.entityToVectorMap(entityID);

				physicsObjects.compVec[physicsCompVecIndex].boxCorners = {sf::Vector2f(0, -7.5), sf::Vector2f(6, -7.5), sf::Vector2f(0, 0), sf::Vector2f(6, 0)}; //the bounding box for a user character
				physicsObjects.compVec[physicsCompVecIndex].coordinates.y = -5; //they start off at this height (floor is below them)

				userPtr->socket = socket;														   //make the new user object contain their socket
				userPtr->timeOfExpiry = sf::seconds(expiryTimer.getElapsedTime().asSeconds() + 5); //set the expiry time for their socket
				userPtr->loggedIn = true;
				userPtr->avatar = response;

				users.compVec[componentVectorIndex] = *userPtr;									 //add this user to the users vector
				sessionIDToEntityID.insert({userPtr->sessionID, entityID}); //will add a map entry, mapping their unique session ID to some index in the component vector

				selector.add(*socket); //add this vector to the selector

				std::cout << outputString;
			}
		}
		else
		{ //if a new user hasn't joined...
			delete userPtr; //delete the user struct
			delete socket;  //and delete the socket
		}

		sf::sleep(sf::milliseconds(150)); //slows down the listener loop so less intensive on my poor laptop
	}
}

physics *physics::instance = 0; //to make it singleton
physics::physics() {}			//basically default constructor

physics *physics::getInstance()
{
	if (instance == 0)
	{
		instance = new physics();
	}
	return instance;
}

void physics::userInput(json keysAndID)
{
	std::lock_guard<std::mutex> lock(mutexs::userLocationsMutex); //will block attempts to lock this mutex again, thereby allowing us to prevent accidentally accessing shared data at the wrong time, released at the end of scope

	unsigned short entityID = sessionIDToEntityID[keysAndID["sessionID"]];
	unsigned short physicsVectorIndex = physicsObjects.entityToVectorMap(entityID);
	unsigned short drawablesVectorIndex = drawables.entityToVectorMap(entityID);

	sf::Vector2f *userVelocity = &ecs::component::physicsObjects.compVec[physicsVectorIndex].velocity;
	bool *onFloor = &ecs::component::physicsObjects.compVec[physicsVectorIndex].onFloor;
	sf::Vector2i *direction = &ecs::component::drawables.compVec[drawablesVectorIndex].direction;

	if ((keysAndID["left"] && keysAndID["right"]) || (!keysAndID["left"] && !keysAndID["right"]))
	{ //if both or neither of them are pressed, the velocity is zero
		userVelocity->x = 0;
	}
	else
	{
		if (keysAndID["left"]){
			userVelocity->x = -velocity.x;
		}
		if (keysAndID["right"]){
			userVelocity->x = velocity.x;
		}
	}

	if (keysAndID["jump"] && *onFloor)
	{									   //will jump up and indicate that it's not on the floor anymore
		userVelocity->y = -acceleration.y; //negative y is upwards
		*onFloor = false;
	}
	
	//if velocity in x/y is more than 0 (so right of the screen or down respectively), then direction = 1, otherwise the opposite is direction = -1, and 0 means it's not moving
	if(userVelocity->x > 0){
		direction->x = 1;
	}else if(userVelocity->x < 0){
		direction->x = -1;
	}else{
		direction->x = 0;
	}
	
	if(userVelocity->y > 0){
		direction->y = 1;
	}else if(userVelocity->y < 0){
		direction->y = -1;
	}else{
		direction->y = 0;
	}	
}

void physics::moveEntities()
{
	std::lock_guard<std::mutex> lock(mutexs::userLocationsMutex); //will block attempts to lock this mutex again, thereby allowing us to prevent accidentally accessing shared data at the wrong time, released at the end of scope
	
	int componentIndex = 0;
	ecs::entity::entity tempEntity; //used to put the users entity inside some chunk
	for (int j = 0; j < ecs::component::physicsObjects.compVec.size(); j++)
	{ //loops through physical structs, by reference
		tempEntity.id = 0;
		auto physicsalStruct = &ecs::component::physicsObjects.compVec[j];
		componentIndex = j;

		if(physicsalStruct->objType == COLLISION){
			continue; //we don't really care about floors/stairs colliding
		}

		if (!physicsalStruct->onFloor){
			physicsalStruct->velocity.y += deceleration.y; //if it's not on ground it should accelerate downwards
		}

		sf::Vector2f newCoordinates(physicsalStruct->coordinates.x + physicsalStruct->velocity.x, physicsalStruct->coordinates.y + physicsalStruct->velocity.y);

		coordinatesStruct newChunkCoords(chunkCoordHelper(int(newCoordinates.x), chunkPixelSize_x), chunkCoordHelper(int(newCoordinates.y), chunkPixelSize_y));
		coordinatesStruct currentChunkCoords(chunkCoordHelper(int(physicsalStruct->coordinates.x), chunkPixelSize_x), chunkCoordHelper(int(physicsalStruct->coordinates.y), chunkPixelSize_y)); //get what their coordinates would be in the world using simple mod math

		//the below moves an entity out of its old chunk and into the one it is now in
		if (newChunkCoords.coordinates.first != currentChunkCoords.coordinates.first || newChunkCoords.coordinates.second != currentChunkCoords.coordinates.second){
			unsigned int entityID = physicsObjects.vectorToEntityMap(componentIndex);

			for (int i = 0; i < chunks[currentChunkCoords].second.size(); i++)
			{ //get the coordinate retrieved above, and get the chunk at that coordinate, and loop through the vector storing all the entities at that coordinate
				if (chunks[currentChunkCoords].second[i].id == entityID)
				{																			  //if the entityID is equal to the one we retrieved, it's the user we're looking for
					chunks[currentChunkCoords].second.erase(chunks[currentChunkCoords].second.begin() + i); //remove the user from the vector
					chunks[currentChunkCoords].first.userCount--; //decrements the number of users in this chunk
					//std::cout << "decrementing user count (moving) \n";
					tempEntity.id = entityID; //sets the correct entityID
					chunks[newChunkCoords].second.push_back(tempEntity); //and pushes to the vector in the new chunk
					chunks[newChunkCoords].first.userCount++; //increments the number of users in this chunk
					break;
				}
			}
			//std::cout << chunks[newChunkCoords].size() << " -- " << chunks[currentChunkCoords].size() << std::endl;

			std::cout << "moved " << users.compVec[users.entityToVectorMap(entityID)].username << " to chunk: " << newChunkCoords.coordinates.first << ", " << newChunkCoords.coordinates.second << std::endl;
				
			updateActiveChunkData::getInstance()->updateActiveChunks(); //updates the active chunks
		}


		if (physicsalStruct->coordinates.y + physicsalStruct->velocity.y > -5)
		{ //this means below ground level, so manually set to ground
			physicsalStruct->velocity.y = 0;
			physicsalStruct->coordinates.y = -5;
			physicsalStruct->onFloor = true;
		}

		physicsalStruct->coordinates.x += physicsalStruct->velocity.x;
		physicsalStruct->coordinates.y += physicsalStruct->velocity.y;
	}
}

mutexs *mutexs::instance = 0;
mutexs::mutexs() {}
mutexs *mutexs::getInstance()
{
	if (instance == 0)
	{
		instance = new mutexs();
	}
	return instance;
}

std::mutex mutexs::userLocationsMutex; //defines the mutex
std::mutex mutexs::removeUserMutex; //defines the mutex

gameBroadcast *gameBroadcast::instance = 0;
gameBroadcast *gameBroadcast::getInstance()
{
	if (instance == 0)
	{
		instance = new gameBroadcast();
	}
	return instance;
}

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
					sf::Vector2f currentChunk = {chunkCoordHelper(int(physicsObjects.compVec[physicsCompVecIndex].coordinates.x), chunkPixelSize_x), chunkCoordHelper(int(physicsObjects.compVec[physicsCompVecIndex].coordinates.y), chunkPixelSize_y)};
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

updateActiveChunkData *updateActiveChunkData::instance = 0;
updateActiveChunkData::updateActiveChunkData(){};
updateActiveChunkData *updateActiveChunkData::getInstance()
{
	if (instance == 0)
	{
		instance = new updateActiveChunkData();
	}
	return instance;
}

void updateActiveChunkData::updateActiveChunks()
{ //this is for updating which chunks are actually active
	std::vector<coordinatesStruct> generationCoords = {}; //a vector containing all of the coordinates to generate a new chunk at
	for (auto &chunk : chunks)
	{
		if(chunk.second.first.userCount > 0){
			for(int i = chunk.first.coordinates.first-1; i <= chunk.first.coordinates.first+1;i++){
				for(int j = chunk.first.coordinates.second-1; j <= chunk.first.coordinates.second+1;j++){ //loops around all the surrounding chunks
					if(!chunks.count(coordinatesStruct(i, j))){ //if the chunk hasn't been made yet
						generationCoords.push_back(coordinatesStruct(i, j)); //flag this up for generation
					}
				}
			}
		}
	}
	
	std::cout << generationCoords.size() << "\n";
	/*
	Generation:
	-Every power of 2 on the x-axis excluding the first 3 (so until the 8th chunk), generate a city there
		-> their y vaues are always multiples of 5
	*/

	std::vector<coordinatesStruct> airCoords; //used for the stairs or whatever requires air above

	bool generationFlag = false; //used to indicate if generation has happened or not (so to skip the semi random generation bit)
	for(auto &generation : generationCoords){
		unsigned int entityID = ecs::entity::superEntityManager.create({components::DRAWABLE, components::PHYSICAL}); //a new object with those attributes is made
		
		if(generation.coordinates.first == 0 && std::floor((float)generation.coordinates.second/5) == (float)generation.coordinates.second/5){
			chunks[generation].first.settingID = 1; //ID: 1 is city
			generationFlag = true;
		}else if(std::floor((float)log2(abs(generation.coordinates.first))) == (float)log2(abs(generation.coordinates.first)) && std::floor((float)generation.coordinates.second/5) == (float) generation.coordinates.second/5){
			int newChunkX = (int)log2(abs((float)generation.coordinates.first));
			if(newChunkX >= 3){ //don't want to generate for the first 8*chunkWidth pixels
				chunks[generation].first.settingID = 1; //ID: 1 is city
				chunks[coordinatesStruct(generation.coordinates.first, generation.coordinates.second+1)].first.settingID = 9; //below it a cave is generated (this is for the edge case when nothing is generated)
				generationFlag = true;
			}
		}
		
		if(!generationFlag){ //just remember up is negative, so chunk.y-1 is the y coordinate of chunks above it (which is used for checking if a city is above stairs below)
			int chance = rand() % 100 + 1;
			if(chance > 90 && chance < 95 && chunks[coordinatesStruct(generation.coordinates.first, generation.coordinates.second-1)].first.settingID != 1){ //so long as city isn't above
				chunks[generation].first.settingID = 4; //ID: 4 is stairs right
				airCoords.push_back(coordinatesStruct(generation.coordinates.first, generation.coordinates.second-1)); //-1 is above it
			}else if(chance > 95 && chunks[coordinatesStruct(generation.coordinates.first, generation.coordinates.second-1)].first.settingID != 1){ //so long as city isn't above
				chunks[generation].first.settingID = 5; //ID: 5 is stairs left
				airCoords.push_back(coordinatesStruct(generation.coordinates.first, generation.coordinates.second-1)); //-1 is above it
			}else{
				chunks[generation].first.settingID = 9; //ID: 9 is cave
			}
		}
		
		physicsObjects.compVec[physicsObjects.entityToVectorMap(entityID)].boxCorners = {
			sf::Vector2f(generation.coordinates.first * chunkPixelSize_x, (generation.coordinates.second * chunkPixelSize_y) + chunkPixelSize_y - 5), 
			sf::Vector2f((generation.coordinates.first * chunkPixelSize_x) + chunkPixelSize_x, (generation.coordinates.second * chunkPixelSize_y) + chunkPixelSize_y - 5),
			sf::Vector2f(generation.coordinates.first * chunkPixelSize_x, (generation.coordinates.second * chunkPixelSize_y) + chunkPixelSize_y), 
			sf::Vector2f((generation.coordinates.first * chunkPixelSize_x) + chunkPixelSize_x, (generation.coordinates.second * chunkPixelSize_y) + chunkPixelSize_y)
		}; //sets the corners of these boxes, remember negative is up
		physicsObjects.compVec[physicsObjects.entityToVectorMap(entityID)].objType = COLLISION; //sets the object type

		if(chunks[generation].first.settingID != 4 && chunks[generation].first.settingID != 5){ //just for testing
			chunks[generation].second.push_back(entityID); //pushes the floor entity to the chunks object
		}
	}

	for(auto &generateAir : airCoords){
		chunks[generateAir].first.settingID = 0; //ID: 0 is air
	}
}

void updateActiveChunkData::updateChunkData()
{					  //this is for updating the gameData object
	gameData.clear(); //empties the gameData object
	for (auto &chunkEntityVector : chunks){
		gameData[chunkEntityVector.first] = json::object();
		for (int i = 0; i < chunkEntityVector.second.second.size(); i++)
		{
			std::lock_guard<std::mutex> mutex(mutexs::removeUserMutex);	//locks the mutex so user can't logout while this is being updated or vice versa
			int entityID = chunkEntityVector.second.second[i].id;

			

			if(users.entityToVectorMap(entityID) != -1){ //it must be a user
				gameData[chunkEntityVector.first]["entities"][i]["type"] = "USER"; //its type is user, use these sort of capital letter words to describe the entity 'type'
			}else if(drawables.entityToVectorMap(entityID) != -1 && physicsObjects.entityToVectorMap(entityID) != -1){ //its probably floor or wall or something
				gameData[chunkEntityVector.first]["entities"][i]["type"] = "COLLISION";
			}else{
				gameData[chunkEntityVector.first]["entities"][i]["type"] = "OTHER"; //this should never be here, but just to be safe
			}
			//add in other conditions like these for mobs, items, or other things
			
			//the below are the generic information stuff

			if(users.entityToVectorMap(entityID) != -1){ //entityToVectorMap returns -1 if the entityID is not found in its bimap, so we just use that to find
				unsigned int usersIndex = users.entityToVectorMap(entityID);
				gameData[chunkEntityVector.first]["entities"][i]["username"] = users.compVec[usersIndex].username;
				gameData[chunkEntityVector.first]["entities"][i]["avatar"] = users.compVec[usersIndex].avatar;
				gameData[chunkEntityVector.first]["entities"][i]["id"] = users.compVec[usersIndex].userID;
			}
			
			if(drawables.entityToVectorMap(entityID) != -1){
				unsigned int drawablesIndex = drawables.entityToVectorMap(entityID);
				gameData[chunkEntityVector.first]["entities"][i]["direction"]["x"] = drawables.compVec[drawablesIndex].direction.x;
				gameData[chunkEntityVector.first]["entities"][i]["direction"]["y"] = drawables.compVec[drawablesIndex].direction.y;
			}

			if(physicsObjects.entityToVectorMap(entityID) != -1){
				unsigned int physicsIndex = physicsObjects.entityToVectorMap(entityID);
				gameData[chunkEntityVector.first]["entities"][i]["location"]["x"] = physicsObjects.compVec[physicsIndex].coordinates.x;
				gameData[chunkEntityVector.first]["entities"][i]["location"]["y"] = physicsObjects.compVec[physicsIndex].coordinates.y;
				//the hit box stuff
				gameData[chunkEntityVector.first]["entities"][i]["hitBox"]["top-left"]["x"] = physicsObjects.compVec[physicsIndex].boxCorners[0].x;
				gameData[chunkEntityVector.first]["entities"][i]["hitBox"]["top-left"]["y"] = physicsObjects.compVec[physicsIndex].boxCorners[0].y;
				gameData[chunkEntityVector.first]["entities"][i]["hitBox"]["bottom-right"]["x"] = physicsObjects.compVec[physicsIndex].boxCorners[3].x;
				gameData[chunkEntityVector.first]["entities"][i]["hitBox"]["bottom-right"]["y"] = physicsObjects.compVec[physicsIndex].boxCorners[3].y;
			}

		}
		gameData[chunkEntityVector.first]["data"]["x"] = chunkEntityVector.first.coordinates.first * chunkPixelSize_x;
		gameData[chunkEntityVector.first]["data"]["y"] = chunkEntityVector.first.coordinates.second * chunkPixelSize_y;
		gameData[chunkEntityVector.first]["data"]["width"] = chunkPixelSize_x;
		gameData[chunkEntityVector.first]["data"]["height"] = chunkPixelSize_y;
		gameData[chunkEntityVector.first]["data"]["userCount"] = chunkEntityVector.second.first.userCount;
		gameData[chunkEntityVector.first]["data"]["entityCount"] = chunkEntityVector.second.second.size();
		gameData[chunkEntityVector.first]["data"]["setting_id"] = chunkEntityVector.second.first.settingID;
	}
}

game *game::instance = 0;
game::game() {}
game *game::getInstance() 
{
	if (instance == 0)
	{
		instance = new game();
	}
	return instance;
}

void game::runGame()
{
	while (true)
	{
		physics::getInstance()->moveEntities();
		updateActiveChunkData::getInstance()->updateChunkData();
		gameBroadcast::getInstance()->broadcastGameState();

		sf::sleep(sf::milliseconds(1000 / fps)); //will only run the *fps* number of times per second at most, maybe slightly longer due to mutex locking
	}
}