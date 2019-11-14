#ifndef HELPER_HEAD
#define HELPER_HEAD

#include <unordered_map>
#include <string>

bool extractInformation(std::string &msg);
bool checkLeave(std::string &msg);
bool processString(std::string msg);
size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp);

//simple bidirectional map, assuming that the set of all keys and the set of A and B are unique in their sets (considering the parameters below)
template<typename T, typename U>
struct bimap{
    std::unordered_map<T, U> AtoB;
    std::unordered_map<U, T> BtoA;
    void insert(T A, U B) { AtoB.insert({A, B}); BtoA.insert({B, A}); };
    U getBfromA(T A) { return AtoB[A]; }; //will return the corresponding B value
    T getAfromB(U B) { return BtoA[B]; }; //will return the corresponding A value
    U setAfromB(U B, T A) { BtoA[B] = A; }; //uses B to set a value of A
    U setBfromA(T A, U B) { AtoB[A] = B; }; //uses A to set a value of B
    unsigned int countInA(T A) { return AtoB.count(A); }; //will count appearances of the key in the AtoB map
    unsigned int countInB(U B) { return BtoA.count(B); }; //will count appearances of the key in the BtoA map
    void eraseFromA(T A) { BtoA.erase(AtoB[A]); AtoB.erase(A); }; //will use A to erase the values
    void eraseFromB(U B) { AtoB.erase(BtoA[B]); BtoA.erase(B); }; //will use B to erase the values
};

#endif