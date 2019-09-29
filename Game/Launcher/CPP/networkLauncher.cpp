//you need to define this to make sure <windows.h> doesn't conflict with any min()/max() functions, 
//as this header file has min() and max() functions as macros, so would otherwise cause conflix
#define NOMINMAX

#include <Windows.h>

#include "../headerFuncs.h"
#include <list>
#include <string>

bool networking::login(std::string username, std::string password) {
	std::string readBuffer;
	GETresponse(readBuffer, std::string("https://erewhon.xyz/game/loginToken.php?username=" + username + "&password=" + password));

	if (readBuffer.find("false") != 0 && readBuffer.size() != 0) { //if the flag is at the beginning of the string
		loggedinToken = readBuffer;
		loggedIn = true;
		confirmedUsername = username;

		/*code below gets stuff from the update thing, use this to check against local files n stuff*/
		updateFiles();

		return true;
	}
	else {
		return false;
	}

}

void networking::getChecksumTokens(std::vector<std::pair<std::string, std::string>> &tokens, bool local, std::string location) {
	//the code will run either in local mode or non local mode
	//in non local mode, it will check the remote URL for checksum tokens for the files, then it will read them into a string, then delimit them into pairs, and add them to a vector
	//in local mode the same will happen, but from a local file which is specified in the location parameter, which is also used for the URL ones
	//the local bool decides the mode
	if (!local) {
		std::string readBuffer;
		GETresponse(readBuffer, location); //reads the checksum tokens from the URL into a string

		if (readBuffer != "") { //if the string isn't empty
			std::vector<std::string> fileHashTokens;
			delimitString(" ", readBuffer, fileHashTokens); //fills a vector with tokens which were extracted from the delimited string, they are in the form hash###filename

			std::vector<std::string> tempSeparateTokens;
			for (int i = 0; i < fileHashTokens.size(); i++) {
				delimitString("###", fileHashTokens[i], tempSeparateTokens);
				tokens.push_back(std::make_pair(tempSeparateTokens[1], tempSeparateTokens[0])); //fills the vector in the parameter with entries of pairs, of {filename, hash} instead of a single string
				tempSeparateTokens.clear();
			}
		}
	}
	else {
		//the below basically does the exact same, but for a local file rather than a remote one
		std::stringstream buffer;
		std::string fileContents;

		std::ifstream checksumLocal(location, std::ios::in);
		buffer << checksumLocal.rdbuf();
		fileContents = buffer.str();

		if (fileContents != "") {
			std::vector<std::string> fileHashTokens;
			delimitString(" ", fileContents.substr(0, fileContents.length()-1), fileHashTokens);

			std::vector<std::string> tempSeparateTokens;
			for (int i = 0; i < fileHashTokens.size(); i++) {
				delimitString("###", fileHashTokens[i], tempSeparateTokens);
				tokens.push_back(std::make_pair(tempSeparateTokens[1], tempSeparateTokens[0]));
				tempSeparateTokens.clear();
			}
		}

	}
}

void networking::compareTokenList(std::vector<std::pair<std::string, std::string>> &localTokens, std::vector<std::pair<std::string, std::string>> &serverTokens, std::vector<std::pair<std::string, std::string>> &returnTokens, std::vector<std::pair<std::string, std::string>> &deleteTokens) {
	bool found = false;

	//the below loops through all of the tokens in the remote file, and compares them to the tokens in the local file, if the tokens show that the name is the same 
	//but the file hash is different, then the file is put down to be redownloaded, by being put down in the returnTokens vector (so functions as updating files)
	//if the token from the server is not found at all, it is put down to be downloaded, as it doesn't exist in the local file yet
	for (int i = 0; i < serverTokens.size(); i++) {
		for (int j = 0; j < localTokens.size(); j++) {
			if (serverTokens[i].first == localTokens[j].first) {
				if (serverTokens[i].second != localTokens[j].second) {
					returnTokens.push_back(serverTokens[j]);
				}
				found = true;
				break;
			}
		}
		if (found == false) {
			returnTokens.push_back(serverTokens[i]);
		}
		else {
			found = false;
		}
	}

	//this checks the opposite of above basically, if the local file names are identical to the remote ones, then they are skipped over, but if they aren't found at all
	//they are put down to be deleted, by being added to the deleteTokens vector
	for (int i = 0; i < localTokens.size(); i++) {
		for (int j = 0; j < serverTokens.size(); j++) {
			if (localTokens[i].first == serverTokens[j].first) {
				found = true;
				break;
			}
		}
		if (found == false) {
			deleteTokens.push_back(localTokens[i]);
		}
		else {
			found = false;
		}
	}
}

void networking::parseReturnTokens(std::vector<std::pair<std::string, std::string>> &returnTokens) { 
	//if the returnTokens vector is greater than zero, then all the files in it are downloaded from the server
	if (returnTokens.size() > 0) {
		GETftpFile("Game\\checksumFile.a", "https://erewhon.xyz/game/latestFiles/checksumFile.a");

		std::string dirPath = "Game\\";
		std::vector<std::string> tempFilePathTokens;
		for (int i = 0; i < returnTokens.size(); i++) {
			if (returnTokens[i].first.find("/") == std::string::npos) { //if the file isn't in a subdirectory, just download it
				GETftpFile(std::string("Game\\" + returnTokens[i].first), std::string("http://erewhon.xyz/game/latestFiles/" + returnTokens[i].first));
			}
			else { //if the file is in a subdirectory
				delimitString("/", returnTokens[i].first, tempFilePathTokens); //add the subdirectories to a vector

				std::string fileName = tempFilePathTokens[tempFilePathTokens.size() - 1]; //get the filename from the above vector, as it'll be the final parameter

				std::string fileLocation = "Game\\"; //all files for the game are put in this subdirectory

				for (int i = 0; i < tempFilePathTokens.size() - 1; i++) {
					fileLocation += tempFilePathTokens[i] + "\\"; //appends subdirectories to this string which will be the directory of the files
				}

				CreateDirectory(fileLocation.c_str(), NULL); //makes the root directory for the file

				tempFilePathTokens.clear(); //clears the temp vector for the file path

				GETftpFile(std::string("Game\\" + fileName), std::string("http://erewhon.xyz/game/latestFiles/" + returnTokens[i].first)); //then downloads the file to the Game directory
				MoveFile(std::string("Game\\" + fileName).c_str(), std::string(fileLocation + fileName).c_str()); //then moves the file to the subdirectoy
				//for some reason it doesn't work by downloading directly to the subdirectory
			}
		}
	}
}

bool networking::updateFiles() { //updates files
	std::vector<std::pair<std::string, std::string>> serverTokens;
	getChecksumTokens(serverTokens, false, "https://erewhon.xyz/game/update.php"); //gets the tokens on the server

	if (doesFileExist("Game\\checksumFile.a")) { //if the local checksum file does exist
		//below makes the return and delete token vectors, then populates them, then the return tokens are parsed
		//I haven't made a deleteTokens vector parser yet, but it basically just needs to delete files which are in that vector
		std::vector<std::pair<std::string, std::string>> returnTokens;
		std::vector<std::pair<std::string, std::string>> deleteTokens;

		std::vector<std::pair<std::string, std::string>> localTokens;
		getChecksumTokens(localTokens, true, "Game\\checksumFile.a");
		
		compareTokenList(localTokens, serverTokens, returnTokens, deleteTokens);

		parseReturnTokens(returnTokens);
	}
	else {
		GETftpFile("Game\\checksumFile.a", "https://erewhon.xyz/game/latestFiles/checksumFile.a"); //if the checksum file doesn't exist, download it

		parseReturnTokens(serverTokens); //and redownload all files just in case, as we have no way of knowing if the file are already there
	}
}