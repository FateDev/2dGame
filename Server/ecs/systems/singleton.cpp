#include "../../header/ecs.h"

using namespace ecs::system;

physics *physics::instance = 0; //to make it singleton
physics::physics() {}			//basically default constructor

physics *physics::getInstance()
{
	if (instance == 0)
	{
		instance = new physics();
	}
	return instance;
}

std::mutex mutexs::userLocationsMutex; //defines the mutex
std::mutex mutexs::removeUserMutex; //defines the mutex

mutexs *mutexs::instance = 0;
mutexs::mutexs() {}
mutexs *mutexs::getInstance()
{
	if (instance == 0)
	{
		instance = new mutexs();
	}
	return instance;
}

game *game::instance = 0;
game::game() {}
game *game::getInstance() 
{
	if (instance == 0)
	{
		instance = new game();
	}
	return instance;
}

updateActiveChunkData *updateActiveChunkData::instance = 0;
updateActiveChunkData::updateActiveChunkData(){};
updateActiveChunkData *updateActiveChunkData::getInstance()
{
	if (instance == 0)
	{
		instance = new updateActiveChunkData();
	}
	return instance;
}

gameBroadcast *gameBroadcast::instance = 0;
gameBroadcast *gameBroadcast::getInstance()
{
	if (instance == 0)
	{
		instance = new gameBroadcast();
	}
	return instance;
}