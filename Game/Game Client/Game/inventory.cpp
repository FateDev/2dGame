#include <gui.h>
#include <header.h>

inventory::inventory(tgui::Gui& gui, sf::RenderWindow* window, game* gameObj) : window(window), gameObj(gameObj) {
	theme = tgui::Theme("Game.txt");
	//the above loads the colours and stuff for the widgets from the file "Game.txt"

	//below makes the inventory items buttons
	for (int i = 0; i < 7; i++) {
		smallInventoryButtons.push_back(tgui::BitmapButton::create());
		smallInventoryButtons[i]->setRenderer(theme.getRenderer("Button2"));
		smallInventoryButtons[i]->setSize("5%", "8%");
		smallInventoryButtons[i]->setPosition(std::to_string(65 + 5 * i) + "%", 0);
		smallInventoryButtons[i]->connect("pressed", [&, i]() { setCurrentSelected(i); updateSelectedItem(i); drawInventoryItems(); }); //clicking the inventory button would set the current item to that, then update it on the server side, and draw the updated stuff

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
		for (int i = 1; i <= 6; i++) {
			if (gameObj->networkObj->settings["item" + std::to_string(i)].get<std::string>() == key && !gameObj->networkObj->msgBoxFocused) {
				updateSelectedItem(i-1); //doing i-1 as the numbers are from 1 to 6, but the item index is from 0 to 5
				currentSelectedItemIndex = i - 1; //same as above but for internal drawing of the selected box
				drawInventoryItems();
			}
		}

		if (gameObj->networkObj->settings["inventory"].get<std::string>() == key && !gameObj->networkObj->msgBoxFocused) { //did they press the key to open or close the inventory, as long as the message box isn't focused
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
			guiInventoryButtons[i][j]->setImage(gameObj->itemsFromFile[item]["resourceLocation"].get<std::string>());
			guiInventoryButtons[i][j]->setImageScaling(0.7);

			if (i == 0) {
				if (j == currentSelectedItemIndex) {
					smallInventoryButtons[j]->setRenderer(theme.getRenderer("ExpandedInventoryButton"));
					smallInventoryButtons[j]->setImage(gameObj->itemsFromFile[item]["resourceLocation"].get<std::string>());
					smallInventoryButtons[j]->setImageScaling(0.7);
					guiInventoryButtons[i][j]->setRenderer(theme.getRenderer("ExpandedInventoryButton"));
					guiInventoryButtons[i][j]->setImage(gameObj->itemsFromFile[item]["resourceLocation"].get<std::string>());
					guiInventoryButtons[i][j]->setImageScaling(0.7);
				}
				else {
					smallInventoryButtons[j]->setRenderer(theme.getRenderer("InventoryToolbarButton"));
					smallInventoryButtons[j]->setImage(gameObj->itemsFromFile[item]["resourceLocation"].get<std::string>());
					smallInventoryButtons[j]->setImageScaling(0.7);
					guiInventoryButtons[i][j]->setRenderer(theme.getRenderer("InventoryToolbarButton"));
					guiInventoryButtons[i][j]->setImage(gameObj->itemsFromFile[item]["resourceLocation"].get<std::string>());
					guiInventoryButtons[i][j]->setImageScaling(0.7);
				}
			}
		}
	}
}

void inventory::InventoryLive(sf::View *gameView) { //makes the drag and drop item follow the user's cursor
	if (dragDropActive) {
		dragDropItem.setPosition(sf::Vector2f(sf::Mouse::getPosition().x-50, sf::Mouse::getPosition().y-50)); //items are 100x100 so this kinda centers them around the cursor
		window->setView(window->getDefaultView());
		window->draw(dragDropItem);
	}
	/*
	if (currentUserItemChanged) {
		if (lastClickedOnBox.x == 0) { //I'm using i and j rather than x and y, and it turns out that x is i, which is the row number
			if (lastClickedOnBox.y == currentUserSelectionSmallInventory) {
				userItem = sf::RectangleShape(sf::Vector2f(100, 100));
				userItemTexture = sf::Texture();
				int item = inventoryJSON[lastClickedOnBox.x][lastClickedOnBox.y].get<int>(); //get the item which is now in the user's current selection
				userItemTexture.loadFromFile("resources/items/" + std::to_string(item) + ".png");
				userItem.setTexture(&userItemTexture);
				currentUserItemChanged = false; //reset the flag
			}
		}
	}

	if (!isInventoryOpen()) { //only draw the item if the inventory isn't open
		long x = gameObj->currentPosition_x;
		long y = gameObj->currentPosition_y;
		userItem.setPosition(x, y);
		window->setView(*gameView);
		window->draw(userItem);
	}
	*/
}

void inventory::inventoryItemClickRegister(std::string buttonText) {
	//the number stored in the text there is transparent so invisible, and the number is the ith row and jth column of the 2d button grid representing items, paired using the cantor pairing function
	//so we invert the pairing below and get the ith row and jth column back
	long i = 0;
	long j = 0;
	float z = std::stoi(buttonText);
	inverseCantorPairingFunction(z, &i, &j);

	dragDropItemTexture = sf::Texture(); //resets the texture
	dragDropItem = sf::RectangleShape(sf::Vector2f(100, 100)); //resets the drag and drop item
	//gameObj->itemsFromFile[item]["resourceLocation"].get<std::string>()
	int item = inventoryJSON[i][j].get<int>(); //gets the item in the box that was just clicked on
	if (item != 0 && dragDropActive != true) { //if the item isn't 0
		dragDropActive = true; //commence a drag and drop operation
		dragDropItemTexture.loadFromFile(gameObj->itemsFromFile[item]["resourceLocation"].get<std::string>()); //set the drag and drop texture's texture as the one of this box
		dragDropItem.setTexture(&dragDropItemTexture); //set the item's texture as the above
		dragDropItemIndexes = sf::Vector2i(i, j); //the origin of the drag and drop operation is from this box's index
		guiInventoryButtons[i][j]->setImage(NULL); //and delete the image in this box
		if (i == 0) { //if it's the first row...
			smallInventoryButtons[j]->setImage(NULL); //...then replicate this change in the toolbar inventory thing
		}
		updateServerSide();
		/*
		lastClickedOnBox = sf::Vector2i(i, j); //the last box which was clicked on (needed to draw the current item)
		currentUserItemChanged = true; //the user's current item may have changed*/
	}
	else if(dragDropActive == true){
		if (item == 0 || (dragDropItemIndexes.x == i && dragDropItemIndexes.y == j)) { //if there is no item there or if we're trying to put an item back where it was
			int item = inventoryJSON[dragDropItemIndexes.x][dragDropItemIndexes.y].get<int>(); //get the item at the origin of the operation
			inventoryJSON[dragDropItemIndexes.x][dragDropItemIndexes.y] = 0; //set the item at the origin as 0 now, as we're moving it
			inventoryJSON[i][j] = item; //set the current box's item as the one which was at the origin (so it's moved now)
			guiInventoryButtons[i][j]->setImage(gameObj->itemsFromFile[item]["resourceLocation"].get<std::string>()); //set the image in the GUI
			guiInventoryButtons[i][j]->setImageScaling(0.7); //scale the image
			dragDropActive = false; //the drag and drop operation is complete
			if (i == 0) { //if it's the first row...
				smallInventoryButtons[j]->setImage(gameObj->itemsFromFile[item]["resourceLocation"].get<std::string>()); //...then replicate this change in the toolbar inventory thing
			}
			updateServerSide();
			/*currentUserItemChanged = true; //the user's current item may have changed*/
		}
		else if (item != 0) { //if the item in the current box isn't 0 and the drag and drop operation is active, then we do a swap operation
			int itemDragDrop = inventoryJSON[dragDropItemIndexes.x][dragDropItemIndexes.y].get<int>(); //we get the item at the origin of the drag and drop operation
			inventoryJSON[dragDropItemIndexes.x][dragDropItemIndexes.y] = item; //we set the item at the origin as the item we just clicked on (so a swap has basically occurred)
			inventoryJSON[i][j] = itemDragDrop; //and set the item of this box to be the one which was at the origin (finishes up the swap operation - the non graphical bit at least)
			dragDropItemTexture.loadFromFile(gameObj->itemsFromFile[item]["resourceLocation"].get<std::string>()); //sets the new texture of the drag and drop texture
			dragDropItem.setTexture(&dragDropItemTexture); //sets the above texture for the drag and drop item
			guiInventoryButtons[i][j]->setImage(gameObj->itemsFromFile[itemDragDrop]["resourceLocation"].get<std::string>()); //sets the texture for the box we clicked on as the image which was originally at the origin of the drag and drop operation
			if (i == 0) { //if it's the first row...
				smallInventoryButtons[j]->setImage(gameObj->itemsFromFile[itemDragDrop]["resourceLocation"].get<std::string>()); //...then replicate this change in the toolbar inventory thing
			}
			updateServerSide();
			/*currentUserItemChanged = true; //the user's current item may have changed*/
		}
		else { //catch all that sets the texture of the drag and drop item as that of the item which was at the operation's origin
			int item = inventoryJSON[dragDropItemIndexes.x][dragDropItemIndexes.y].get<int>(); //get the item at the origin of the operation
			dragDropItemTexture.loadFromFile(gameObj->itemsFromFile[item]["resourceLocation"].get<std::string>()); //set the drag and drop texture's texture as the one of this box
			dragDropItem.setTexture(&dragDropItemTexture); //set the item's texture as the above
		}
		/*lastClickedOnBox = sf::Vector2i(i, j); //the last box which was clicked on (needed to draw the current item)*/
	}
}

void inventory::updateServerSide() {
	std::string inventoryJSONString = "UPDATE::INVENTORY::" + inventoryJSON.dump();
	sf::Packet sendPacket;
	sendPacket << inventoryJSONString;
	gameObj->networkObj->socket->send(sendPacket); //will send the current state of the inventory
}

void inventory::updateSelectedItem(int newSelected) {
	std::string inventoryJSONString = "UPDATE::SELECTED_ITEM::" + std::to_string(newSelected);
	sf::Packet sendPacket;
	sendPacket << inventoryJSONString;
	gameObj->networkObj->socket->send(sendPacket); //will send the current state of the inventory
}