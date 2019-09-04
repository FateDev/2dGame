#include <gui.h>
#include <header.h>

socialTabClass::socialTabClass(tgui::Gui &gui, networking* networkObject) : window(window), networkObject(networkObject) {
	float percentWidth = (float(sf::VideoMode::getDesktopMode().width - 400) / sf::VideoMode::getDesktopMode().width) * 100;
	float percentX = (float)(400.0f / (float)sf::VideoMode::getDesktopMode().width) * 100;

	chatBox = new chat(percentWidth, 70, percentX, 30); //setting temp values as also need to use absolute positioning
	chatBox->chatBoxContainer->setPosition(std::to_string(percentX) + "%", sf::VideoMode::getDesktopMode().height - chatBox->chatBoxContainer->getFullSize().y - 30);
	chatBox->chatBoxContainer->setPositionLocked(true); //so that it can't move

	roomGuildSelectBox = tgui::ScrollablePanel::create({ std::to_string(100 - percentWidth) + "%", chatBox->chatBoxContainer->getFullSize().y+1 });
	roomGuildSelectBox->setPosition(0, chatBox->chatBoxContainer->getPosition().y-1);
	roomGuildSelectBox->setVerticalScrollbarPolicy(tgui::Scrollbar::Policy::Always);
	roomGuildSelectBox->setHorizontalScrollbarPolicy(tgui::Scrollbar::Policy::Never);
	//this will make the above occupy the left of the screen and have the same height as the chat box

	socialTabGroup->add(chatBox->chatBoxContainer);
	socialTabGroup->add(roomGuildSelectBox);
	this->setActive(false);

	gui.add(socialTabGroup);
}

void socialTabClass::liveUpdate(sf::Clock* globalClock) {
	chatBox->liveUpdate(networkObject, globalClock); //calls the live update function for the chat box, so basically allows for sending messages
}

void socialTabClass::setActive(bool active) { //by setting the visibility through this, groups are really easy to manage so multiple screens are easy to manage
	if (active == true) {
		//for the above, when the input parameter is true, it makes the group visible and sets the active boolean in the object as
		//true (used to decide whether or not to have the live update function be active or not in the main game loop)
		this->active = true;
		socialTabGroup->setVisible(true);

		//these three functions below will remove the contents of the chatbox, then get the new messages from the database, and then add those to the chatbox
		networkObject->getMessagesFromDB();
		chatBoxBulkAdd(this->networkObject, chatBox);

		populateRoomGuildSelectBox(); //this will add all the room guild select buttons and stuff

		//the below 2 lines will make it so that the current chat box is the one to send messages to
		networkObject->chatBoxObject = chatBox;
	}
	else {
		//sets the active variable false and makes the main screen group invisible so they won't be drawn
		this->active = false;
		socialTabGroup->setVisible(false);

		roomGuildSelectBox->removeAllWidgets();
		chatBox->flushMessages();

		currentMaxHeightRoomGuildSelect = 0; //this will reset the max height so that it doesn't start from this offset the next time this stuff is displayed
	}
}

void socialTabClass::addButtonToPanel(tgui::ScrollablePanel::Ptr panel, std::string text, float percentWidth) {
	auto button = tgui::Button::create(text);
	button->setSize({ "100%", button->getFullSize().y + 30 });

	//sets it's y position to be at the maximum current y position, so at the bottom of the chat box scrollable panel
	button->setPosition(0, currentMaxHeightRoomGuildSelect);

	currentMaxHeightRoomGuildSelect += button->getFullSize().y; //updates the maximum y pos for the next message (assuming there is one)

	button->connect("Clicked", &socialTabClass::changeRoomGuild, this, button->getText());

	panel->add(button);
}

void socialTabClass::populateRoomGuildSelectBox() {
	CURL* curl = curl_easy_init(); //we can set options for this to make it control how a transfer/transfers will be made
	std::string readBuffer; //string for the returning data

	curl_easy_setopt(curl, CURLOPT_URL, "http://erewhon.xyz/game/roomGuild.php");
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); //the callback
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); //will write data to the string, so the fourth param of the last callback is stored here

	curl_easy_perform(curl);
	delete curl;

	roomGuildList = json::parse(readBuffer);

	float percentWidthRoomGuildSelectBox = (roomGuildSelectBox->getFullSize().y / sf::VideoMode::getDesktopMode().width) * 100;

	for (int i = 0; i < roomGuildList.size(); i++) {
		addButtonToPanel(roomGuildSelectBox, roomGuildList[i]["name"].get<std::string>(), percentWidthRoomGuildSelectBox);
	}
}

void socialTabClass::changeRoomGuild(std::string buttonText) {
	chatBox->flushMessages();

	networkObject->roomGuild = buttonText; //the button text is the room guild name in this case, which we can use to request whatever data we need

	networkObject->getMessagesFromDB();
	chatBoxBulkAdd(this->networkObject, chatBox);
}