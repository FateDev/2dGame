#ifndef ECS_HEAD
#define ECS_HEAD

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <initializer_list>
#include <SFML/Network.hpp>

namespace ecs{
    namespace component{
        //the structs below are the components so far
        struct user{ 
            int userID; //the user ID from the database
            bool loggedIn; //whether or not they're logged in
            std::string username; //the username
            std::string roomGuild = "main.alpha"; //the current roomGuild for the social tab
            std::string tempNewestMsgID = ""; //the ID of the newest message
            std::string accessToken = ""; //for making sure there is only one client session active at a time
            sf::TcpSocket* socket; //the actual socket connection
            sf::Time timeOfExpiry; //the time till the socket should be removed, this is updated every time the client pings the server
        };

        struct location{
            sf::Vector2f coordinates = { 0, 0 }; //the in game location of the player
        };

        enum components {USER, LOCATION}; //enum for all of the components

        //it's using a template because it could be used for any of the components above
        template <class T>
        class ecsComponentStructure{
            private:
                std::unordered_map<unsigned int, unsigned int> entityVectorMap; //will map entities to vector indexes so we can move about vector elements without breaking indexes
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

        struct systemsManager{
            unsigned int port;
            network networkObj;

            systemsManager(unsigned int PORT) : port(PORT) {};
            void systemStart();
            void systemEnd();

            sf::Thread* processNetwork = 0; //the thread for running the main receiving and sending processes
            sf::Thread* listenNetwork = 0;
        };
    }
}

#endif