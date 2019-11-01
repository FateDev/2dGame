#include <game.h>

game::game(networking* networkObject, gameNetwork* gameConnection) : networkObj(networkObject), gameNetworkObj(gameConnection) { //I added in a map the opposite way round just in case
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
	std::string switchTargetKey = networkObj->settings["switchTarget"];
	std::string interactNPCKey = networkObj->settings["interactNPC"];
	std::string jumpKey = networkObj->settings["jump"];
	std::string leftKey = networkObj->settings["left"];
	std::string rightKey = networkObj->settings["right"];
	std::string custom1Key = networkObj->settings["custom_binding1"]["key"];
	std::string custom2Key = networkObj->settings["custom_binding2"]["key"];
	std::string custom3Key = networkObj->settings["custom_binding3"]["key"];
	keysObject[switchTargetKey] = false;
	keysObject[interactNPCKey] = false;
	keysObject[jumpKey] = false;
	keysObject[leftKey] = false;
	keysObject[rightKey] = false;
	keysObject[custom1Key] = false;
	keysObject[custom2Key] = false;
	keysObject[custom3Key] = false;
}

void game::listenForKeys(sf::Event event) {
	std::string key = sfKeyToAbstractKeyMap[event.key.code];

	if (event.type == sf::Event::KeyPressed) { //keysObject[key] = true/false; true = key is pressed, false = key is not pressed/released
		if (key==networkObj->settings["left"] || key == networkObj->settings["right"] || key == networkObj->settings["jump"] || key == networkObj->settings["interactNPC"] || key == networkObj->settings["switchTarget"] || key == networkObj->settings["custom_binding1"]["key"] || key == networkObj->settings["custom_binding2"]["key"] || key == networkObj->settings["custom_binding3"]["key"]) {
			keysObject[key] = true;
		}
		else { //the ones above are the keys to tell the server about, the ones below should deal with stuff like opening inventory and other stuff like that

		}
	}

	if (event.type == sf::Event::KeyReleased) { //this would be used to stop moving for example
		if (key == networkObj->settings["left"] || key == networkObj->settings["right"] || key == networkObj->settings["jump"] || key == networkObj->settings["custom_binding1"]["key"] || key == networkObj->settings["custom_binding2"]["key"] || key == networkObj->settings["custom_binding3"]["key"]) {
			keysObject[key] = false;
		}
	}
}

void game::live() {
	keysObject["sessionID"] = networkObj->sessionID; //adds the session ID for the basic auth
	gameNetworkObj->sendData(keysObject); //sends the object to the server

	//we're resetting some of them here, as they just do a single action, rather than move or whatever
	std::string switchTargetKey = networkObj->settings["switchTarget"];
	std::string interactNPCKey = networkObj->settings["interactNPC"];
	keysObject[switchTargetKey] = false;
	keysObject[interactNPCKey] = false;
}

/*

Basically, you need to send the keys to the server, then the server will move the player appropriately. We'll need to use the concept of velocity as we only get press
and release events.
That helper function [mapKeys(...)] just maps SFML keys to the ones I'm storing in the database.
We will send those through UDP.


*/