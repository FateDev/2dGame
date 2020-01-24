#include "../../header/ecs.h"
#include <chrono>

using namespace ecs::system;
using namespace ecs::component;

void game::runGame() {
	while (true){
		std::chrono::system_clock::time_point timePoint = std::chrono::system_clock::now() + std::chrono::milliseconds(1000/fps);
		mutexs::chunkLockMutex.lock();
		updateActiveChunkData::getInstance()->updateChunkData();
		mutexs::chunkLockMutex.unlock();
		std::this_thread::sleep_until(timePoint); //sleeps until time to process everything again
	}
}

void game::physicsLoop(){
	while (true){
		std::chrono::system_clock::time_point timePoint = std::chrono::system_clock::now() + std::chrono::milliseconds(1000/fps);
		mutexs::chunkLockMutex.lock();
		physics::getInstance()->moveEntities();
		mutexs::chunkLockMutex.unlock();
		std::this_thread::sleep_until(timePoint); //sleeps until time to process everything again
	}
}

void game::broadcastGameLoop(){
	while (true){
		std::chrono::system_clock::time_point timePoint = std::chrono::system_clock::now() + std::chrono::milliseconds(1000/fps);
		mutexs::chunkLockMutex.lock();
		gameBroadcast::getInstance()->broadcastGameState();
		mutexs::chunkLockMutex.unlock();
		std::this_thread::sleep_until(timePoint); //sleeps until time to process everything again
	}
}

void game::chunksUpdateLoop(){
	while(true){
		std::chrono::system_clock::time_point timePoint = std::chrono::system_clock::now() + std::chrono::milliseconds(50);
		mutexs::chunkLockMutex.lock();
		updateActiveChunkData::getInstance()->updateActiveChunks(); //updates the active chunks, we call it after anyone moves in any chunk as that's when a new chunk may need to be generated
		mutexs::chunkLockMutex.unlock();
		std::this_thread::sleep_until(timePoint); //sleeps until time to process everything again
	}
}

void game::updateInRangeLoop(){
	while(true){
		std::chrono::system_clock::time_point timePoint = std::chrono::system_clock::now() + std::chrono::milliseconds(200);
		mutexs::chunkLockMutex.lock();
		physics::getInstance()->updateEntitiesInRange(); //updates what entities are in range of what entity
		mutexs::chunkLockMutex.unlock();
		std::this_thread::sleep_until(timePoint); //sleeps until time to process everything again
	}
}

void game::damageLoop(){
	while(true){
		std::chrono::system_clock::time_point timePoint = std::chrono::system_clock::now() + std::chrono::milliseconds(100);
		mutexs::chunkLockMutex.lock();
		mpHpSystem::getInstance()->damage(); //updates what entities are in range of what entity
		mutexs::chunkLockMutex.unlock();
		std::this_thread::sleep_until(timePoint); //sleeps until time to process everything again
	}
}