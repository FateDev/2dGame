#ifndef HELPER_HEAD
#define HELPER_HEAD

#include <string>

bool extractInformation(std::string &msg);
bool checkLeave(std::string &msg);
bool processString(std::string msg);
size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp);

#endif