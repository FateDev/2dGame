#include <header.h>
#include <gui.h>

mainScreen::mainScreen(tgui::Gui &gui, networking* networkObject) : window(window), networkObject(networkObject) {

	/////////////////////////////
	////////ADD GROUP STUFF

	//makes a chat object and then adds it to the main screen group
	chatBox = new chat(25, 50, 2, 10);

	chatBoxBulkAdd(this->networkObject, chatBox); //will add messages to the chat box
	mainScreenGroup->add(chatBox->chatBoxContainer);

	//gives the network object the chatBox object, and sets the bool indicating whether the chatBox is active to true
	networkObject->chatBoxObject = chatBox;
	networkObject->chatBoxActive = true;

	//adds the main screen group to the gui and makes sure that it is invisible
	gui.add(mainScreenGroup);
	mainScreenGroup->setVisible(false);
}

void mainScreen::liveUpdate(sf::Clock* globalClock){
	chatBox->liveUpdate(networkObject, globalClock); //calls the live update function for the chat box, so basically allows for sending messages
}

void mainScreen::setActive(bool active) { //by setting the visibility through this, groups are really easy to manage so multiple screens are easy to manage
	if (active == true) { 
		//for the above, when the input parameter is true, it makes the group visible and sets the active boolean in the object as 
		//true (used to decide whether or not to have the live update function be active or not in the main game loop)
		this->active = true;
		mainScreenGroup->setVisible(true);

		//these three functions below will remove the contents of the chatbox, then get the new messages from the database, and then add those to the chatbox
		chatBox->flushMessages();
		networkObject->getMessagesFromDB();
		chatBoxBulkAdd(networkObject, chatBox);

		//the below 2 lines will make it so that the current chat box is the one to send messages to
		networkObject->chatBoxObject = chatBox;
	}
	else {
		//sets the active variable false and makes the main screen group invisible so they won't be drawn
		this->active = false;
		mainScreenGroup->setVisible(false); 
	}
}