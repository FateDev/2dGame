//you need to define this to make sure <windows.h> doesn't conflict with any min()/max() functions, 
//as this header file has min() and max() functions as macros, so would otherwise cause conflix
#define NOMINMAX

#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <curl/curl.h>
#include "../headerFuncs.h"

void helpFunction() { //the function to make the help button open the help setting in browser
	ShellExecute(0, 0, "https://erewhon.xyz/game/help/", 0, 0, SW_SHOW);
	//opens URL in browser
	//1st param is handle to parent windows, but we're using SFML rather than win32 windows so this is NULL or 0
	//2nd param is the action, which isnt necessary here
	//3rd is the thing to 'do' (the URL), so the default browser would be used on windows
	//4th would be parameters to pass to the file, but not necessary for opening a URL
	//5th is the working directory of the action, none specified so current one used
	//6th is how to show the application once opened, so it shows it
}

size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp) //first one is pointer to data, second is to the size of one data item,
{ //third is how many there are, so size * nmemb is data size, and fourth is pointer to the data you get back, which you actually access using CURLOPT_WRITEDATA and a reference
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

void GETresponse(std::string &readBuffer, std::string urlAndParams) { //get helper function, simple string return
	CURL* curl = curl_easy_init(); //we can set options for this to make it control how a transfer/transfers will be made

	curl_easy_setopt(curl, CURLOPT_URL, urlAndParams.c_str());
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); //the callback
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); //will write data to the string, so the fourth param of the last callback is stored here

	curl_easy_perform(curl);

	curl_easy_cleanup(curl);
}

struct FtpFile { //helper struct for FTP transfer
	const char *filename;
	FILE *stream;
};

static size_t FileWriteCallback(void *buffer, size_t size, size_t nmemb, void *stream) //callback to save FTP files to local disk, refer to comments about the WriteCallback above
{
	struct FtpFile *out = (struct FtpFile *)stream;
	if (!out->stream) {
		/* open file for writing */
		out->stream = fopen(out->filename, "wb");
		if (!out->stream)
			return -1; /* failure, can't open file to write */
	}
	return fwrite(buffer, size, nmemb, out->stream); //write the file
}

void GETftpFile(std::string fileLocation, std::string remoteFileURL) { //helper function for FTP file transfers
	CURL *curl;
	CURLcode res;

	struct FtpFile ftpfile = { //makes a new instance of the FTP file helper struct
	  fileLocation.c_str(),
	  NULL
	};

	//normal CURL stuff below basically
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, remoteFileURL.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, FileWriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);

		res = curl_easy_perform(curl);

		/* always cleanup */
		curl_easy_cleanup(curl);

		if (CURLE_OK != res) {
			/* we failed */
			fprintf(stderr, "curl told us %d\n", res);
		}
	}

	if (ftpfile.stream)
		fclose(ftpfile.stream); /* close the local file */

	curl_global_cleanup();
}

bool doesFileExist(std::string fileName) { //checks if the file exists, a helper function
	std::ifstream infile(fileName.c_str());
	return infile.good();
}

void delimitString(std::string delimiter, std::string stringToSplit, std::vector<std::string> &outputTokens) { //simple delimiter function, puts it into a vector
	int pos = 0;
	while ((pos = stringToSplit.find(delimiter)) != std::string::npos) {
		outputTokens.push_back(stringToSplit.substr(0, pos));
		stringToSplit.erase(0, pos + delimiter.length());
	}

	outputTokens.push_back(stringToSplit.substr(0, stringToSplit.length()));
}