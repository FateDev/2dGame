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
			unsigned int cantorPairedValues = 0.5 * (i+j) * (i+j+1) + j;
			guiInventoryButtons[i][j]->setText(std::to_string(cantorPairedValues)); //uses cantor pairing function to make i and j into 1 number
			guiInventoryButtons[i][j]->setTextSize(1);
			guiInventoryButtons[i][j]->setSize("10%", "15%");
			guiInventoryButtons[i][j]->setPosition(std::to_string(20 + 10 * j) + "%", std::to_string(3 + 15 * i) + "%");
			guiInventoryButtons[i][j]->connect("pressed", &inventory::inventoryItemClickRegister, this, guiInventoryButtons[i][j]->getText());

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

void inventory::drawInventoryItems() { //will draw items, where the number (ID) of the item corresponds directly to the texture
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			int item = inventoryJSON[i][j].get<int>();
			//std::cout << "resources/items/" + std::to_string(item) + ".jpg" << "\n";
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

void inventory::InventoryLive() { //makes the drag and drop item follow the user's cursor
	if (dragDropActive) {
		dragDropItem.setPosition(sf::Vector2f(sf::Mouse::getPosition().x-50, sf::Mouse::getPosition().y-50)); //items are 100x100 so this kinda centers them around the cursor
		window->draw(dragDropItem);
	}

}

void inventory::inventoryItemClickRegister(std::string buttonText) {
	//the number stored in the text there is transparent so invisible, and the number is the ith row and jth column of the 2d button grid representing items, paired using the cantor pairing function
	//so we invert the pairing below and get the ith row and jth column back
	float z = std::stoi(buttonText);
	float w = floor(0.5 * (sqrt(8 * z + 1) - 1));
	float t = 0.5 * (pow(w, 2) + w);
	int j = z - t;
	int i = w - j;

	dragDropItemTexture = sf::Texture(); //resets the texture
	dragDropItem = sf::RectangleShape(sf::Vector2f(100, 100)); //resets the drag and drop item

	int item = inventoryJSON[i][j].get<int>(); //gets the item in the box that was just clicked on
	if (item != 0 && dragDropActive != true) { //if the item isn't 0
		dragDropActive = true; //commence a drag and drop operation
		dragDropItemTexture.loadFromFile("resources/items/" + std::to_string(item) + ".png"); //set the drag and drop texture's texture as the one of this box
		dragDropItem.setTexture(&dragDropItemTexture); //set the item's texture as the above
		dragDropItemIndexes = sf::Vector2i(i, j); //the origin of the drag and drop operation is from this box's index
		guiInventoryButtons[i][j]->setImage(NULL); //and delete the image in this box
		if (i == 0) { //if it's the first row...
			smallInventoryButtons[j]->setImage(NULL); //...then replicate this change in the toolbar inventory thing
		}
	}
	else if(dragDropActive == true){
		if (item == 0 || (dragDropItemIndexes.x == i && dragDropItemIndexes.y == j)) { //if there is no item there or if we're trying to put an item back where it was
			int item = inventoryJSON[dragDropItemIndexes.x][dragDropItemIndexes.y].get<int>(); //get the item at the origin of the operation
			inventoryJSON[dragDropItemIndexes.x][dragDropItemIndexes.y] = 0; //set the item at the origin as 0 now, as we're moving it
			inventoryJSON[i][j] = item; //set the current box's item as the one which was at the origin (so it's moved now)
			guiInventoryButtons[i][j]->setImage("resources/items/" + std::to_string(item) + ".png"); //set the image in the GUI
			guiInventoryButtons[i][j]->setImageScaling(0.7); //scale the image
			dragDropActive = false; //the drag and drop operation is complete
			if (i == 0) { //if it's the first row...
				smallInventoryButtons[j]->setImage("resources/items/" + std::to_string(item) + ".png"); //...then replicate this change in the toolbar inventory thing
			}
		}
		else if (item != 0) { //if the item in the current box isn't 0 and the drag and drop operation is active, then we do a swap operation
			int itemDragDrop = inventoryJSON[dragDropItemIndexes.x][dragDropItemIndexes.y].get<int>(); //we get the item at the origin of the drag and drop operation
			inventoryJSON[dragDropItemIndexes.x][dragDropItemIndexes.y] = item; //we set the item at the origin as the item we just clicked on (so a swap has basically occurred)
			inventoryJSON[i][j] = itemDragDrop; //and set the item of this box to be the one which was at the origin (finishes up the swap operation - the non graphical bit at least)
			dragDropItemTexture.loadFromFile("resources/items/" + std::to_string(item) + ".png"); //sets the new texture of the drag and drop texture
			dragDropItem.setTexture(&dragDropItemTexture); //sets the above texture for the drag and drop item
			guiInventoryButtons[i][j]->setImage("resources/items/" + std::to_string(itemDragDrop) + ".png"); //sets the texture for the box we clicked on as the image which was originally at the origin of the drag and drop operation
			if (i == 0) { //if it's the first row...
				smallInventoryButtons[j]->setImage("resources/items/" + std::to_string(itemDragDrop) + ".png"); //...then replicate this change in the toolbar inventory thing
			}
		}
		else {
			int item = inventoryJSON[dragDropItemIndexes.x][dragDropItemIndexes.y].get<int>(); //get the item at the origin of the operation
			dragDropItemTexture.loadFromFile("resources/items/" + std::to_string(item) + ".png"); //set the drag and drop texture's texture as the one of this box
			dragDropItem.setTexture(&dragDropItemTexture); //set the item's texture as the above
		}
	}
}