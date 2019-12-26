#ifndef HELPER_HEAD
#define HELPER_HEAD

#include <unordered_map>
#include <string>
#include <cmath>

int chunkCoordHelperX(float coord, float screenSize);
int chunkCoordHelperY(float coord, float screenSize);
bool extractInformation(std::string &msg);
bool checkLeave(std::string &msg);
bool processString(std::string msg);
size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp);

//simple bidirectional map, assuming that the set of all keys and the set of A and B are unique in their sets (considering the parameters below)
template<typename T, typename U> //T is A, U is B
struct bimap{
    std::unordered_map<T, U> mapA; //A to B mapping
    std::unordered_map<U, T> mapB; //B to A mapping
    void insert(T A, U B) { mapA[A] = B; mapB[B] = A; };
    U getBfromA(T A) { return mapA[A]; }; //will return the corresponding B value
    T getAfromB(U B) { return mapB[B]; }; //will return the corresponding A value
    U setInMapB(U B, T A) { mapB[B] = A; }; //uses B to set a value of A
    U setInMapA(T A, U B) { mapA[A] = B; }; //uses A to set a value of B
    unsigned int countInA(T A) { return mapA.count(A); }; //will count appearances of the key in the mapA
    unsigned int countInB(U B) { return mapB.count(B); }; //will count appearances of the key in the mapB map
    void eraseFromA(T A) { mapB.erase(mapA[A]); mapA.erase(A); }; //will use A to erase the values
    void eraseFromB(U B) { mapA.erase(mapB[B]); mapB.erase(B); }; //will use B to erase the values
};

#endif