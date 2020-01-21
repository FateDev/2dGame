#include "../../header/ecs.h"
#include "../../header/helper.h"

using namespace ecs::system;
using namespace ecs::component;

void updateActiveChunkData::initWorld(coordinatesStruct startCoord){ //generates the initial 9 chunks or so, these may be different to the rest
	std::vector<coordinatesStruct> generationCoords = {};
	for(int i = -1; i <= 1; i++){
		for(int j = -1; j <= 1; j++){
			generationCoords.push_back(coordinatesStruct(startCoord.coordinates.first + i, startCoord.coordinates.second + j));
		}
	}

	generateChunks(generationCoords, true); //and they should be permanent
}

void updateActiveChunkData::updateActiveChunks()
{ //this is for updating which chunks are actually active
	std::vector<coordinatesStruct> generationCoords = {}; //a vector containing all of the coordinates to generate a new chunk at
	std::vector<coordinatesStruct> deletionCoords = {}; //a vector containing all of the coordinates to delete chunks from
	for (auto &chunk : chunks) {
		if(chunk.second.first.userCount > 0){
			for(int i = chunk.first.coordinates.first-1; i <= chunk.first.coordinates.first+1;i++){
				for(int j = chunk.first.coordinates.second-1; j <= chunk.first.coordinates.second+1;j++){ //loops around all the surrounding chunks
					if(!chunks[coordinatesStruct(i, j)].first.generated){ //if the chunk hasn't been made yet
						generationCoords.push_back(coordinatesStruct(i, j)); //flag this up for generation
					}
				}
			}
		}else if(!chunk.second.first.permanent){ //if there are no users in this, then potentially flag it up for deletion
			bool usersPresentInSurroundingChunks = false; //any users present in how many surrounding chunks?
			for(int i = chunk.first.coordinates.first-1; i <= chunk.first.coordinates.first+1;i++){
				for(int j = chunk.first.coordinates.second-1; j <= chunk.first.coordinates.second+1;j++){
					if(chunks.count(coordinatesStruct(i, j))){
						if(chunks[coordinatesStruct(i, j)].first.userCount > 0){
							usersPresentInSurroundingChunks = true;
							break;
						}
					}
				}
				if(usersPresentInSurroundingChunks) { break; } //if users are present in surrounding chunk, no longer need to continue checking
			}
			if(!usersPresentInSurroundingChunks){ //as long as there are no users in it, and it's not flagged as a permanent chunk, delete it
				deletionCoords.push_back(chunk.first); //flag for deletion
			}
		}
	}
	
	/*
	Generation:
	-Every power of 2 on the x-axis excluding the first 3 (so until the 8th chunk), generate a city there
		-> their y vaues are always multiples of 5
	std::cout << "Deleting: " << deletionCoords.size() << "\n";
	std::cout << "Generating: " << generationCoords.size() << "\n";
	std::cout << "Total number: " << chunks.size() << "\n";
	*/

	cleanupChunks(deletionCoords); //deletes the deletionCoords ones
	generateChunks(generationCoords, false); //generates the ones at generationCoords, false is saying that these ones shouldn't be permanent
}

void updateActiveChunkData::cleanupChunks(std::vector<coordinatesStruct> deletionCoords){
	for(auto &deletion : deletionCoords){ //deletes the ones flagged for deletion
		for(auto &entitiesInChunk : chunks[deletion].second){
			if(ecs::entity::superEntityManager.getType(entitiesInChunk) == ecs::entity::entityType::MOB){
				chunks[deletion].first.mobCount--;
			}else if(ecs::entity::superEntityManager.getType(entitiesInChunk) == ecs::entity::entityType::NPC){
				chunks[deletion].first.npcCount--;
			}else if(ecs::entity::superEntityManager.getType(entitiesInChunk) == ecs::entity::entityType::ITEM_THROWN){
				chunks[deletion].first.itemCount--;
			}
			ecs::entity::superEntityManager.destroy(entitiesInChunk);
		}
		chunks.erase(deletion);
	}
}

void updateActiveChunkData::generateChunks(std::vector<coordinatesStruct> generationCoords, bool permanent){
	for(auto &generation : generationCoords){
		bool generationFlag = false; //used to indicate if generation has happened or not (so to skip the semi random generation bit)
		unsigned int entityID = ecs::entity::superEntityManager.create(ecs::entity::COLLISION_OBJECT); //a new object with those attributes is made
		
		if((generation.coordinates.first == 0 && generation.coordinates.second%5==0) || (generation.coordinates.first == 0 && generation.coordinates.second==0)){
			chunks[generation].first.settingID = 1; //ID: 1 is city
			generationFlag = true;
		}else if(std::floor((float)log2(abs(generation.coordinates.first))) == (float)log2(abs(generation.coordinates.first)) && generation.coordinates.second%5==0){
			int newChunkX = (int)log2(abs((float)generation.coordinates.first));
			if(newChunkX >= 3){ //don't want to generate for the first 8*chunkWidth pixels
				chunks[generation].first.settingID = 1; //ID: 1 is city
				chunks[coordinatesStruct(generation.coordinates.first, generation.coordinates.second+1)].first.settingID = 9; //below it a cave is generated (this is for the edge case when nothing is generated)
				generationFlag = true;
			}
		}
		
		if(!generationFlag){ //just remember up is negative, so chunk.y-1 is the y coordinate of chunks above it (which is used for checking if a city is above stairs below)
			int chance = rand() % 100 + 1;
			if(chance > 80 && chance < 90 && chunks[coordinatesStruct(generation.coordinates.first, generation.coordinates.second-1)].first.settingID != 1){ //so long as city isn't above
				chunks[generation].first.settingID = 4; //ID: 4 is stairs right
			}else if(chance > 90 && chunks[coordinatesStruct(generation.coordinates.first, generation.coordinates.second-1)].first.settingID != 1){ //so long as city isn't above
				chunks[generation].first.settingID = 5; //ID: 5 is stairs left
			}else{
				chunks[generation].first.settingID = 9; //ID: 9 is cave
			}
		}
		
		if(permanent){
			chunks[generation].first.permanent = true; //make it permanent on request
		}

		chunks[generation].first.generated = true; //it's been generated, so flag it as such
		
		auto physicsObj = &physicsObjects.compVec[physicsObjects.entityToVectorMap(entityID)];
		physicsObj->boxCorners = {
			sf::Vector2f(0, -5), 
			sf::Vector2f(chunkPixelSize_x, -5),
			sf::Vector2f(0, 0), 
			sf::Vector2f(chunkPixelSize_x, 0)
		}; //sets the corners of these boxes, remember negative is up
		physicsObj->objType = COLLISION; //sets the object type
		physicsObj->coordinates = {generation.coordinates.first * chunkPixelSize_x, generation.coordinates.second * chunkPixelSize_y};

		if(chunks[generation].first.settingID != 4 && chunks[generation].first.settingID != 5){
			chunks[generation].second.push_back(entityID); //pushes the floor entity to the chunks object
			if(chunks[generation].first.settingID != 1){
				mobSystem::getInstance()->generateMobsAt(generation); //generates mobs at these coordinates
			}
		}else{
			ecs::entity::superEntityManager.destroy(entityID); //we didn't use the entity so destroy it
		}
	}
}

void updateActiveChunkData::updateChunkData(){ //this is for updating the gameData object
	std::lock_guard<std::mutex> mutex(mutexs::mainUserLockMutex);	//locks the mutex so user can't logout while this is being updated or vice versa
	gameData.clear(); //empties the gameData object

	for(int i = 0; i < users.compVec.size(); i++){
		int entityID = users.vectorToEntityMap(i);
		sf::Vector2f *userCoordinates = &physicsObjects.compVec[physicsObjects.entityToVectorMap(entityID)].coordinates;
		for(int i = -1; i <= 1; i++){
			for(int j = -1; j <= 1; j++){
				coordinatesStruct chunkCoord = coordinatesStruct(chunkCoordHelperX(userCoordinates->x, chunkPixelSize_x) + i, chunkCoordHelperY(userCoordinates->y, chunkPixelSize_y) + j);
				if(gameData[chunkCoord].empty()){
					gameData[chunkCoord] = json::object();
					prepareGameData(chunkCoord); //prepares this chunk with its data, to be sent
				}
			}
		}
	}
}

void updateActiveChunkData::prepareGameData(coordinatesStruct coordinate){
	for (int i = 0; i < chunks[coordinate].second.size(); i++){
		int entityID = chunks[coordinate].second[i].id;
		
		switch (entity::superEntityManager.getType(entity::entity(entityID))){ //setting the type of the entity
			case entity::entityType::USER:
				gameData[coordinate]["entities"][i]["type"] = "USER"; //its type is user, use these sort of capital letter words to describe the entity 'type'
				break;
			case entity::entityType::COLLISION_OBJECT:
				gameData[coordinate]["entities"][i]["type"] = "COLLISION"; //probably the floor or wall or something similar
				break;
			case entity::entityType::MOB:
				gameData[coordinate]["entities"][i]["type"] = "MOB"; //it's probably a mob so set it as such
				break;
			case entity::entityType::ITEM_THROWN: {
				gameData[coordinate]["entities"][i]["type"] = "ITEM";
				gameData[coordinate]["entities"][i]["itemID"] = thrown_items.compVec[thrown_items.entityToVectorMap(entityID)].item_id; //sets the item ID
				break;
			}
			//add in other conditions like these for mobs, items, or other things
			default:{
				gameData[coordinate]["entities"][i]["type"] = "OTHER"; //this should never be true, but just to be safe
			}
		}
		
		//the below are the generic information stuff
		if(users.entityToVectorMap(entityID) != -1){ //entityToVectorMap returns -1 if the entityID is not found in its bimap, so we just use that to find
			unsigned int usersIndex = users.entityToVectorMap(entityID);
			gameData[coordinate]["entities"][i]["username"] = users.compVec[usersIndex].username;
			gameData[coordinate]["entities"][i]["id"] = users.compVec[usersIndex].userID;
			gameData[coordinate]["entities"][i]["itemID"] =  users.compVec[usersIndex].currentItem;
			gameData[coordinate]["entities"][i]["balance"] =  int(users.compVec[usersIndex].balance*100); //will be stored in std::atomic, so only divide by 100 there for displaying
		}

		if(mpHpObjects.entityToVectorMap(entityID) != -1){ //basically if it's a living thing add data for it
			unsigned int mpHpIndex = mpHpObjects.entityToVectorMap(entityID);
			//the below are all x100, so that I can use std::atomic int on client side, so no need to do locking, as that can be expensive
			gameData[coordinate]["entities"][i]["mp"] =  int(mpHpObjects.compVec[mpHpIndex].mp*100);
			gameData[coordinate]["entities"][i]["hp"] =  int(mpHpObjects.compVec[mpHpIndex].hp*100);
			gameData[coordinate]["entities"][i]["max_mp"] =  int(mpHpObjects.compVec[mpHpIndex].max_mp*100);
			gameData[coordinate]["entities"][i]["max_hp"] =  int(mpHpObjects.compVec[mpHpIndex].max_hp*100);
		}
		
		if(drawables.entityToVectorMap(entityID) != -1){
			unsigned int drawablesIndex = drawables.entityToVectorMap(entityID);
			gameData[coordinate]["entities"][i]["direction"]["x"] = drawables.compVec[drawablesIndex].direction.x;
			gameData[coordinate]["entities"][i]["direction"]["y"] = drawables.compVec[drawablesIndex].direction.y;
			gameData[coordinate]["entities"][i]["texture"] = drawables.compVec[drawablesIndex].texture;
		}

		if(physicsObjects.entityToVectorMap(entityID) != -1){
			unsigned int physicsIndex = physicsObjects.entityToVectorMap(entityID);
			gameData[coordinate]["entities"][i]["location"]["x"] = physicsObjects.compVec[physicsIndex].coordinates.x;
			gameData[coordinate]["entities"][i]["location"]["y"] = physicsObjects.compVec[physicsIndex].coordinates.y;
			//the hit box stuff, for anything other than a wall/floor it won't give the actual location, rather locations relative to the local origin
			gameData[coordinate]["entities"][i]["hitBox"]["top-left"]["x"] = physicsObjects.compVec[physicsIndex].boxCorners[0].x;
			gameData[coordinate]["entities"][i]["hitBox"]["top-left"]["y"] = physicsObjects.compVec[physicsIndex].boxCorners[0].y;
			gameData[coordinate]["entities"][i]["hitBox"]["bottom-right"]["x"] = physicsObjects.compVec[physicsIndex].boxCorners[3].x;
			gameData[coordinate]["entities"][i]["hitBox"]["bottom-right"]["y"] = physicsObjects.compVec[physicsIndex].boxCorners[3].y;
		}

	}
	gameData[coordinate]["data"]["x"] = coordinate.coordinates.first * chunkPixelSize_x;
	gameData[coordinate]["data"]["y"] = coordinate.coordinates.second * chunkPixelSize_y;
	gameData[coordinate]["data"]["width"] = chunkPixelSize_x;
	gameData[coordinate]["data"]["height"] = chunkPixelSize_y;
	gameData[coordinate]["data"]["userCount"] = chunks[coordinate].first.userCount;
	gameData[coordinate]["data"]["entityCount"] = chunks[coordinate].second.size();
	gameData[coordinate]["data"]["setting_id"] = chunks[coordinate].first.settingID;
}