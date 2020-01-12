#include <gui.h>
#include <game.h>

inventory::inventory(tgui::Gui& gui, sf::RenderWindow* window, game* gameObj) : window(window), gameObj(gameObj) {
	tgui::Theme theme("Game.txt");
	//the above loads the colours and stuff for the widgets from the file "Game.txt"

	//below makes the inventory items buttons
	for (int i = 0; i < 7; i++) {
		smallInventoryButtons.push_back(tgui::BitmapButton::create());
		smallInventoryButtons[i]->setRenderer(theme.getRenderer("Button2"));
		smallInventoryButtons[i]->setSize("5%", "8%");
		smallInventoryButtons[i]->setPosition(std::to_string(65 + 5 * i) + "%", 0);

		inventoryBar->add(smallInventoryButtons[i]); //adds current one to the main group
	}

	guiInventoryButtons.resize(7, std::vector<tgui::BitmapButton::Ptr>(8)); //correctly sizing the vector to prevent uninitialised errors

	//this is the temporary exit button for the main gui

	guiInventoryButtons[0][7] = tgui::BitmapButton::create();
	guiInventoryButtons[0][7]->setRenderer(theme.getRenderer("InventoryToolbarButton"));
	guiInventoryButtons[0][7]->setSize("8%", "10%");
	guiInventoryButtons[0][7]->setPosition("92%", 0);
	guiInventoryButtons[0][7]->setImage("resources/exit.png");
	guiInventoryButtons[0][7]->setImageScaling(0.7);

	inventoryGUIBackground->setRenderer(theme.getRenderer("InventoryGUI"));
	inventoryBox->add(inventoryGUIBackground);

	inventoryBox->add(guiInventoryButtons[0][7]);

	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			guiInventoryButtons[i][j] = tgui::BitmapButton::create();
			if (i == 0) {
				guiInventoryButtons[i][j]->setRenderer(theme.getRenderer("InventoryToolbarButton"));
			}
			else {
				guiInventoryButtons[i][j]->setRenderer(theme.getRenderer("ExpandedInventoryButton"));
			}
			guiInventoryButtons[i][j]->setSize("10%", "15%");
			guiInventoryButtons[i][j]->setPosition(std::to_string(20 + 10 * j) + "%", std::to_string(3 + 15 * i) + "%");

			inventoryBox->add(guiInventoryButtons[i][j]); //adds current one to the main group
		}
	}

	smallInventoryButtons[0]->setRenderer(theme.getRenderer("Button3"));
	smallInventoryButtons[6]->setRenderer(theme.getRenderer("Button4"));
	smallInventoryButtons[6]->setImage("resources/more.png");
	smallInventoryButtons[6]->setImageScaling(0.7);
	//the above set some special cases which aren't possible through the loops

	smallInventoryButtons[6]->connect("pressed", &inventory::openInventory, this);
	guiInventoryButtons[0][7]->connect("pressed", &inventory::closeInventory, this); //exit button

	gui.add(inventoryBar);
	gui.add(inventoryBox);

	inventoryBox->setVisible(false);
	inventoryBar->setVisible(false); //initially set this to false, then set it to true after the entire loading screen bit
}

void inventory::displayToolbar() {
	inventoryBar->setVisible(true);
}

void inventory::openInventory() {
	isOpen = true; //the inventory is open
	inventoryBox->setVisible(true);
	inventoryBar->setVisible(false);
}

void inventory::closeInventory() {
	isOpen = false; //the inventory is closed
	inventoryBox->setVisible(false);
	inventoryBar->setVisible(true);
}

void inventory::listenForKeys(sf::Event event) {
	if (event.type == sf::Event::KeyPressed) {
		std::string key = gameObj->sfKeyToAbstractKeyMap[event.key.code];
		if (gameObj->networkObj->settings["inventory"].get<std::string>() == key) { //did they press the key to open or close the inventory
			if (this->isInventoryOpen()) {
				this->closeInventory();
			}
			else {
				this->openInventory();
			}
		}
	}
}

void inventory::drawToolbarInventoryItems() { //will draw items, where the number (ID) of the item corresponds directly to the texture
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			int item = inventoryJSON[i][j].get<int>();
			std::cout << "resources/items/" + std::to_string(item) + ".jpg" << "\n";
			if (item != 0) {

				guiInventoryButtons[i][j]->setImage("resources/items/" + std::to_string(item) + ".png");
				guiInventoryButtons[i][j]->setImageScaling(0.7);
			}
			if (i == 0) {
				if (item != 0) {
					smallInventoryButtons[j]->setImage("resources/items/" + std::to_string(item) + ".png");
					smallInventoryButtons[j]->setImageScaling(0.7);

				}
			}
		}
	}
}

void inventory::drawGUIInventoryItems() {

}