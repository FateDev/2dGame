#include <gui.h>
#include <header.h>

socialTabClass::socialTabClass(tgui::Gui &gui, networking* networkObject) : window(window), networkObject(networkObject) {
	int percentWidth = (float(sf::VideoMode::getDesktopMode().width - 400) / sf::VideoMode::getDesktopMode().width) * 100;
	int percentX = (400.0f / sf::VideoMode::getDesktopMode().width) * 100;

	chatBox = new chat(percentWidth, 70, percentX, 30); //setting temp values as also need to use absolute positioning
	chatBox->chatBoxContainer->setPosition(std::to_string(percentX) + "%", sf::VideoMode::getDesktopMode().height - chatBox->chatBoxContainer->getFullSize().y - 30);
	chatBox->chatBoxContainer->setPositionLocked(true); //so that it can't move

	socialTabGroup->add(chatBox->chatBoxContainer);
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
		chatBox->flushMessages();
		networkObject->getMessagesFromDB();
		chatBoxBulkAdd(this->networkObject, chatBox);

		//the below 2 lines will make it so that the current chat box is the one to send messages to
		networkObject->chatBoxObject = chatBox;
	}
	else {
		//sets the active variable false and makes the main screen group invisible so they won't be drawn
		this->active = false;
		socialTabGroup->setVisible(false);


		chatBox->flushMessages();
	}
}