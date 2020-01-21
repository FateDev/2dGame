#include <game.h>
#include <header.h>
#include <sstream>
#include <fstream>

game::game(networking* networkObject, gameNetwork* gameConnection, sf::RenderWindow* gameWindow, sf::View* gameView) : networkObj(networkObject), gameNetworkObj(gameConnection), gameWindow(gameWindow), gameView(gameView) { 
	//I added in a map the opposite way round just in case
	sfKeyToAbstractKeyMap[sf::Keyboard::A] = "a"; abstractKeyTosfKeyMap["a"] = sf::Keyboard::A;
	sfKeyToAbstractKeyMap[sf::Keyboard::B] = "b"; abstractKeyTosfKeyMap["b"] = sf::Keyboard::B;
	sfKeyToAbstractKeyMap[sf::Keyboard::C] = "c"; abstractKeyTosfKeyMap["c"] = sf::Keyboard::C;
	sfKeyToAbstractKeyMap[sf::Keyboard::D] = "d"; abstractKeyTosfKeyMap["d"] = sf::Keyboard::D;
	sfKeyToAbstractKeyMap[sf::Keyboard::E] = "e"; abstractKeyTosfKeyMap["e"] = sf::Keyboard::E;
	sfKeyToAbstractKeyMap[sf::Keyboard::F] = "f"; abstractKeyTosfKeyMap["f"] = sf::Keyboard::F;
	sfKeyToAbstractKeyMap[sf::Keyboard::G] = "g"; abstractKeyTosfKeyMap["g"] = sf::Keyboard::G;
	sfKeyToAbstractKeyMap[sf::Keyboard::H] = "h"; abstractKeyTosfKeyMap["h"] = sf::Keyboard::H;
	sfKeyToAbstractKeyMap[sf::Keyboard::I] = "i"; abstractKeyTosfKeyMap["i"] = sf::Keyboard::I;
	sfKeyToAbstractKeyMap[sf::Keyboard::J] = "j"; abstractKeyTosfKeyMap["j"] = sf::Keyboard::J;
	sfKeyToAbstractKeyMap[sf::Keyboard::K] = "k"; abstractKeyTosfKeyMap["k"] = sf::Keyboard::K;
	sfKeyToAbstractKeyMap[sf::Keyboard::L] = "l"; abstractKeyTosfKeyMap["l"] = sf::Keyboard::L;
	sfKeyToAbstractKeyMap[sf::Keyboard::M] = "m"; abstractKeyTosfKeyMap["m"] = sf::Keyboard::M;
	sfKeyToAbstractKeyMap[sf::Keyboard::N] = "n"; abstractKeyTosfKeyMap["n"] = sf::Keyboard::N;
	sfKeyToAbstractKeyMap[sf::Keyboard::O] = "o"; abstractKeyTosfKeyMap["o"] = sf::Keyboard::O;
	sfKeyToAbstractKeyMap[sf::Keyboard::P] = "p"; abstractKeyTosfKeyMap["p"] = sf::Keyboard::P;
	sfKeyToAbstractKeyMap[sf::Keyboard::Q] = "q"; abstractKeyTosfKeyMap["q"] = sf::Keyboard::Q;
	sfKeyToAbstractKeyMap[sf::Keyboard::R] = "r"; abstractKeyTosfKeyMap["r"] = sf::Keyboard::R;
	sfKeyToAbstractKeyMap[sf::Keyboard::S] = "s"; abstractKeyTosfKeyMap["s"] = sf::Keyboard::S;
	sfKeyToAbstractKeyMap[sf::Keyboard::T] = "t"; abstractKeyTosfKeyMap["t"] = sf::Keyboard::T;
	sfKeyToAbstractKeyMap[sf::Keyboard::U] = "u"; abstractKeyTosfKeyMap["u"] = sf::Keyboard::U;
	sfKeyToAbstractKeyMap[sf::Keyboard::V] = "v"; abstractKeyTosfKeyMap["v"] = sf::Keyboard::V;
	sfKeyToAbstractKeyMap[sf::Keyboard::W] = "w"; abstractKeyTosfKeyMap["w"] = sf::Keyboard::W;
	sfKeyToAbstractKeyMap[sf::Keyboard::X] = "x"; abstractKeyTosfKeyMap["x"] = sf::Keyboard::X;
	sfKeyToAbstractKeyMap[sf::Keyboard::Y] = "y"; abstractKeyTosfKeyMap["y"] = sf::Keyboard::Y;
	sfKeyToAbstractKeyMap[sf::Keyboard::Z] = "z"; abstractKeyTosfKeyMap["z"] = sf::Keyboard::Z;
	sfKeyToAbstractKeyMap[sf::Keyboard::Num0] = "0"; abstractKeyTosfKeyMap["Num0"] = sf::Keyboard::Num0;
	sfKeyToAbstractKeyMap[sf::Keyboard::Num1] = "1"; abstractKeyTosfKeyMap["Num1"] = sf::Keyboard::Num1;
	sfKeyToAbstractKeyMap[sf::Keyboard::Num2] = "2"; abstractKeyTosfKeyMap["Num2"] = sf::Keyboard::Num2;
	sfKeyToAbstractKeyMap[sf::Keyboard::Num3] = "3"; abstractKeyTosfKeyMap["Num3"] = sf::Keyboard::Num3;
	sfKeyToAbstractKeyMap[sf::Keyboard::Num4] = "4"; abstractKeyTosfKeyMap["Num4"] = sf::Keyboard::Num4;
	sfKeyToAbstractKeyMap[sf::Keyboard::Num5] = "5"; abstractKeyTosfKeyMap["Num5"] = sf::Keyboard::Num5;
	sfKeyToAbstractKeyMap[sf::Keyboard::Num6] = "6"; abstractKeyTosfKeyMap["Num6"] = sf::Keyboard::Num6;
	sfKeyToAbstractKeyMap[sf::Keyboard::Num7] = "7"; abstractKeyTosfKeyMap["Num7"] = sf::Keyboard::Num7;
	sfKeyToAbstractKeyMap[sf::Keyboard::Num8] = "8"; abstractKeyTosfKeyMap["Num8"] = sf::Keyboard::Num8;
	sfKeyToAbstractKeyMap[sf::Keyboard::Num9] = "9"; abstractKeyTosfKeyMap["Num9"] = sf::Keyboard::Num9;
	sfKeyToAbstractKeyMap[sf::Keyboard::LControl] = "lcontrol"; abstractKeyTosfKeyMap["lcontrol"] = sf::Keyboard::LControl;
	sfKeyToAbstractKeyMap[sf::Keyboard::RControl] = "rcontrol"; abstractKeyTosfKeyMap["rcontrol"] = sf::Keyboard::RControl;
	sfKeyToAbstractKeyMap[sf::Keyboard::LShift] = "lshift"; abstractKeyTosfKeyMap["lshift"] = sf::Keyboard::LShift;
	sfKeyToAbstractKeyMap[sf::Keyboard::RShift] = "rshift"; abstractKeyTosfKeyMap["rshift"] = sf::Keyboard::RShift;
	sfKeyToAbstractKeyMap[sf::Keyboard::Space] = "space"; abstractKeyTosfKeyMap["space"] = sf::Keyboard::Space;
	sfKeyToAbstractKeyMap[sf::Keyboard::Tab] = "tab"; abstractKeyTosfKeyMap["tab"] = sf::Keyboard::Tab;

	//below initialises the JSON object
	keysObject["left"] = false;
	keysObject["right"] = false;
	keysObject["jump"] = false;
	keysObject["interactNPC"] = false;
	keysObject["switchTarget"] = false;
	keysObject["throwItem"] = false;
	keysObject["custom_binding1"] = false;
	keysObject["custom_binding2"] = false;
	keysObject["custom_binding3"] = false;

	keysObject["sessionID"] = networkObj->sessionID; //adds the session ID for the basic auth

	font.loadFromFile("Lato-Regular.ttf");

	playerTexture.loadFromFile("resources/default_character.png");
	opponentTexture.loadFromFile("resources/default_character.png");

	sf::Texture texture;
	texture.loadFromFile("resources/default_character.png");
	textures["default"] = texture;

	texture = sf::Texture();
	texture.loadFromFile("resources/mob_1.png");
	textures["mob_1"] = texture;

	std::ifstream itemsJSONFile("items.json");
	std::stringstream jsonContents;
	jsonContents << itemsJSONFile.rdbuf(); //reads into read buffer
	itemsFromFile = json::parse(jsonContents.str()); //puts into object

	for (int i = 0; i < itemsFromFile.size(); i++) {
		texture = sf::Texture();
		texture.loadFromFile(itemsFromFile[i]["resourceLocation"].get<std::string>());
		textures[std::to_string(itemsFromFile[i]["itemID"].get<int>())] = texture;
	}
}

void game::listenForKeys(sf::Event event) {
	if (event.type == sf::Event::KeyPressed) { //keysObject[key] = true/false; true = key is pressed, false = key is not pressed/released
		std::string key = sfKeyToAbstractKeyMap[event.key.code];
		if (key==networkObj->settings["left"].get<std::string>() || key == networkObj->settings["right"].get<std::string>() || key == networkObj->settings["jump"].get<std::string>() || key == networkObj->settings["interactNPC"].get<std::string>() || key == networkObj->settings["throwItem"].get<std::string>() || key == networkObj->settings["switchTarget"].get<std::string>() || key == networkObj->settings["custom_binding1"]["key"].get<std::string>() || key == networkObj->settings["custom_binding2"]["key"].get<std::string>() || key == networkObj->settings["custom_binding3"]["key"].get<std::string>()) {
			if (key == networkObj->settings["left"].get<std::string>() && keysObject["left"] != true) {
				keysObject["left"] = true;
				changeInButtonState = true; //used to indicate that s  ome buttons have been pressed
			}else if (key == networkObj->settings["right"].get<std::string>() && keysObject["right"] != true) {
				keysObject["right"] = true;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["jump"].get<std::string>() && keysObject["jump"] != true) {
				keysObject["jump"] = true;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["interactNPC"].get<std::string>() && keysObject["interactNPC"] != true) {
				keysObject["interactNPC"] = true;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["switchTarget"].get<std::string>() && keysObject["switchTarget"] != true) {
				keysObject["switchTarget"] = true;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["throwItem"].get<std::string>() && keysObject["throwItem"] != true) {
				keysObject["throwItem"] = true;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["custom_binding1"]["key"].get<std::string>() && keysObject["custom_binding1"] != true) {
				keysObject["custom_binding1"] = true;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["custom_binding2"]["key"].get<std::string>() && keysObject["custom_binding2"] != true) {
				keysObject["custom_binding2"] = true;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["custom_binding3"]["key"].get<std::string>() && keysObject["custom_binding3"] != true) {
				keysObject["custom_binding3"] = true;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}
		}
		else { //the ones above are the keys to tell the server about, the ones below should deal with stuff like opening inventory and other stuff like that
			//so stuff like pressing open inventory button or whatever
		}
	}

	if (event.type == sf::Event::KeyReleased) { //this would be used to stop moving for example
		std::string key = sfKeyToAbstractKeyMap[event.key.code];
		if (key == networkObj->settings["left"].get<std::string>() || key == networkObj->settings["right"].get<std::string>() || key == networkObj->settings["jump"].get<std::string>() || key == networkObj->settings["throwItem"].get<std::string>() || key == networkObj->settings["custom_binding1"]["key"].get<std::string>() || key == networkObj->settings["custom_binding2"]["key"].get<std::string>() || key == networkObj->settings["custom_binding3"]["key"].get<std::string>()) {
			if (key == networkObj->settings["left"].get<std::string>() && keysObject["left"] != false) {
				keysObject["left"] = false;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["right"].get<std::string>() && keysObject["right"] != false) {
				keysObject["right"] = false;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["jump"].get<std::string>() && keysObject["jump"] != false) {
				keysObject["jump"] = false;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["interactNPC"].get<std::string>() && keysObject["interactNPC"] != false) {
				keysObject["interactNPC"] = false;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["switchTarget"].get<std::string>() && keysObject["switchTarget"] != false) {
				keysObject["switchTarget"] = false;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["throwItem"].get<std::string>() && keysObject["throwItem"] != false) {
				keysObject["throwItem"] = false;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["custom_binding1"]["key"].get<std::string>() && keysObject["custom_binding1"] != false) {
				keysObject["custom_binding1"] = false;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["custom_binding2"]["key"].get<std::string>() && keysObject["custom_binding2"] != false) {
				keysObject["custom_binding2"] = false;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["custom_binding3"]["key"].get<std::string>() && keysObject["custom_binding3"] != false) {
				keysObject["custom_binding3"] = false;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}
		}
	}
}

void game::draw() { //this is called from the tcpGameThread, so not on the main thread, so we used mutexs
	gameNetworkObj->drawMutex.lock();

	rectanglesToDraw.clear(); //this empties the container of rectangles, which are drawn to the screen
	circlesToDraw.clear();
	textToDraw.clear();
	spritesToDraw.clear();
	texturesToUse.clear();
	backgroundRectanglesToDraw.clear();
	spriteAccessories.clear();

	if (!gameData.is_null()) {	//it now moves it
		for (int j = 0; j < 9; j++) { //the server sends 9 chunks of data
			json chunkToDraw = json::parse(gameData["chunks"][j].get<std::string>());

			int scaleFactor = 11; //rather than changing server side stuff, just change this to make everything appear correctly

			if (chunkToDraw.is_null()) {
				//std::cout << "Null" << "\n";
			}

			if (chunkToDraw.is_null()) { //if the data is null skip it (it only happens for the edge of what can be seen for cities, not a huge issue but should fix it (it's a server side issue)
				continue;
			}

			//std::cout << chunkToDraw.dump() << std::endl;
			sf::Vector2i chunkDimensions(chunkToDraw["data"]["width"].get<int>(), chunkToDraw["data"]["height"].get<int>()); //the size of the chunk
			sf::Vector2i chunkOrigin(chunkToDraw["data"]["x"].get<int>(), chunkToDraw["data"]["y"].get<int>() - chunkDimensions.y); //the origin of the chunk
			//I'm basically using a hack to make it appear correct, as for some reason the setting appears shifted down by 1 chunk, so we start above the city rather than in it
			//This fixes it
			//std::cout << chunkToDraw.dump() << std::endl;
			if (!chunkToDraw.is_null()) {
				//std::cout << chunkOrigin.x << " -- " << chunkOrigin.y << " # " << chunkToDraw["data"]["setting_id"].get<int>() << std::endl;
				sf::RectangleShape rectangle(sf::Vector2f(chunkDimensions.x * scaleFactor, chunkDimensions.y * scaleFactor));
				rectangle.setPosition(sf::Vector2f(chunkOrigin.x * scaleFactor, chunkOrigin.y * scaleFactor));
				
				switch (chunkToDraw["data"]["setting_id"].get<int>()) {
				case 1:
					rectangle.setFillColor(sf::Color(29, 34, 39)); //city blue
					break;
				case 9:
					rectangle.setFillColor(sf::Color(17, 19, 22)); //everything else grey
					break;
				case 4:
					rectangle.setFillColor(sf::Color(231, 76, 60)); //red right stairs
					break;
				case 5:
					rectangle.setFillColor(sf::Color(241, 196, 15)); //yellow left stairs
					break;
				case 0:
					rectangle.setFillColor(sf::Color(211, 84, 0)); //orange air
					break;
				}

				backgroundRectanglesToDraw.push_back(rectangle);; //simply add a RectangleShape to the container for it to be drawn
				//std::cout << chunkToDraw["entityCount"].dump() << std::endl;
				for (int i = 0; i < chunkToDraw["data"]["entityCount"]; i++) {
					if (!chunkToDraw["entities"][i].is_null()) { //if the entity is actually null, just skip it
						if (chunkToDraw["entities"][i]["type"].get<std::string>() == "USER") {
							sf::Sprite player;
							player.setOrigin(sf::Vector2f(0, 72));

							//std::cout << "Carrying: resources/items/" << chunkToDraw["entities"][i]["itemID"].get<int>() << ".png" << "\n";
							sf::RectangleShape carryingItem({ 30, 30 });
							carryingItem.setOrigin(15, 15);
							carryingItem.setPosition(chunkToDraw["entities"][i]["location"]["x"].get<float>() * scaleFactor, (chunkToDraw["entities"][i]["location"]["y"].get<float>()) * scaleFactor - 30);

							if (chunkToDraw["entities"][i]["itemID"].get<int>() != 0) { //if it's not zero it means there is a texture to draw
								carryingItem.setTexture(&textures[std::to_string(chunkToDraw["entities"][i]["itemID"].get<int>())]); //will use the correct texture which was loaded from the items.json file and image files
							}

							if (chunkToDraw["entities"][i]["direction"]["x"].get<int>() == 1) {
								player.setOrigin(sf::Vector2f(0, 72));
								player.setScale(1.0, 1.0);
								carryingItem.move({ 30, 0 });
								carryingItem.rotate(45);
							}
							else if (chunkToDraw["entities"][i]["direction"]["x"].get<int>() == -1) {
								player.setOrigin(sf::Vector2f(50, 72));
								player.setScale(-1.0, 1.0);
								carryingItem.move({ 15, 0 });
								carryingItem.rotate(-45);
							}
							else {
								carryingItem.move({ 5, 0 });
							}

							//carryingItem.rotate(45); //rotate it by 45 degrees

							if (chunkToDraw["entities"][i]["id"].get<int>() == networkObj->userID) { //do some operations meant for only this client
								//player.setFillColor(sf::Color::Blue);
								player.setTexture(playerTexture);
								gameView->setCenter(chunkToDraw["entities"][i]["location"]["x"].get<float>() * scaleFactor, (chunkToDraw["entities"][i]["location"]["y"].get<float>()) * scaleFactor);

								if (statsObj) {
									//no need to use locking, as the values used are all atomic types
									statsObj->setBalance(chunkToDraw["entities"][i]["balance"].get<int>());
									statsObj->setHP(chunkToDraw["entities"][i]["hp"].get<int>());
									statsObj->setMP(chunkToDraw["entities"][i]["mp"].get<int>());
									statsObj->setMaxHp(chunkToDraw["entities"][i]["max_hp"].get<int>());
									statsObj->setMaxMp(chunkToDraw["entities"][i]["max_mp"].get<int>());
								}
							}
							else {
								//player.setFillColor(sf::Color::Red);
								player.setTexture(opponentTexture);
							}

							float hpRatio = (chunkToDraw["entities"][i]["hp"].get<float>() / 100) / (chunkToDraw["entities"][i]["max_hp"].get<float>() / 100);
							sf::RectangleShape hpLeft;
							sf::RectangleShape hpRight;
							hpLeft.setPosition(chunkToDraw["entities"][i]["location"]["x"].get<float>() * scaleFactor, (chunkToDraw["entities"][i]["location"]["y"].get<float>()) * scaleFactor - 115);
							hpRight.setPosition(chunkToDraw["entities"][i]["location"]["x"].get<float>() * scaleFactor + (50 * hpRatio), (chunkToDraw["entities"][i]["location"]["y"].get<float>()) * scaleFactor - 115);
							hpLeft.setSize({ 50*hpRatio, 12 });
							hpRight.setSize({ 50*(1-hpRatio), 12 });
							hpLeft.setFillColor(sf::Color(46, 204, 113));
							hpRight.setFillColor(sf::Color(192, 57, 43));

							player.setPosition(chunkToDraw["entities"][i]["location"]["x"].get<float>() * scaleFactor, (chunkToDraw["entities"][i]["location"]["y"].get<float>()) * scaleFactor);
							//std::cout << chunkToDraw["entities"][i]["direction"]["x"].get<float>() << " -- " << chunkToDraw["entities"][i]["direction"]["y"].get<float>() << "aa \n";
							sf::Text text;
							text.setFont(font);
							text.setString(chunkToDraw["entities"][i]["username"].get<std::string>());
							text.setCharacterSize(24);
							text.setPosition(chunkToDraw["entities"][i]["location"]["x"].get<float>() * scaleFactor, (chunkToDraw["entities"][i]["location"]["y"].get<float>()) * scaleFactor - 103);

							if (chunkToDraw["entities"][i]["itemID"].get<int>() != 0) {
								spriteAccessories.push_back(carryingItem); //draw the player's item's texture if it's non zero, so there is a texture for it
							}

							spriteAccessories.push_back(hpLeft);
							spriteAccessories.push_back(hpRight);
							textToDraw.push_back(text);
							spritesToDraw.push_back(player);
						}
						else if (chunkToDraw["entities"][i]["type"].get<std::string>() == "COLLISION") {
							float width = abs(chunkToDraw["entities"][i]["hitBox"]["top-left"]["x"].get<float>() - chunkToDraw["entities"][i]["hitBox"]["bottom-right"]["x"].get<float>());
							float height = abs(chunkToDraw["entities"][i]["hitBox"]["top-left"]["y"].get<float>() - chunkToDraw["entities"][i]["hitBox"]["bottom-right"]["y"].get<float>());

							//std::cout << chunkToDraw["entities"][i]["hitBox"]["top-left"].dump() << " -- " << chunkToDraw["entities"][i]["hitBox"]["bottom-right"].dump() << " -- " << width << " -- " << height << " # " << chunkToDraw["data"]["setting_id"].get<int>() << std::endl;
							sf::RectangleShape rectangle(sf::Vector2f(width * scaleFactor, height * scaleFactor));

							rectangle.setPosition({ (chunkToDraw["entities"][i]["hitBox"]["top-left"]["x"].get<float>() + chunkToDraw["entities"][i]["location"]["x"].get<float>()) * scaleFactor, (chunkToDraw["entities"][i]["hitBox"]["top-left"]["y"].get<float>() + chunkToDraw["entities"][i]["location"]["y"].get<float>()) * scaleFactor });
							rectangle.setFillColor(sf::Color(51, 56, 63)); //grey floor
							backgroundRectanglesToDraw.push_back(rectangle);; //simply add a RectangleShape to the container for it to be drawn
						}
						else if (chunkToDraw["entities"][i]["type"].get<std::string>() == "ITEM") {
							sf::RectangleShape carryingItem({ 20, 20 });
							carryingItem.setOrigin(0, 20);
							carryingItem.setPosition(chunkToDraw["entities"][i]["location"]["x"].get<float>() * scaleFactor, (chunkToDraw["entities"][i]["location"]["y"].get<float>()) * scaleFactor);

							carryingItem.setTexture(&textures[std::to_string(chunkToDraw["entities"][i]["itemID"].get<int>())]); //will use the texture in the map (set above)
							rectanglesToDraw.push_back(carryingItem); //draw the player's item's texture if it's non zero, so there is a texture for it
						}
						else if (chunkToDraw["entities"][i]["type"].get<std::string>() == "MOB") {
							float width = abs(chunkToDraw["entities"][i]["hitBox"]["top-left"]["x"].get<float>() - chunkToDraw["entities"][i]["hitBox"]["bottom-right"]["x"].get<float>());
							float height = abs(chunkToDraw["entities"][i]["hitBox"]["top-left"]["y"].get<float>() - chunkToDraw["entities"][i]["hitBox"]["bottom-right"]["y"].get<float>());

							//std::cout << chunkToDraw["entities"][i]["hitBox"]["top-left"].dump() << " -- " << chunkToDraw["entities"][i]["hitBox"]["bottom-right"].dump() << " -- " << width << " -- " << height << " # " << chunkToDraw["data"]["setting_id"].get<int>() << std::endl;
							//std::cout << width << " -- " << height << "\n";
							sf::RectangleShape rectangle(sf::Vector2f(width * scaleFactor, height * scaleFactor));
							rectangle.setTexture(&textures["mob_1"]);
							if (chunkToDraw["entities"][i]["direction"]["x"].get<int>() == 1) {
								rectangle.setOrigin(sf::Vector2f(50, 0));
								rectangle.setScale(-1.0, 1.0);
							}
							//std::cout << chunkToDraw["entities"][i]["direction"]["x"].get<int>() << "\n";
							//std::cout << chunkToDraw["entities"][i]["location"]["y"].dump() << "\n";
							rectangle.setPosition({ (chunkToDraw["entities"][i]["hitBox"]["top-left"]["x"].get<float>() + chunkToDraw["entities"][i]["location"]["x"].get<float>()) * scaleFactor, (chunkToDraw["entities"][i]["hitBox"]["top-left"]["y"].get<float>() + chunkToDraw["entities"][i]["location"]["y"].get<float>()) * scaleFactor });
							//rectangle.move(0, -20);
							//rectangle.setFillColor(sf::Color(211, 84, 0)); //grey floor
							rectanglesToDraw.push_back(rectangle);; //simply add a RectangleShape to the container for it to be drawn
						}
					}
				}
			}
		}
	}

	gameNetworkObj->drawMutex.unlock();
}

void game::live(inventory* inventoryBit) { //this is called from the main thread
	if (!inventoryBit->isInventoryOpen()) {
		if (changeInButtonState && !networkObj->msgBoxFocused) {
			gameNetworkObj->sendData(keysObject); //sends the object to the server

			//we're resetting some of them here, as they just do a single action, rather than move or whatever
			keysObject["interactNPC"] = false;
			keysObject["switchTarget"] = false;
			keysObject["jump"] = false; //we want to be able to jump repeatedly

			changeInButtonState = false; //reset this so that we don't repeatedly send the same data to the server
		}

		if (sf::Keyboard::isKeyPressed(abstractKeyTosfKeyMap[networkObj->settings["jump"].get<std::string>()])) { //this is just so the user can continuously jump if they so wish
			keysObject["jump"] = true;
			changeInButtonState = true; //used to indicate that some buttons have been pressed
		}
	}

	gameNetworkObj->drawMutex.lock(); //called from main thread, don't want access violations so use mutex
	gameWindow->setView(*gameView);
	for (auto& x : backgroundRectanglesToDraw) {
		gameWindow->draw(x);
	}
	for (auto& x : circlesToDraw) {
		gameWindow->draw(x);
	}
	for (auto& x : rectanglesToDraw) {
		gameWindow->draw(x);
	}
	for (auto& x : textToDraw) {
		gameWindow->draw(x);
	}
	for (auto& x : spritesToDraw) {
		gameWindow->draw(x);
	}
	for (auto& x : spriteAccessories) {
		gameWindow->draw(x);
	}
	gameNetworkObj->drawMutex.unlock();
}

/*

Basically, you need to send the keys to the server, then the server will move the player appropriately. We'll need to use the concept of velocity as we only get press
and release events.
That helper function [mapKeys(...)] just maps SFML keys to the ones I'm storing in the database.
We will send those through UDP.


*/