#include "../../header/ecs.h"
#include "../../header/helper.h"

using namespace ecs::system;
using namespace ecs::component;

void updateActiveChunkData::updateActiveChunks()
{ //this is for updating which chunks are actually active
	std::vector<coordinatesStruct> generationCoords = {}; //a vector containing all of the coordinates to generate a new chunk at
	for (auto &chunk : chunks)
	{
		if(chunk.second.first.userCount > 0){
			for(int i = chunk.first.coordinates.first-1; i <= chunk.first.coordinates.first+1;i++){
				for(int j = chunk.first.coordinates.second-1; j <= chunk.first.coordinates.second+1;j++){ //loops around all the surrounding chunks
					if(!chunks.count(coordinatesStruct(i, j))){ //if the chunk hasn't been made yet
						generationCoords.push_back(coordinatesStruct(i, j)); //flag this up for generation
					}
				}
			}
		}
	}
	
	//std::cout << generationCoords.size() << "\n";
	/*
	Generation:
	-Every power of 2 on the x-axis excluding the first 3 (so until the 8th chunk), generate a city there
		-> their y vaues are always multiples of 5
	*/

	std::vector<coordinatesStruct> airCoords; //used for the stairs or whatever requires air above

	bool generationFlag = false; //used to indicate if generation has happened or not (so to skip the semi random generation bit)
	for(auto &generation : generationCoords){
		unsigned int entityID = ecs::entity::superEntityManager.create({components::DRAWABLE, components::PHYSICAL}); //a new object with those attributes is made
		
		if(generation.coordinates.first == 0 && generation.coordinates.second%5==0){
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
			if(chance > 90 && chance < 95 && chunks[coordinatesStruct(generation.coordinates.first, generation.coordinates.second-1)].first.settingID != 1){ //so long as city isn't above
				chunks[generation].first.settingID = 4; //ID: 4 is stairs right
				airCoords.push_back(coordinatesStruct(generation.coordinates.first, generation.coordinates.second-1)); //-1 is above it
			}else if(chance > 95 && chunks[coordinatesStruct(generation.coordinates.first, generation.coordinates.second-1)].first.settingID != 1){ //so long as city isn't above
				chunks[generation].first.settingID = 5; //ID: 5 is stairs left
				airCoords.push_back(coordinatesStruct(generation.coordinates.first, generation.coordinates.second-1)); //-1 is above it
			}else{
				chunks[generation].first.settingID = 9; //ID: 9 is cave
			}
		}
		
		physicsObjects.compVec[physicsObjects.entityToVectorMap(entityID)].boxCorners = {
			sf::Vector2f(generation.coordinates.first * chunkPixelSize_x, (generation.coordinates.second * chunkPixelSize_y) + chunkPixelSize_y - 5), 
			sf::Vector2f((generation.coordinates.first * chunkPixelSize_x) + chunkPixelSize_x, (generation.coordinates.second * chunkPixelSize_y) + chunkPixelSize_y - 5),
			sf::Vector2f(generation.coordinates.first * chunkPixelSize_x, (generation.coordinates.second * chunkPixelSize_y) + chunkPixelSize_y), 
			sf::Vector2f((generation.coordinates.first * chunkPixelSize_x) + chunkPixelSize_x, (generation.coordinates.second * chunkPixelSize_y) + chunkPixelSize_y)
		}; //sets the corners of these boxes, remember negative is up
		physicsObjects.compVec[physicsObjects.entityToVectorMap(entityID)].objType = COLLISION; //sets the object type
		
		chunks[generation].second.push_back(entityID); //pushes the floor entity to the chunks object
	}

	for(auto &generateAir : airCoords){ 
		//generates air above stairs chunks, which does cause an air chunk to potentially suddenly appear on one of an active user's surrounding chunks, but not a huge issue
		chunks[generateAir].first.settingID = 0; //ID: 0 is air
	}
}

void updateActiveChunkData::updateChunkData()
{					  //this is for updating the gameData object
	gameData.clear(); //empties the gameData object
	for (auto &chunkEntityVector : chunks){
		gameData[chunkEntityVector.first] = json::object();
		for (int i = 0; i < chunkEntityVector.second.second.size(); i++)
		{
			std::lock_guard<std::mutex> mutex(mutexs::removeUserMutex);	//locks the mutex so user can't logout while this is being updated or vice versa
			int entityID = chunkEntityVector.second.second[i].id;

			if(users.entityToVectorMap(entityID) != -1){ //it must be a user
				gameData[chunkEntityVector.first]["entities"][i]["type"] = "USER"; //its type is user, use these sort of capital letter words to describe the entity 'type'
			}else if(drawables.entityToVectorMap(entityID) != -1 && physicsObjects.entityToVectorMap(entityID) != -1){ //its probably floor or wall or something
				gameData[chunkEntityVector.first]["entities"][i]["type"] = "COLLISION";
			}else{
				gameData[chunkEntityVector.first]["entities"][i]["type"] = "OTHER"; //this should never be here, but just to be safe
			}
			//add in other conditions like these for mobs, items, or other things
			
			//the below are the generic information stuff

			if(users.entityToVectorMap(entityID) != -1){ //entityToVectorMap returns -1 if the entityID is not found in its bimap, so we just use that to find
				unsigned int usersIndex = users.entityToVectorMap(entityID);
				gameData[chunkEntityVector.first]["entities"][i]["username"] = users.compVec[usersIndex].username;
				gameData[chunkEntityVector.first]["entities"][i]["avatar"] = users.compVec[usersIndex].avatar;
				gameData[chunkEntityVector.first]["entities"][i]["id"] = users.compVec[usersIndex].userID;
			}
			
			if(drawables.entityToVectorMap(entityID) != -1){
				unsigned int drawablesIndex = drawables.entityToVectorMap(entityID);
				gameData[chunkEntityVector.first]["entities"][i]["direction"]["x"] = drawables.compVec[drawablesIndex].direction.x;
				gameData[chunkEntityVector.first]["entities"][i]["direction"]["y"] = drawables.compVec[drawablesIndex].direction.y;
			}

			if(physicsObjects.entityToVectorMap(entityID) != -1){
				unsigned int physicsIndex = physicsObjects.entityToVectorMap(entityID);
				gameData[chunkEntityVector.first]["entities"][i]["location"]["x"] = physicsObjects.compVec[physicsIndex].coordinates.x;
				gameData[chunkEntityVector.first]["entities"][i]["location"]["y"] = physicsObjects.compVec[physicsIndex].coordinates.y;
				//the hit box stuff
				gameData[chunkEntityVector.first]["entities"][i]["hitBox"]["top-left"]["x"] = physicsObjects.compVec[physicsIndex].boxCorners[0].x;
				gameData[chunkEntityVector.first]["entities"][i]["hitBox"]["top-left"]["y"] = physicsObjects.compVec[physicsIndex].boxCorners[0].y;
				gameData[chunkEntityVector.first]["entities"][i]["hitBox"]["bottom-right"]["x"] = physicsObjects.compVec[physicsIndex].boxCorners[3].x;
				gameData[chunkEntityVector.first]["entities"][i]["hitBox"]["bottom-right"]["y"] = physicsObjects.compVec[physicsIndex].boxCorners[3].y;
			}

		}
		gameData[chunkEntityVector.first]["data"]["x"] = chunkEntityVector.first.coordinates.first * chunkPixelSize_x;
		gameData[chunkEntityVector.first]["data"]["y"] = chunkEntityVector.first.coordinates.second * chunkPixelSize_y;
		gameData[chunkEntityVector.first]["data"]["width"] = chunkPixelSize_x;
		gameData[chunkEntityVector.first]["data"]["height"] = chunkPixelSize_y;
		gameData[chunkEntityVector.first]["data"]["userCount"] = chunkEntityVector.second.first.userCount;
		gameData[chunkEntityVector.first]["data"]["entityCount"] = chunkEntityVector.second.second.size();
		gameData[chunkEntityVector.first]["data"]["setting_id"] = chunkEntityVector.second.first.settingID;
	}
}
