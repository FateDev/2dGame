#include <game.h>

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
	sfKeyToAbstractKeyMap[sf::Keyboard::Num0] = "Num0"; abstractKeyTosfKeyMap["Num0"] = sf::Keyboard::Num0;
	sfKeyToAbstractKeyMap[sf::Keyboard::Num1] = "Num1"; abstractKeyTosfKeyMap["Num1"] = sf::Keyboard::Num1;
	sfKeyToAbstractKeyMap[sf::Keyboard::Num2] = "Num2"; abstractKeyTosfKeyMap["Num2"] = sf::Keyboard::Num2;
	sfKeyToAbstractKeyMap[sf::Keyboard::Num3] = "Num3"; abstractKeyTosfKeyMap["Num3"] = sf::Keyboard::Num3;
	sfKeyToAbstractKeyMap[sf::Keyboard::Num4] = "Num4"; abstractKeyTosfKeyMap["Num4"] = sf::Keyboard::Num4;
	sfKeyToAbstractKeyMap[sf::Keyboard::Num5] = "Num5"; abstractKeyTosfKeyMap["Num5"] = sf::Keyboard::Num5;
	sfKeyToAbstractKeyMap[sf::Keyboard::Num6] = "Num6"; abstractKeyTosfKeyMap["Num6"] = sf::Keyboard::Num6;
	sfKeyToAbstractKeyMap[sf::Keyboard::Num7] = "Num7"; abstractKeyTosfKeyMap["Num7"] = sf::Keyboard::Num7;
	sfKeyToAbstractKeyMap[sf::Keyboard::Num8] = "Num8"; abstractKeyTosfKeyMap["Num8"] = sf::Keyboard::Num8;
	sfKeyToAbstractKeyMap[sf::Keyboard::Num9] = "Num9"; abstractKeyTosfKeyMap["Num9"] = sf::Keyboard::Num9;
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
	keysObject["custom_binding1"] = false;
	keysObject["custom_binding2"] = false;
	keysObject["custom_binding3"] = false;

	keysObject["sessionID"] = networkObj->sessionID; //adds the session ID for the basic auth
}

void game::listenForKeys(sf::Event event) {
	if (event.type == sf::Event::KeyPressed) { //keysObject[key] = true/false; true = key is pressed, false = key is not pressed/released
		std::string key = sfKeyToAbstractKeyMap[event.key.code];
		if (key==networkObj->settings["left"] || key == networkObj->settings["right"] || key == networkObj->settings["jump"] || key == networkObj->settings["interactNPC"] || key == networkObj->settings["switchTarget"] || key == networkObj->settings["custom_binding1"]["key"] || key == networkObj->settings["custom_binding2"]["key"] || key == networkObj->settings["custom_binding3"]["key"]) {
			if (key == networkObj->settings["left"] && keysObject["left"] != true) {
				keysObject["left"] = true;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["right"] && keysObject["right"] != true) {
				keysObject["right"] = true;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["jump"] && keysObject["jump"] != true) {
				keysObject["jump"] = true;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["interactNPC"] && keysObject["interactNPC"] != true) {
				keysObject["interactNPC"] = true;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["switchTarget"] && keysObject["switchTarget"] != true) {
				keysObject["switchTarget"] = true;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["custom_binding1"] && keysObject["custom_binding1"] != true) {
				keysObject["custom_binding1"] = true;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["custom_binding2"] && keysObject["custom_binding2"] != true) {
				keysObject["custom_binding2"] = true;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["custom_binding3"] && keysObject["custom_binding3"] != true) {
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
		if (key == networkObj->settings["left"] || key == networkObj->settings["right"] || key == networkObj->settings["jump"] || key == networkObj->settings["custom_binding1"]["key"] || key == networkObj->settings["custom_binding2"]["key"] || key == networkObj->settings["custom_binding3"]["key"]) {
			if (key == networkObj->settings["left"] && keysObject["left"] != false) {
				keysObject["left"] = false;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["right"] && keysObject["right"] != false) {
				keysObject["right"] = false;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["jump"] && keysObject["jump"] != false) {
				keysObject["jump"] = false;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["interactNPC"] && keysObject["interactNPC"] != false) {
				keysObject["interactNPC"] = false;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["switchTarget"] && keysObject["switchTarget"] != false) {
				keysObject["switchTarget"] = false;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["custom_binding1"] && keysObject["custom_binding1"] != false) {
				keysObject["custom_binding1"] = false;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["custom_binding2"] && keysObject["custom_binding2"] != false) {
				keysObject["custom_binding2"] = false;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}else if (key == networkObj->settings["custom_binding3"] && keysObject["custom_binding3"] != false) {
				keysObject["custom_binding3"] = false;
				changeInButtonState = true; //used to indicate that some buttons have been pressed
			}
		}
	}
}

void game::draw() {
	gameNetworkObj->jsonMutex.lock(); //will lock the gameData resource
	//std::cout << "locked resource" << std::endl;

	if (!gameData.is_null()) {	//it now moves it
		sf::CircleShape circleThing(60, 60);
		circleThing.setOrigin(-60, 60);
		sf::RectangleShape rectangle(sf::Vector2f(gameWindow->getSize().x, gameWindow->getSize().y / 2));
		rectangle.setPosition(sf::Vector2f(-int(gameWindow->getSize().x) / 2, 0));
		gameWindow->draw(circleThing);
		gameWindow->draw(rectangle);
		for (int j = 0; j < 8; j++) {
			json chunkToDraw = json::parse(gameData["chunks"][j].get<std::string>());
			//std::cout << chunkToDraw.dump() << std::endl;
			if (chunkToDraw.dump() != "null") {
				//std::cout << chunkToDraw.dump() << std::endl;
				for (int i = 0; i < chunkToDraw["entityCount"]; i++) {
				
					//std::cout << gameNetworkObj->unixMicrosecondsOfLastPacket << ": " << chunkToDraw["entities"][0]["location"]["x"].get<float>() << " -- " << chunkToDraw["entities"][0]["location"]["y"].get<float>() << std::endl;

					if (networkObj->userID == chunkToDraw["entities"][i]["id"].get<int>()) {
						//std::cout << "x: " << chunkToDraw["entities"][i]["location"]["x"].get<float>() * 10 << " -- y: " << chunkToDraw["entities"][i]["location"]["y"].get<float>() * 10 << std::endl;
						gameView->setCenter(chunkToDraw["entities"][i]["location"]["x"].get<float>() * 10, chunkToDraw["entities"][i]["location"]["y"].get<float>() * 10);
						sf::RectangleShape player(sf::Vector2f(40, 40));
						player.setOrigin(-20, 40);
						player.setFillColor(sf::Color::Blue);
						player.setPosition(chunkToDraw["entities"][i]["location"]["x"].get<float>() * 10, chunkToDraw["entities"][i]["location"]["y"].get<float>() * 10);

						gameWindow->draw(player);
					}
					else {
						sf::RectangleShape opponent(sf::Vector2f(40, 40));
						opponent.setOrigin(-20, 40);
						opponent.setFillColor(sf::Color::Red);
						opponent.setPosition(chunkToDraw["entities"][i]["location"]["x"].get<float>() * 10, chunkToDraw["entities"][i]["location"]["y"].get<float>() * 10);

						gameWindow->draw(opponent);
					}
				}
			}
		}
	}

	gameNetworkObj->jsonMutex.unlock(); //will free the resource
}

void game::live() {
	if (changeInButtonState && !networkObj->msgBoxFocused) {
		gameNetworkObj->sendData(keysObject); //sends the object to the server

		//we're resetting some of them here, as they just do a single action, rather than move or whatever
		keysObject["interactNPC"] = false;
		keysObject["switchTarget"] = false;

		changeInButtonState = false; //reset this so that we don't repeatedly send the same data to the server
	}
	draw(); //will draw the actual game
}

/*

Basically, you need to send the keys to the server, then the server will move the player appropriately. We'll need to use the concept of velocity as we only get press
and release events.
That helper function [mapKeys(...)] just maps SFML keys to the ones I'm storing in the database.
We will send those through UDP.


*/