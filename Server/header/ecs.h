#ifndef ECS_HEAD
#define ECS_HEAD

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <initializer_list>
#include <SFML/Network.hpp>
#include <thread>
#include <mutex>

#include "../header/helper.h"

//the size of one chunk
const int chunkPixelSize_x = 1920;
const int chunkPixelSize_y = 1080;
const int fps = 60;
const sf::Vector2f deceleration = { 0, 0.1 }; //x = friction on surface deceleration, y = gravity
const sf::Vector2f acceleration = { 0, 1 }; //x = acceleration by arrow keys (don't want to accelerate), y = acceleration when jumping up
const sf::Vector2f velocity = { 1, 0 }; //x = initial velocity with arrow keys, y = initial velocity with arrow keys (don't want any so zero)

#include "../deps/json.hpp"
using json = nlohmann::json;

namespace ecs{
    namespace entity{
        enum entityType {USER, MOB}; //enum for the types of entities
    }
}

namespace ecs{
    namespace component{
        //the structs below are the components so far
        struct user{ 
            int userID = -1; //the user ID from the database
            bool loggedIn = false; //whether or not they're logged in
            bool gameConnected = false; //whether or not the gameSocket has been connected
            std::string sessionID; //unique ID for current session
            std::string username; //the username
            std::string roomGuild = "main.alpha"; //the current roomGuild for the social tab
            std::string tempNewestMsgID = ""; //the ID of the newest message
            sf::TcpSocket* socket = NULL; //the actual socket connection
            sf::TcpSocket* gameSocket = NULL; //the socket connection for sending game data
            sf::Time timeOfExpiry; //the time till the socket should be removed, this is updated every time the client pings the server
        };

        struct drawable{
            std::string imgLocation;
            ecs::entity::entityType type;
        };

        struct physical{
            sf::Vector2f hitBoxOffset = { 0, 0}; //the offset from the location of the entity, of the hit box
            float hitBoxRadius = 1; //the radius of the hit box
        };

        struct location{
            sf::Vector2f coordinates = { 0, 0 }; //the in game location of the player
            sf::Vector2f velocity = { 0, 0 }; //this is the velocity of the player
            bool onFloor = true;
        };

        enum components {USER, LOCATION, DRAWABLE, PHYSICAL}; //enum for all of the components

        //it's using a template because it could be used for any of the components above
        template <class T>
        class ecsComponentStructure{
            private:
                bimap<unsigned int, unsigned int> entityVectorMap; //will map entities to vector indexes so we can move about vector elements without breaking indexes (maps both ways)
            public:
                void addComponent(T component, unsigned int entityID); //will add this component
                void removeComponent(unsigned int entityID); //will remove the component at this index
                unsigned int entityToVectorMap(unsigned int entityID);
                unsigned int vectorToEntityMap(unsigned int componentIndex);
                std::vector<T> compVec; //will contain components of some type (component vector)
        };
        
        //each new component added to the game should be made exactly as they have been below, and obviously should have the components enum updated, and the explicit instantiation
        //stuff at the bottom of components.h should be updated, and the code for removing entities should be updated, and the code for adding new ones should also be updated
        extern ecsComponentStructure<user> users; //this basically tells the compiler that the variable declared is defined somewhere else in the program (main.cpp in this case)
        extern ecsComponentStructure<drawable> drawables; //this basically tells the compiler that the variable declared is defined somewhere else in the program (main.cpp in this case)
        extern ecsComponentStructure<location> locationStructs; //this basically tells the compiler that the variable declared is defined somewhere else in the program (main.cpp in this case)
    }
    
    namespace entity{
        struct entity{
            unsigned int id; //basic struct to say that this is actually an entity

            bool operator==(const entity& t) const { //this is needed to put entities in the unordered_set structure, this basically allows for direct comparison of different entity structs
                return (this->id == t.id); 
            }
        };

        struct Hash { //this is also needed to put the entity struct into an unordered_set, this makes the hashing function literally just be the entity ID, which isn't a problem as each entity has a unique ID
            size_t operator() (const ecs::entity::entity &entity) const {
                return entity.id;
            }
        };

        class entityManager{
            private:
                std::unordered_set<entity, Hash> entities; //will contain all of the entities
                entity nextEntity; //will allow for creating new entities, and easy iteration through the above set, to find the next free id
            public:
                unsigned int create(std::initializer_list<ecs::component::components> initialiseWithStructs); //makes entity struct with provided structs, will return the entity
                bool alive(entity entitStruct); //will simply check if the provided entity exists in the entities set
                void destroy(unsigned int entityID); //will destroy an entity, removing it from the above set, and removing all of its entries from the components struct
        };
        
        extern entityManager superEntityManager; //this basically tells the compiler that the variable declared is defined somewhere else in the program (main.cpp in this case)
    }

    namespace system{
        extern std::unordered_map<std::string, unsigned int>  uniqueIDToUserVectorIndexMap; //will map user's uniqueID's to their user vector index
        
        struct mapCleanup{
            void chunksMapCleanup(); //should be run on a thread, should iterate through every single chunk, with something like 100ms sleep inbetween as it's not that important
            //it should basically just remove entities that don't exist anymore
        };

        struct coordinatesStruct{
            int uniqueID = 0;    
            std::pair<int, int> coordinates;

            coordinatesStruct(int xCoord, int yCoord) { 
                std::string hashString = std::to_string(xCoord) + std::to_string(yCoord);
                hashString += std::to_string(hashString.size());
                uniqueID = std::stoi(hashString);

                coordinates.first = xCoord; //sets the coordinates
                coordinates.second = yCoord; //sets the coordinates
            };

            bool operator==(const coordinatesStruct& t) const { //this is needed to put entities in the unordered_map structure, this basically allows for direct comparison of different entity structs
                return (this->uniqueID == t.uniqueID); 
            }
        };

        struct Hash { //this is also needed to put the coordinates struct into an unordered_set, this makes the hashing function literally just be the coordinate ID, which isn't a problem as each entity has a unique ID
            size_t operator() (const coordinatesStruct &coords) const {
                return coords.uniqueID;
            }
        };

        class mutexs{
            private:
                static mutexs* instance;
                mutexs();
            public:
                static std::mutex userLocationsMutex; //declares the mutex for reading to/from user location comp vec
                static mutexs* getInstance();
        };
       
       class physics{
           private:
                static physics* instance;
                physics();
           public:
                enum collisionType { FLOOR, ENTITY, WALL, CEILING, NONE };
                static physics* getInstance();
                void userInput(json keysAndID);
                void moveEntities();
                collisionType checkCollision(sf::Vector2f coordinates, sf::Vector2f velocity);
                void userIndependentPhysics();
       };

        class gameBroadcast{
            private:
                static gameBroadcast* instance;
                gameBroadcast() {};
            public:
                sf::SocketSelector selector;
                static gameBroadcast* getInstance();
                void broadcastGameState(); //this will read the gameData object and send the relavent chunk data to some connected client
                void listenToUsers(); //recieves data from users
                void server();

        };

        class network{
            private:
                //sockets are normally in blocking mode, i.e, waits until a process is complete, so for say socket->receive(packet), it will wait until at least some data
                //has been passed to it, so essentially freezes the program, so it'd be good if this blocking occurred, but for every single socket at the same time,
                //i.e wait for any sort of activity and process it
                //this is achieved using a selector
                sf::SocketSelector selector; //a selector, which, once a socket of any type is added to it, listens and waits for activity from any of them, and let's them proceed
                sf::Clock expiryTimer; //this is the global clock against which long polling is employed
                //basically is used to check how long it's been since the server was last pinged
            public:
                //prototypes
                void removeUser(unsigned int i);
                void server(unsigned short PORT);
                void process();
                void saveMsgDB(std::string msg, int userNum, int time);
                void forwardToAllUsers(std::string msg, int userNum);
                void broadcastToLocal(std::string msg, int userNum);
                bool login(std::string input, ecs::component::user* userPtr);
        };

        struct updateActiveChunkData{
            private:
                static updateActiveChunkData* instance;
                updateActiveChunkData();
            public:
                static updateActiveChunkData* getInstance();
                std::unordered_map<coordinatesStruct, bool, Hash> activeChunks; 
                //can be used for selecting data to be extracted from the ECS and put into the gameData object, and for physics engine related stuff

                //the map would contain the coordinates of chunks, and whether or not they are active, loops through the locations component vector to get the coordinates of users
                //and using simple modulus math, it can find what chunk a user is in (using chunkPixelSize_x and chunkPixelSize_y), and insert or remove entries for active chunks
                //as neccessary
                void updateActiveChunks(); //this would update the activeChunks map, removing inactive ones, adding new ones which should be set to active
                void updateChunkData(); //this would get the data associated with some chunk and store it in the gameData object
                
                std::vector<sf::Vector2f> retrievePlayerChunks(unsigned int entityID); 
                //using the entityID of the user, this should return the coordinates of 9 chunks who's data the player should receive, again, using simple 
                //mod math and chunkPixelSize_x/chunkPixelSize_y
                //it should access the location component, run the mod math on it, then it can literally use the numbers to directly access gameData and retrieve all of the
                //chunk data that a user needs
        };
        
        /*
            Note that as updateChunkData() will run on its own thread, and so will broadcastGameState(), but they will both require access to gameData, you'll
            need to use sf::Mutex to make sure that they don't simultaneously try to access resources

            Also you need to implement broadcastGameState() using UDP, so loop through the users component vector, then get the user IP address from the socket
            then get access to the associated location struct, then use simple modulus math on the coordinates to retrieve the 

            All chunk data is stored in gameData, but for internal processing for physics and the such it should be in the chunks map
        */
       
       class game{
           private:
                static game* instance;
                game();
            public:
                static game* getInstance();
                void runGame();
       };
       
        class systemsManager{
            private:
                unsigned int port;
            public:
                network networkObj;

                systemsManager(unsigned int PORT) : port(PORT) {};
                void systemStart();
                void systemEnd();

                sf::Thread* processNetwork = 0; //the thread for running the main receiving and sending processes
                sf::Thread* listenNetwork = 0; //listening for incoming connections

                sf::Thread* gameConnectServer = 0;
                sf::Thread* gameListen = 0;

                sf::Thread* mainGame = 0;
        };


        extern std::unordered_map<coordinatesStruct, std::vector<ecs::entity::entity>, Hash> chunks; //will contain a list of all the entities in each chunk, useful for physics + read below
        extern std::unordered_map<coordinatesStruct, json, Hash> gameData; //this basically tells the compiler that the variable declared is defined somewhere else in the program (main.cpp in this case)
    }
}

#endif