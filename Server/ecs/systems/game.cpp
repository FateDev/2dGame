#include "../../header/ecs.h"
#include <chrono>

using namespace ecs::system;
using namespace ecs::component;

void game::runGame()
{
	while (true){
		std::chrono::system_clock::time_point timePoint = std::chrono::system_clock::now() + std::chrono::milliseconds(1000/refreshRate);

		mutexs::chunkLockMutex.lock();
		physics::getInstance()->moveEntities();
		updateActiveChunkData::getInstance()->updateChunkData();
		mobSystem::getInstance()->mobGeneration();
		mutexs::chunkLockMutex.unlock();

		std::this_thread::sleep_until(timePoint); //sleeps until time to process everything again
	}
}

void game::broadcastGame(){
	while(true){
		std::chrono::system_clock::time_point timePoint = std::chrono::system_clock::now() + std::chrono::milliseconds(1000/fps);

		mutexs::readGameDataMutex.lock();
		gameBroadcast::getInstance()->broadcastGameState();
		mutexs::readGameDataMutex.unlock();

		std::this_thread::sleep_until(timePoint); //sleeps until time to broadcast again
	}
}