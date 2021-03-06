#include <future>
#include <gui.h>
#include <header.h>
#include "key.h"

socialTabClass::socialTabClass(tgui::Gui &gui, networking* networkObject) : window(window), networkObject(networkObject) {
	mainTheme = tgui::Theme("socialTab.txt");

	float percentWidth = (float(sf::VideoMode::getDesktopMode().width - 400) / sf::VideoMode::getDesktopMode().width) * 100;
	float percentX = (float)(400.0f / (float)sf::VideoMode::getDesktopMode().width) * 100;

	chatBox = new chat(percentWidth, 70, percentX, 30); //setting temp values as also need to use absolute positioning
	chatBox->chatBoxContainer->setPosition(std::to_string(percentX) + "%", sf::VideoMode::getDesktopMode().height - chatBox->chatBoxContainer->getFullSize().y - 60);
	chatBox->chatBoxContainer->setPositionLocked(true); //so that it can't move

	chatBoxContainerHeight = sf::VideoMode::getDesktopMode().height * 0.7 + 50;
	chatBoxContainerYCoord = sf::VideoMode::getDesktopMode().height - chatBox->chatBoxContainer->getFullSize().y - 60;

	roomGuildSelectBox = tgui::ScrollablePanel::create({ std::to_string(100 - percentWidth) + "%", chatBox->chatBoxContainer->getFullSize().y + 1 });
	roomGuildSelectBox->setPosition(0, chatBox->chatBoxContainer->getPosition().y - 1);
	roomGuildSelectBox->setVerticalScrollbarPolicy(tgui::Scrollbar::Policy::Always);
	roomGuildSelectBox->setHorizontalScrollbarPolicy(tgui::Scrollbar::Policy::Never);
	//this will make the above occupy the left of the screen and have the same height as the chat box

	//below sets the stuff for the guild select box
	guildSelectBox = tgui::ScrollablePanel::create({ "80%", "80%" });
	guildSelectBox->setPosition("10%", chatBox->chatBoxContainer->getPosition().y);
	guildSelectBox->setVerticalScrollbarPolicy(tgui::Scrollbar::Policy::Always);
	guildSelectBox->setHorizontalScrollbarPolicy(tgui::Scrollbar::Policy::Never);
	guildSelectBox->setVisible(false);

	roomsBtn = tgui::Button::create("Rooms");
	roomsBtn->setPosition(0, chatBox->chatBoxContainer->getPosition().y - 40);
	roomsBtn->setSize("25%", 40);
	roomsBtn->connect("Clicked", &socialTabClass::switchTabs, this, roomsBtn->getText());
	areaChatBtn = tgui::Button::create("Area Chat");
	areaChatBtn->setPosition("25%", chatBox->chatBoxContainer->getPosition().y - 40);
	areaChatBtn->setSize("25%", 40);

	areaChatBtn->connect("Clicked", &socialTabClass::switchTabs, this, areaChatBtn->getText());

	guildSelectBtn = tgui::Button::create("Guilds");
	guildSelectBtn->setPosition("50%", chatBox->chatBoxContainer->getPosition().y - 40);
	guildSelectBtn->setSize("25%", 40);
	guildSelectBtn->connect("Clicked", &socialTabClass::switchTabs, this, guildSelectBtn->getText());
	privateMessagingBtn = tgui::Button::create("Private Messaging");
	privateMessagingBtn->setPosition("75%", chatBox->chatBoxContainer->getPosition().y - 40);
	privateMessagingBtn->setSize("25%", 40);
	privateMessagingBtn->connect("Clicked", &socialTabClass::switchTabs, this, privateMessagingBtn->getText());
	privateMessagingBtn->setEnabled(false); //temporarily disables it as this is a pretty complex feature so it'll be shipped later on

	socialTabGroup->add(roomsBtn);
	socialTabGroup->add(areaChatBtn);
	socialTabGroup->add(guildSelectBtn);
	socialTabGroup->add(privateMessagingBtn);
	socialTabGroup->add(chatBox->chatBoxContainer);
	socialTabGroup->add(roomGuildSelectBox);
	socialTabGroup->add(guildSelectBox);
	this->setActive(false);

	gui.add(socialTabGroup);
}

void socialTabClass::liveUpdate(sf::Clock* globalClock) {
	chatBox->liveUpdate(networkObject, globalClock); //calls the live update function for the chat box, so basically allows for sending messages
}

void socialTabClass::setActive(bool active) { //by setting the visibility through this, groups are really easy to manage so multiple screens are easy to manage
	//the function is called every frame for some reason, figure it out, so I take advantage of this and call the code to get data using threads here
	if (active == true) {
		//for the above, when the input parameter is true, it makes the group visible and sets the active boolean in the object as
		//true (used to decide whether or not to have the live update function be active or not in the main game loop)
		this->active = true;
		socialTabGroup->setVisible(true);

		//the chunk of code below will retrieve messages from another thread and then raise a flag (updateMsgGUI) which will add the retrieved messages to the chat box, it uses a lambda function
		if (guildRoomMsgUpdateThread) { //checks if it's not a nullptr
			guildRoomMsgUpdateThread->join(); //waits for it to finish execution
			delete guildRoomMsgUpdateThread; //deletes it
		}
		guildRoomMsgUpdateThread = new std::thread([this]() {
			networkObject->getMessagesFromDB();
			updateMsgGUI = true;
		});

		roomGuildBoxUpdateThread = new std::thread(&socialTabClass::populateRoomGuildSelectBox, this); //this will retrieve the room guild list, and add it to the GUI

		//the below 2 lines will make it so that the current chat box is the one to send messages to, and will add the currently store of messages to the chat box
		chatBoxBulkAdd(networkObject, chatBox);
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

void socialTabClass::addButtonToPanel(tgui::ScrollablePanel::Ptr panel, std::string text, float percentWidth, float &maxHeightVar, std::string joined, bool roomGuild) {
	auto button = tgui::Button::create(text);
	button->setSize({ "100%", button->getFullSize().y + 30 });

	//sets it's y position to be at the maximum current y position, so at the bottom of the chat box scrollable panel
	button->setPosition(0, maxHeightVar);

	button->setUserData(joined);

	if (!roomGuild) { //if the roomGuild parameter (final parameter) is false, then this is for joining/leaving guilds
		if (joined == "true") { //joined is applicable only for this type of button, if it's true then set the appropriate UI
			button->setRenderer(mainTheme.getRenderer("button.joined"));
		} else {
			button->setRenderer(mainTheme.getRenderer("button.notjoined")); //and similar for if it's not joined
		}
		button->connect("Clicked", &socialTabClass::changeGuild, this, button); //then connect the appropriate function
	} else { //else this is for changing room guild
		button->connect("Clicked", &socialTabClass::changeRoomGuild, this, button->getText()); //then connect the appropriate function
	}

	maxHeightVar += button->getFullSize().y; //updates the maximum y pos for the next message (assuming there is one)
	panel->add(button);
}

void socialTabClass::populateRoomGuildSelectBox() { //adds widgets to select what room guild to talk in
	roomGuildSelectBox->removeAllWidgets(); //removes all widgets from the roomGuildSelectBox
	currentMaxHeightRoomGuildSelect = 0;

	CURL* curl = curl_easy_init(); //we can set options for this to make it control how a transfer/transfers will be made
	std::string readBuffer; //string for the returning data

	curl_easy_setopt(curl, CURLOPT_URL, std::string("http://erewhon.xyz/game/roomGuild.php?id=" + std::to_string(networkObject->userID)).c_str());
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); //the callback
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); //will write data to the string, so the fourth param of the last callback is stored here

	curl_easy_perform(curl);
	delete curl;

	roomGuildList = json::parse(readBuffer);

	float percentWidthRoomGuildSelectBox = (roomGuildSelectBox->getFullSize().y / sf::VideoMode::getDesktopMode().width) * 100;

	for (int i = 0; i < roomGuildList.size(); i++) { //adds the clickable widgets for each room guild
		addButtonToPanel(roomGuildSelectBox, roomGuildList[i]["name"].get<std::string>(), percentWidthRoomGuildSelectBox, currentMaxHeightRoomGuildSelect, "", true);
	}
}

void socialTabClass::changeGuild(tgui::Button::Ptr button) { //this will change the colour of the buttons, and send a signal to the server, to indicate that you've joined/left the guild
	std::string joined = button->getUserData<std::string>();
	bool joinedBool = false;
	bool changedJoined = false; //this is to check if the changed thing actually occurred, as we disable this for the main guild

	if (joined == "true" && button->getText() != "main") {
		button->setUserData(std::string("false"));
		button->setRenderer(mainTheme.getRenderer("button.notjoined"));
		changedJoined = true;
	}
	else if (joined == "false") {
		button->setUserData(std::string("true"));
		button->setRenderer(mainTheme.getRenderer("button.joined"));
		joinedBool = true;
		changedJoined = true;
	}
	
	if (changedJoined) {
		CURL* curl = curl_easy_init(); //we can set options for this to make it control how a transfer/transfers will be made

		//basically a simple key which is unknown to the user is passed to the server to leave/join channels so only the client programs can communicate with the server
		std::string urlKey = curl_easy_escape(curl, key.c_str(), key.length());

		//then this request basically says to change to whatever
		curl_easy_setopt(curl, CURLOPT_URL, std::string("https://erewhon.xyz/game/joinLeaveGuild.php?guilds&userID=" + std::to_string(networkObject->userID) + "&key=" + urlKey + "&guildName=" + button->getText() + "&joined=" + std::to_string(joinedBool)).c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		curl_easy_perform(curl);
		delete curl;
	}

	if (roomGuildBoxUpdateThread) { //checks if this thread is not a nullptr, and then waits for it to finish current execution
		roomGuildBoxUpdateThread->join();
		delete roomGuildBoxUpdateThread; //deletes it after it has done its job
	}
	roomGuildBoxUpdateThread = new std::thread(&socialTabClass::populateRoomGuildSelectBox, this); //runs the function on a separate thread
}

void socialTabClass::changeRoomGuild(std::string buttonText) {
	if ((activeTab == "Rooms" || activeTab == "Area Chat") && networkObject->roomGuild != buttonText) { //checks for correct tab, and that the current room guild isn't the one clicked on
		chatBox->flushMessages();

		networkObject->roomGuild = buttonText; //the button text is the room guild name in this case, which we can use to request whatever data we need

		sf::Packet sendPacket; //the packet which will contain the data to send
		sendPacket << std::string("USER::CHANGEROOMGUILD::" + buttonText).c_str(); //converts the string into a C style array, and puts it into the packet which will be sent
		networkObject->socket->send(sendPacket);

		if (guildRoomMsgUpdateThread) { //checks if it's not a nullptr
			guildRoomMsgUpdateThread->join(); //waits for it to finish execution
			delete guildRoomMsgUpdateThread; //deletes it
		}
		//the chunk of code below will retrieve messages from another thread and then raise a flag (updateMsgGUI) which will add the retrieved messages to the chat box, it uses a lambda function
		guildRoomMsgUpdateThread = new std::thread([this]() {
			networkObject->getMessagesFromDB();
			updateMsgGUI = true;
		});
	}
}

void socialTabClass::populateGuildSelectBox() {
	guildSelectBox->removeAllWidgets();
	currentMaxHeightGuildSelect = 0;

	CURL* curl = curl_easy_init(); //we can set options for this to make it control how a transfer/transfers will be made
	std::string readBuffer; //string for the returning data

	curl_easy_setopt(curl, CURLOPT_URL, std::string("http://erewhon.xyz/game/roomGuild.php?guilds&id=" + std::to_string(networkObject->userID)).c_str());
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); //the callback
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); //will write data to the string, so the fourth param of the last callback is stored here

	curl_easy_perform(curl);
	delete curl;

	guildList = json::parse(readBuffer);

	float percentWidthGuildSelectBox = (guildSelectBox->getFullSize().y / sf::VideoMode::getDesktopMode().width) * 100;

	for (int i = 0; i < guildList.size(); i++) {
		addButtonToPanel(guildSelectBox, guildList[i]["name"].get<std::string>(), percentWidthGuildSelectBox, currentMaxHeightGuildSelect, guildList[i]["joined"].get<std::string>(), false);
	}
}

void socialTabClass::switchTabs(std::string buttonText) { //this will enable switching tabs and stuff
	if (buttonText == "Area Chat") {
		if (activeTab == "Rooms") {
			roomGuildSelectBox->setVisible(false); //makes the select box invisible
			//code to disable the area chat stuff
		} else if (activeTab == "Guilds") {
			guildSelectBox->setVisible(false); //makes the guild select box invisible
			//code to disable the guilds stuff
		} else if (activeTab == "Private Messaging") {
			//code to disable the private messaging stuff
		}
		if (activeTab != "Area Chat") { //this sets the UI to look like how it should for this specific tab, but only once
			chatBox->chatBoxContainer->setPosition("10%", sf::VideoMode::getDesktopMode().height - chatBox->chatBoxContainer->getFullSize().y - 60);  
			//sets correct positioning as the chat box is also used for the Rooms tab
			chatBox->chatBoxContainer->setSize("80%", chatBoxContainerHeight); //sets correct sizing as it's also used for the Rooms tab
			activeTab = "Area Chat"; //sets active tab
			changeRoomGuild("LOCALCHAT"); //changes the current chat rooom to localchat
			chatBox->chatBoxContainer->setVisible(true); //makes the chat box visible
		}
	}
	else if (buttonText == "Rooms") {
		if (activeTab == "Area Chat") {
			//code to disable the area chat stuff
		}
		if (activeTab == "Guilds") {
			guildSelectBox->setVisible(false); //makes the guild select box invisible
			//code to disable the guilds stuff
		}
		if (activeTab == "Private Messaging") {
			//code to disable the private messaging stuff
		}
		if (activeTab != "Rooms") { //the below basically just sets the chatBoxContainer and roomGuildBox container things to contain the stuff they need to, and to be the right sizes

			float percentX = (float)(400.0f / (float)sf::VideoMode::getDesktopMode().width) * 100; //gets the X percentage covering the screen
			chatBox->chatBoxContainer->setSize(std::to_string(100 - percentX) + "%", chatBoxContainerHeight); //sets the correct sizing of the chat box, as we change it for the local chat
			chatBox->chatBoxContainer->setPosition(std::to_string(percentX) + "%", chatBoxContainerYCoord); //sets the correct positioning of the chat box, as we change it for the local chat
			activeTab = "Rooms"; //sets the active tab

			//stuff below is done on every click on this button, just to make sure, maybe will work through them later to decrease overhead or whatever
			roomGuildSelectBox->setVisible(true); //makes the select box visible
			chatBox->chatBoxContainer->setVisible(true); //makes the chat box visible

			changeRoomGuild("main.alpha"); //changes the current chat room guild
		}

	}
	else if (buttonText == "Guilds") {
		if (activeTab == "Area Chat") {
			//code to disable the area chat stuff
			chatBox->chatBoxContainer->setVisible(false); //makes the chat box invisible
		}
		if (activeTab == "Rooms") {
			chatBox->chatBoxContainer->setVisible(false); //makes the chat box invisible
			roomGuildSelectBox->setVisible(false); //makes the select box invisible
			//code to disable the guilds stuff
		}
		if (activeTab == "Private Messaging") {
			//code to disable the private messaging stuff
		}
		if (activeTab != "Guilds") { //the below basically just sets the chatBoxContainer and roomGuildBox container things to contain the stuff they need to, and to be the right sizes
			if (guildSelectBoxUpdateThread) { //if it isn't nullptr
				guildSelectBoxUpdateThread->join(); //waits for it to finish execution
				delete guildSelectBoxUpdateThread; //deletes it
			}
			guildSelectBoxUpdateThread = new std::thread(&socialTabClass::populateGuildSelectBox, this); //runs the new threaded function

			activeTab = "Guilds"; //sets active tab

			//stuff below is done on every click on this button, just to make sure, maybe will work through them later to decrease overhead or whatever
			guildSelectBox->setVisible(true);
		}
	}
}

void socialTabClass::completeThreadWork() { //this is called in the main loop
	//it checks if the flag signalling that data has been received, has been raised, in which case add the data (messages to the chatbox), and reset the flag
	//this is to prevent messages being added to the chatbox while it is being drawn which crashes the program (Access violation reading location [some location in memory])
	if (updateMsgGUI) {
		chatBoxBulkAdd(networkObject, chatBox);
		updateMsgGUI = false;
	}
}

void socialTabClass::destroyThreads() {
	if (guildRoomMsgUpdateThread) {
		guildRoomMsgUpdateThread->join();
	}

	if (guildSelectBoxUpdateThread) {
		guildSelectBoxUpdateThread->join();
	}

	if (roomGuildBoxUpdateThread) {
		roomGuildBoxUpdateThread->join();
	}
}