#ifndef ECS_HEAD
#define ECS_HEAD

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <initializer_list>
#include <SFML/Network.hpp>
#include <thread>
#include <mutex>
#include <iostream>

#include "../header/helper.h"

//the size of one chunk
const int chunkPixelSize_x = 150;
const int chunkPixelSize_y = 50;
const int fps = 60;
const sf::Vector2f deceleration = { 0, 0.1 }; //x = friction on surface deceleration, y = gravity
const sf::Vector2f acceleration = { 0, 1 }; //x = acceleration by arrow keys (don't want to accelerate), y = acceleration when jumping up
const sf::Vector2f velocity = { 1, 0 }; //x = initial velocity with arrow keys, y = initial velocity with arrow keys (don't want any so zero)

#include "../deps/json.hpp"
using json = nlohmann::json;

namespace ecs{
    namespace entity{
        enum entityType {USER, MOB, COLLISION_OBJECT, OTHER}; //enum for the types of entities, only used for deleting them though
    }
}

namespace ecs{
    namespace component{
        enum objectType {COLLIDER, COLLISION, COLLIDER_COLLISION}; //enum for the object types, collider is like users/mobs, collision is like floors/walls (so don't move), collider-collision is both
        enum components {USER, DRAWABLE, PHYSICAL}; //enum for all of the components

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
            bool leave = false; //this will be used to flag when the user leaves
            std::string avatar = "";
        };

        struct drawable{
            sf::Vector2i direction = {0, 0}; //this is only used for drawing moving items, for accurate direction use the velocity
        };

        struct physical{
            sf::Vector2f coordinates = { 0, 0 }; //the in game location of the player
            std::vector<sf::Vector2f> boxCorners = {}; //the hit box corners, they are specified [0] is top left, [1] is top right, [2] is bottom left, [3] is bottom right
            sf::Vector2f velocity = { 0, 0 }; //this is the velocity of the player
            bool onFloor = true; //initialised to being on the floor
            objectType objType = COLLIDER; //initialised to being a collider object (users/mobs etc)
        };

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
        extern ecsComponentStructure<drawable> drawables;
        extern ecsComponentStructure<physical> physicsObjects;

        //to make a new componenent, (1) make an enum in ecs::component::components, and (2) optionally if it needs to be like a mob or something, also in entity::entityType
        //then (3) you need to put the extern bit here and put the (4) definitions of it in main.cpp
        //then you need to (5) put the explicit instantiations of the component structures at the bottom of components.cpp,
        //and (6) then add a way to initialise it in entityManager::create(...) (using the if statements) in entities.cpp,
        //and then (7) add a way to destroy it in entityManager::destroy(...), in entities.cpp
    }
    
    namespace entity{
        struct entity{
            unsigned int id; //basic struct to say that this is actually an entity
            //ecs::entity::entityType type = ecs::entity::USER; //the exact type of the entity, by default user

            entity(int id) : id(id) {}; //initialisation list constructor
            entity() {}; //default constructor

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
                bool alive(entity entityStruct); //will simply check if the provided entity exists in the entities set
                void destroy(entity entityStruct, ecs::entity::entityType type); //will destroy an entity, removing it from the above set, and removing all of its entries from the components struct
        };
        
        extern entityManager superEntityManager; //this basically tells the compiler that the variable declared is defined somewhere else in the program (main.cpp in this case)
    }

    namespace system{
        extern std::unordered_map<std::string, unsigned int>  sessionIDToEntityID; //will map user's uniqueID's to their entity ID

        struct chunkData{
            int settingID = 9; //1 is City, 9 is cave, have a look at that piece of paper is drew on
            int userCount = 0; //how many users are in this chunk
            bool permanent = false; //just checks if this should ever be deleted or not
        };
        
        struct mapCleanup{
            void chunksMapCleanup(); //should be run on a thread, should iterate through every single chunk, with something like 100ms sleep inbetween as it's not that important
            //it should basically just remove entities that don't exist anymore
        };

        struct coordinatesStruct{
            unsigned long uniqueID = 0; //equal to unsigned long int, using long just to be safe
            std::pair<int, int> coordinates;

            coordinatesStruct(int xCoord, int yCoord) {
                int a = xCoord < 0 ? (-2)*(xCoord) - 1 : 2*(xCoord); //makes negative variables positive for Cantors function, which only works for positive integers
                int b = yCoord < 0 ? (-2)*(yCoord) - 1 : 2*(yCoord); //makes negative variables positive for Cantors function, which only works for positive integers
                uniqueID = (0.5 * (a + b) * (a + b + 1)) + b; //Cantor pairing function, where pairing is to uniquely encode two natural numbers into a single natural number, used for the hashing function thing

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
                static std::mutex mainUserLockMutex; //used when sending data (gameBroadcast::broadcastGameState()), and logging the user out, and updating chunk data
                static std::mutex chunkLockMutex; //to be used to lock chunks, so used when sending data to local chunks, adding/removing users from chunks and incrementing/decrementing user count of a chunk
                static mutexs* getInstance();
        };
       
       class physics{
           private:
                static physics* instance;
                physics();
           public:
                static physics* getInstance();
                void userInput(json keysAndID);
                void moveEntities();
                bool checkCollision(entity::entity colliderEntity);
                bool AABB_collision(int collisionEntityID, int colliderEntityID);
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
                static network* instance;
                //sockets are normally in blocking mode, i.e, waits until a process is complete, so for say socket->receive(packet), it will wait until at least some data
                //has been passed to it, so essentially freezes the program, so it'd be good if this blocking occurred, but for every single socket at the same time,
                //i.e wait for any sort of activity and process it
                //this is achieved using a selector
                sf::SocketSelector selector; //a selector, which, once a socket of any type is added to it, listens and waits for activity from any of them, and let's them proceed
                sf::Clock expiryTimer; //this is the global clock against which long polling is employed
                //basically is used to check how long it's been since the server was last pinged
	            sf::TcpListener listener; //makes a tcp listener
            public:
                //prototypes
                static network* getInstance();
                network() { listener.listen(5000); };
                void removeUser(unsigned int i);
                void server();
                void process();
                void saveMsgDB(std::string msg, int userNum, int time);
                void forwardToAllUsers(std::string msg, int userNum);
                void broadcastToLocal(std::string msg, int userNum);
                void messageProcessing(); //simply to simplyify the entire process thread
                void getUserInventory(int userID, json* jsonObj); //will get the user's inventory
                void saveUserInventory(int userID, json jsonObj); //saves the user's inventory
                std::string login(std::string input, ecs::component::user *userPtr);
        };

        struct updateActiveChunkData{
            private:
                static updateActiveChunkData* instance;
                updateActiveChunkData();
            public:
                static updateActiveChunkData* getInstance();
                std::unordered_map<coordinatesStruct, bool, Hash> activeChunks; 
                //can be used for selecting data to be extracted from the ECS and put into the gameData object, and for physics engine related stuff

                void updateActiveChunks(); //this would update the activeChunks map, removing inactive ones, adding new ones which should be set to active
                void updateChunkData(); //this would get the data associated with some chunk and store it in the gameData object
                
                std::vector<sf::Vector2f> retrievePlayerChunks(unsigned int entityID); 
        };
       
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
                systemsManager(unsigned int PORT) : port(PORT) {};
                void systemStart();
                void systemEnd();

                sf::Thread* processNetwork = 0; //the thread for running the main receiving and sending processes
                sf::Thread* listenNetwork = 0; //listening for incoming connections

                sf::Thread* gameConnectServer = 0;
                sf::Thread* gameListen = 0;

                sf::Thread* mainGame = 0;
        };
        
        extern std::unordered_map<int, json> userInventories; //will contain json objects of user's inventories (user ID -> user inventory JSON)
        extern std::unordered_map<coordinatesStruct, std::pair<chunkData, std::vector<ecs::entity::entity>>, Hash> chunks; //will contain a list of all the entities in each chunk, useful for physics + read below
        extern std::unordered_map<coordinatesStruct, json, Hash> gameData; //this basically tells the compiler that the variable declared is defined somewhere else in the program (main.cpp in this case)
        //the pair in the chunk structure is used, so that I can store the entityID in the first value, and the entities of the scene in the second bit
    }
}

#endif