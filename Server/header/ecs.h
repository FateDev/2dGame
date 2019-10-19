#ifndef ECS_HEAD
#define ECS_HEAD

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <initializer_list>
#include <SFML/Network.hpp>

namespace ecs{
    namespace component{
        struct user{ //struct to hold user's sockets, usernames, and the time left until their socket expires
            int userID;
            bool loggedIn;
            std::string username;
            std::string roomGuild = "main.alpha";
            std::string tempNewestMsgID = "";
            std::string accessToken = "";
            sf::TcpSocket* socket;
            sf::Time timeOfExpiry;
        };

        struct location{
            sf::Vector2f coordinates = { 0, 0 };
        };

        enum components {USER, LOCATION}; //enum for all of the components

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

        extern ecsComponentStructure<user> users;
        extern ecsComponentStructure<location> locationStructs;
    }
    
    namespace entity{
        struct entity{
            unsigned int id; //basic struct to say that this is actually an entity

            bool operator==(const entity& t) const { 
                return (this->id == t.id); 
            }
        };

        struct Hash {
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
        
        extern entityManager superEntityManager;
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