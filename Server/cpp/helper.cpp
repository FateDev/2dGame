#include "../header/helper.h"
#include <iostream>

size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp) //first one is pointer to data, second is to the size of one data item,
{ //third is how many there are, so size * nmemb is data size, and fourth is pointer to the data you get back, which you actually access using CURLOPT_WRITEDATA and a reference
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
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

int chunkCoordHelperX(float coord, float screenSize){
	return floor(coord/screenSize);
}

int chunkCoordHelperY(float coord, float screenSize){
	return ceil(coord/screenSize);
}

double findDistance(sf::Vector2f point1, sf::Vector2f point2){
	double distanceX = point1.x - point2.x;
	double distanceY = point1.y - point2.y;
	return sqrt(pow(distanceX, 2) + pow(distanceY, 2));
}