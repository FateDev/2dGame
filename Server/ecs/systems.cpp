#include "../header/ecs.h"
#include "../header/helper.h"

#include <cmath>
#include <functional>
#include <iostream>
#include <SFML/Network.hpp>
#include <curl/curl.h>

using namespace ecs::system;
using namespace ecs::component;

void systemsManager::systemStart(){
	processNetwork = new sf::Thread(&network::process, &networkObj); //makes the thread
	processNetwork->launch(); //launches it

	listenNetwork = new sf::Thread(std::bind(&network::server, &networkObj, port)); //launches the server to listen on that specific port
	listenNetwork->launch(); //launches it
	
	listenUdp = new sf::Thread(&udpBroadcast::listenToUsers, &udpNetworkObj);
	listenUdp->launch();

	sendUdp = new sf::Thread(&udpBroadcast::broadcastGameState, &udpNetworkObj);
	sendUdp->launch();
}

void systemsManager::systemEnd(){
    if(processNetwork){ //if a thread exists
		processNetwork->wait(); //wait until it's completed this bit of code
		delete processNetwork; //delete it, by the way as sever(...) has an infinite while loop inside, this is only really called when quitting the program
	}

    if(listenNetwork){ //if a thread exists
		listenNetwork->wait(); //wait until it's completed this bit of code
		delete listenNetwork; //delete it, by the way as sever(...) has an infinite while loop inside, this is only really called when quitting the program
	}
}

float distanceMagnitude(unsigned int index_1, unsigned int index_2){ //this simple function takes in the component vector indexes of two users, retrieves their in game positions and returns the distance between them
    unsigned int entityID_1 = users.vectorToEntityMap(index_1);
    unsigned int entityID_2 = users.vectorToEntityMap(index_2);

    sf::Vector2f location1 = locationStructs.compVec[entityID_1].coordinates;
    sf::Vector2f location2 = locationStructs.compVec[entityID_2].coordinates;

    float yDist = abs(location2.y - location1.y);
    float xDist = abs(location2.x - location1.x);

    return sqrt(pow(xDist, 2) + pow(yDist, 2)); //this is the magnitude of the x and y distances combined (the hypotenuse)
}

void network::removeUser(unsigned int i){ //function to basically properly log out a user
    //have to get the values below before removing the user from the array, so we can send a request to the HTTP server and have their online/offline status updated
    std::string id = std::to_string(users.compVec[i].userID);

    selector.remove(*users.compVec[i].socket); //removes this socket from the selector

	uniqueIDToUserVectorIndexMap.erase(users.compVec[i].sessionID); //will erase the mapping of the unique session ID to the index in the component vector for this user
    
    unsigned int entityID = users.vectorToEntityMap(i); //get the user entityID
	sf::Vector2f coordinates = locationStructs.compVec[locationStructs.entityToVectorMap(entityID)].coordinates; //get the user's coordinates
	ecs::system::coordinatesStruct removeUserCoordinates((int(coordinates.x) % chunkPixelSize_x), (int(coordinates.y) % chunkPixelSize_y)); //get what their coordinates would be in the world using simple mod math

	for(int i = 0; i < chunks[removeUserCoordinates].size(); i++){ //get the coordinate retrieved above, and get the chunk at that coordinate, and loop through the vector storing all the entities at that coordinate
		if(chunks[removeUserCoordinates][i].id == entityID){ //if the entityID is equal to the one we retrieved, it's the user we're looking for
			chunks[removeUserCoordinates].erase(chunks[removeUserCoordinates].begin()+i); //remove the user from the vector
			break;
		}
	}

    ecs::entity::superEntityManager.destroy(entityID); //removes them from the array if it is

    std::string msgString = "SERVER: A USER HAS LEFT\nSERVER: CLIENTS ONLINE: " + std::to_string(users.compVec.size()) + "\n"; //outputs some server info to indicate that the user is gone
    std::cout << msgString; //outputs this string
}

void network::forwardToAllUsers(std::string msg, int userNum){ //forwards to every user
	for(int i = 0; i < users.compVec.size(); i++){
		if(users.compVec[i].loggedIn != true){
			continue;
		}
    
		if(users.compVec[i].roomGuild == users.compVec[userNum].roomGuild){
			sf::Packet packet; //a packet to hold a string
			packet << msg.c_str(); //putting the c style string into the packet

			users.compVec[i].socket->send(packet); //sending the packet
		}
	}
}

void network::broadcastToLocal(std::string msg, int userNum){ //this broadcasts to users within 100 units of the player (hence the 100 in the if statement below)
	for(int i = 0; i < users.compVec.size(); i++){
		if(users.compVec[i].loggedIn != true){
			continue;
		}

		if(users.compVec[i].roomGuild == users.compVec[userNum].roomGuild){
			if(distanceMagnitude(userNum, i) < 100){ //if the distance between player is less than 100, transmit messages between them using area chat
				sf::Packet packet; //a packet to hold a string
				packet << msg.c_str(); //putting the c style string into the packet

				users.compVec[i].socket->send(packet); //sending the packet
			}
		}
	}
}

void network::saveMsgDB(std::string msg, int userNum, int time){
	CURL* curl = curl_easy_init(); //we can set options for this to make it control how a transfer/transfers will be made
	std::string readBuffer; //string for the returning data
	
	int id = users.compVec[userNum].userID;
	msg = curl_easy_escape(curl, msg.c_str(), msg.length());
	std::string roomGuild = users.compVec[userNum].roomGuild;
	
	curl_easy_setopt(curl, CURLOPT_URL, ("https://erewhon.xyz/game/serverMessages.php?save=true&id="+std::to_string(id)+"&msg="+msg+"&roomGuild="+roomGuild+"&time="+std::to_string(time)).c_str());
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); //the callback
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); //will write data to the string, so the fourth param of the last callback is stored here

	curl_easy_perform(curl);

	users.compVec[userNum].tempNewestMsgID = readBuffer;
}

void network::process(){
	std::time_t t; //current time
	std::tm* now; //object which processes into year, month, day, etc

	while(true){ //loop endlessly
		if(selector.wait(sf::seconds(1))){ //waits 1 second for any activity from any of the sockets
			for(int i = 0;i < users.compVec.size(); i++){ //loops through every connected socket
				if(selector.isReady(*(users.compVec[i].socket)) && users.compVec[i].loggedIn == true){ //checks through every socket in the selector, checking if it is ready for action (all sockets are in the selector so this is valid)
					sf::Packet receivePacket; //packet for message to be received
					std::string receiveString = ""; //the string for the packet
					std::string currentTime; //currentTime string
					std::string sendString; //string to send to users

					t = std::time(0); //gets current time
					now = std::localtime(&t); //makes it into an object which we can extract time from

					users.compVec[i].socket->receive(receivePacket); //receives packet
						

					if((receivePacket >> receiveString) && receiveString != ""){ //if the packet data can be extracted, and it is not empty
						if(processString(receiveString)){ //if this contains the ping flag which is to be received every few seconds, update the socket expiry time
							users.compVec[i].timeOfExpiry = sf::seconds(expiryTimer.getElapsedTime().asSeconds() + 5); //updates the socket expiry time
							continue; //then continue
						}

						if(checkLeave(receiveString)){
							sf::Packet packet; //a packet to hold a string
							packet << std::string("die").c_str(); //putting the c style string into the packet
							users.compVec[i].socket->send(packet); //sending the packet
                            removeUser(i);
						}

						if(receiveString.find("USER::CHANGEROOMGUILD::") == 0){
							receiveString.erase(receiveString.begin(), receiveString.begin() + (receiveString.find("USER::CHANGEROOMGUILD::") + std::string("USER::CHANGEROOMGUILD::").length()));

							users.compVec[i].roomGuild = receiveString;

							continue;
						}

						if(!extractInformation(receiveString)){ //if username and message could not be extracted...
							continue; //then continue
						}
						
						//below is basically saying to set the local message id to 0 if LOCALCHAT, you'd only use it to make sure your messages aren't recorded whatsoever
						if(users.compVec[i].roomGuild != "LOCALCHAT"){
							saveMsgDB(receiveString, i, t);
						}else{
							users.compVec[i].tempNewestMsgID = "-1"; //ID is not
						}

						std::string time = std::to_string(t);

						sendString += "USER::ID::" + users.compVec[i].tempNewestMsgID; //USER::ID is actually the message ID
						sendString += "USER::USERNAME::" + users.compVec[i].username;
						sendString += "USER::TIME::" + time;
						sendString += "USER::MSG::" + receiveString;

						std::cout << sendString << std::endl;
						
						if(users.compVec[i].roomGuild != "LOCALCHAT"){
							forwardToAllUsers(sendString, i); //also sends to all users but the one which just sent this
						}else{
							broadcastToLocal(sendString, i);
						}
						
						currentTime = ""; //empties this string
					}
				}
			}
		}

		for(int i = 0;i < users.compVec.size();i++){ //loops through every user...
			if(users.compVec[i].timeOfExpiry.asSeconds() <= expiryTimer.getElapsedTime().asSeconds()){ //checks if their socket's time of expiry is now or passed...
                removeUser(i);
			}
		}
		
		sf::sleep(sf::milliseconds(50)); //slows down the process loop so less intensive on my poor laptop
	}
}

bool network::login(std::string input, user* userPtr) {
	std::string usernameToken = "SERVER::LOGON::USERNAME::";
	std::string username = input;


	if (input.find(usernameToken) == 0 != std::string::npos) {
		username.erase(username.begin(), (username.begin() + usernameToken.size()));
		
		CURL* curl = curl_easy_init(); //we can set options for this to make it control how a transfer/transfers will be made
		std::string readBuffer; //string for the returning data

		for(int i = 0; i < users.compVec.size(); i++){ //makes sure the same user can't join twice
			if(users.compVec[i].username == username){
				return false;
			}
		}

		username = curl_easy_escape(curl, username.c_str(), username.length());
		
		curl_easy_setopt(curl, CURLOPT_URL, ("http://erewhon.xyz/game/serverLogin.php?username="+username).c_str());
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

void network::server(unsigned short PORT){ //the function for server initialisation
	sf::TcpListener listener; //makes a tcpListener

	listener.listen(PORT); //listens on the provided port

	sf::TcpSocket* socket; //makes a socket pointer

	user* userPtr; //makes a user pointer

	std::cout << "SERVER: SERVER STARTED" << std::endl; //outputs that the server has started
	//unsigned int entityID2 = ecs::entity::superEntityManager.create({components::USER, components::LOCATION});
	//std::cout << entityID2 << std::endl;

	sf::Clock clock;

	ecs::system::coordinatesStruct startCoord(0, 0); //the starting coordinates
	ecs::entity::entity tempEntity; //used to put the users entity inside some chunk

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
				sendPacket << "SERVER::LOGON::RESPONSE::falseUSER::ID::-1";
				socket->send(sendPacket);

				delete userPtr;
				delete socket;
				continue;
			}else{
				userPtr->sessionID =  userPtr->username + std::to_string(clock.getElapsedTime().asMicroseconds());

				sendPacket << std::string("SERVER::LOGON::RESPONSE::trueUSER::SESSION_ID::" + userPtr->sessionID +"USER::ID::" + std::to_string(userPtr->userID)).c_str();
				socket->send(sendPacket);

				outputString = "SERVER: NEW CONNECTION @ " + socket->getRemoteAddress().toString() + "\n"; //make a string which includes their IP address...

                unsigned int entityID = ecs::entity::superEntityManager.create({components::USER, components::LOCATION}); //a new object with those attributes is made
				
				//std::cout << entityID << std::endl;
				tempEntity.id = entityID; //sets the temp entity to have the correct ID
				ecs::system::chunks[startCoord].push_back(tempEntity); //pushes users to the (0, 0) chunk

                unsigned int componentVectorIndex = users.entityToVectorMap(entityID); //gets the component vector index of this entity

				userPtr->socket = socket; //make the new user object contain their socket
				userPtr->timeOfExpiry = sf::seconds(expiryTimer.getElapsedTime().asSeconds() + 5); //set the expiry time for their socket
				userPtr->loggedIn = true;

				users.compVec[componentVectorIndex] = *userPtr; //add this user to the users vector
				uniqueIDToUserVectorIndexMap.insert({userPtr->sessionID, componentVectorIndex}); //will add a map entry, mapping their unique session ID to some index in the component vector

				socket->getRemoteAddress();

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

physics* physics::instance = 0; //to make it singleton
physics::physics() {} //basically default constructor

physics* physics::getInstance(){
	if(instance == 0){
		instance = new physics();
	}
	return instance;
}


void physics::userInput(json keysAndID){
	unsigned short userVectorIndex = uniqueIDToUserVectorIndexMap[keysAndID["sessionID"]];
	sf::Vector2f* userVelocity = &ecs::component::locationStructs.compVec[userVectorIndex].velocity;
	bool* onFloor = &ecs::component::locationStructs.compVec[userVectorIndex].onFloor;

	if((keysAndID["left"] && keysAndID["right"]) || (!keysAndID["left"] && !keysAndID["right"])){ //if both or neither of them are pressed, the velocity is zero
		userVelocity->x = 0;
	}else{
		if(keysAndID["left"])
			userVelocity->x = -velocity.x;
		if(keysAndID["right"])
			userVelocity->x = velocity.x;
	}

	if(keysAndID["jump"] && *onFloor){ //will jump up and indicate that it's not on the floor anymore
		userVelocity->y = -acceleration.y; //negative y is upwards
		*onFloor = false;
	}
	//std::cout << ecs::component::locationStructs.compVec[userVectorIndex].velocity.x << " -- " << ecs::component::locationStructs.compVec[userVectorIndex].velocity.y << " -- " << ecs::component::locationStructs.compVec[userVectorIndex].onFloor << std::endl;

}

void physics::userIndependentPhysics(){
	//make gravity effects here
}

physics::collisionType physics::checkCollision(sf::Vector2f coordinates, sf::Vector2f velocity){
	//checks collision detection, uses mod math to find current chunk, then does collision detection
	sf::Vector2f currentChunkCoords(int(coordinates.x) % chunkPixelSize_x, int(coordinates.y) % chunkPixelSize_y);

	for(auto &locationStruct : ecs::component::locationStructs.compVec){ //loops through location structs, by reference

	}

	return NONE; //temporarily return this
}

void physics::moveEntities(){
	for(auto &locationStruct : ecs::component::locationStructs.compVec){ //loops through location structs, by reference
		std::lock_guard<std::mutex> lock(mutexs::userLocationsMutex); //will block attempts to lock this mutex again, thereby allowing us to prevent accidentally accessing shared data at the wrong time, released at the end of scope
		
		if(!locationStruct.onFloor){
			locationStruct.velocity.y += deceleration.y; //if it's not on ground it should accelerate downwards

			collisionType collision = checkCollision(locationStruct.coordinates, locationStruct.velocity);
			if(collision != NONE){
				switch (collision)
				{
				case FLOOR:
					locationStruct.onFloor = true;
					/*bounce off floor, and change velocity obviously*/
					break;
				case ENTITY:
					/*bounce off entity, and change velocity obviously*/
					break;
				case WALL:
					/*bounce off wall, and change velocity obviously*/
					break;
				case CEILING:
					/*bounce off ceiling, and change velocity obviously*/
					break;
				default:
					break;
				}
			}
		}

		locationStruct.coordinates.x += locationStruct.velocity.x;
		locationStruct.coordinates.y += locationStruct.velocity.y;

		sf::sleep(sf::milliseconds(1000/fps)); //will only run the *fps* number of times per second at most, maybe slightly longer due to mutex locking
	}
}

mutexs* mutexs::instance = 0;
mutexs::mutexs() {}
mutexs* mutexs::getInstance(){
	if(instance == 0){
		instance = new mutexs();
	}
	return instance;
}

std::mutex mutexs::userLocationsMutex; //defines the mutex

void udpBroadcast::broadcastGameState(){ //this broadcasts stuff on port 5002
	sf::UdpSocket socket;
	socket.bind(5002);
	while(true){
		json jsonObj;
		jsonObj["happy"] = "sure";
		sf::Packet packet;
		//packet << jsonObj.dump();
		//socket.send(packet, sf::IpAddress::Broadcast, 5002);

		sf::sleep(sf::milliseconds(200));
	}
}

void udpBroadcast::listenToUsers(){ //this listens to stuff on port 5001
	sf::UdpSocket socket;
	socket.bind(5001);
	while(true){
		char buffer[1024];
		std::size_t received = 0;
		sf::IpAddress senderIP;
		unsigned short senderPort;

		socket.receive(buffer, sizeof(buffer), received, senderIP, senderPort); //receive the data

		json jsonObj = json::parse(buffer); //make into json object
		
		if(!jsonObj["sessionID"].is_null()){ //does the sessionID field exist in this json object?
			if(uniqueIDToUserVectorIndexMap.count(jsonObj["sessionID"])){ //does the sessionID belong to any active nusers?
				//std::cout << jsonObj.dump() << std::endl; //print the json data for now
				physics::getInstance()->userInput(jsonObj);
			}
		}
	}
}

void updateActiveChunkData::updateActiveChunks(){
	for(ecs::component::location const& location : ecs::component::locationStructs.compVec){

	}
}