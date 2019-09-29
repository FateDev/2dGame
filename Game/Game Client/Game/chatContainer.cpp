#define _CRT_SECURE_NO_WARNINGS
//the above so that I can use the time stuff

#include <time.h>
#include <network.h>
#include <gui.h>

chat::chat(float percentWidth, float percentHeight, float posPercentX, float posPercentY) : percentWidth(percentWidth), percentHeight(percentHeight) {
	tgui::Theme theme("Game.txt");
	//the above loads the colours and stuff for the widgets from the file "Game.txt"

	chatBox->setSize(float(sf::VideoMode::getDesktopMode().width)*0.01*percentWidth, float(sf::VideoMode::getDesktopMode().height)*0.01*percentHeight);
	//the above sets the chatBox scrollable panel to be the percentage of the window that the parameters specified
	chatBox->setVerticalScrollbarPolicy(tgui::ScrollablePanel::ScrollbarPolicy::Always);
	chatBox->setHorizontalScrollbarPolicy(tgui::ScrollablePanel::ScrollbarPolicy::Never);
	chatBox->setRenderer(theme.getRenderer("chatBox"));

	enterMessage->setPosition(10, float(sf::VideoMode::getDesktopMode().height)*0.01*percentHeight + 7);
	//above puts the enter message box 7px below the chatBox scrollable panel
	enterMessage->setSize(float(sf::VideoMode::getDesktopMode().width)*0.01*percentWidth - chatBox->getScrollbarWidth(), 30);
	enterMessage->setRenderer(theme.getRenderer("EditBox"));
	enterMessage->setDefaultText("Press enter to send a message... (3000 chars max)"); //sets the default text in the input box

	chatBoxContainer->setSize(float(sf::VideoMode::getDesktopMode().width)*0.01*percentWidth, float(sf::VideoMode::getDesktopMode().height)*0.01*percentHeight + 50);
	//the above sets the chatBoxContainer child window to be the percentage of the window that the parameters specified
	chatBoxContainer->setPosition(float(sf::VideoMode::getDesktopMode().width)*0.01*posPercentX, float(sf::VideoMode::getDesktopMode().height)*0.01*posPercentY);
	//the above sets the chatBoxContainer child window to be be positioned as the parameters specified
	chatBoxContainer->setTitleButtons(tgui::ChildWindow::TitleButton::None); //makes sure there are no buttons on the child window
	chatBoxContainer->setRenderer(theme.getRenderer("chatBoxContainer"));

	//below adds both the input field and the chat box to the child window
	chatBoxContainer->add(enterMessage);
	chatBoxContainer->add(chatBox);
}

tgui::Panel::Ptr chat::msgMaker(int time, std::string usernameText, std::string messageText, std::string imgLocation, bool continueFromLast) {
	lastTimeInSeconds = time;
	lastUsername = usernameText;

	float imgRatio = 0; //gets the y/x of the image
	float percentOfScreenX = float(sf::VideoMode::getDesktopMode().width) * 0.01 * percentWidth; //max set by params
	float maxWidth = 0; //the maximum width

	auto username = tgui::Label::create(usernameText);
	username->setSize({ percentOfScreenX, 20 });
	username->setPosition({ 10, 10 });
	username->setVerticalAlignment(tgui::Label::VerticalAlignment::Center);

	auto msg = tgui::Label::create(messageText);
	auto img = tgui::Picture::create(imgLocation);

	std::time_t result = time;
	auto timeLabel = tgui::Label::create(std::asctime(std::localtime(&result)));
	timeLabel->setPosition(10, 10 + username->getFullSize().y);
	//gets the time which was in seconds when received and converts to a readable date and time

	maxWidth = username->getFullSize().x - 10; //so the maximum any image can be, is the text size

	tgui::Theme theme("Game.txt");
	//the above loads the colours and stuff for the widgets from the file "Game.txt"

	username->setRenderer(theme.getRenderer("msg.username"));
	username->setTextSize(15);
	msg->setRenderer(theme.getRenderer("msg.content"));
	timeLabel->setTextSize(10);
	timeLabel->setRenderer(theme.getRenderer("timeLabel"));

	if (continueFromLast == true) {
		timeLabel->setSize(0, 0);
		username->setSize(0, 0);
	}

	if (imgLocation != "") { //if the image was specified, so the image location isn't empty
		if (messageText != "") { //and if the message was not
			//below just positions and sizes everything to appear right
			msg->setPosition(10, username->getFullSize().y + timeLabel->getFullSize().y - 3);
			msg->setMaximumTextWidth(percentOfScreenX - 40);  //sets the maximum width that the message can be
			img->setPosition({ 10, username->getFullSize().y + timeLabel->getFullSize().y + msg->getFullSize().y });
			imgRatio = img->getFullSize().y / img->getFullSize().x; //gets y/x of image

			if (img->getFullSize().y > maxWidth) {
				img->setSize(percentOfScreenX - 20, (percentOfScreenX - 20) * imgRatio); //if the height is greater than half the screen, then resize the width instead
			}
			else {
				img->setSize(1 / imgRatio * maxWidth, maxWidth); //otherwise resize the height
				img->setSize(percentOfScreenX - 20, (percentOfScreenX - 20) * imgRatio); //if the height is greater than half the screen, then resize the width instead
			}

			auto msgContainer = tgui::Panel::create({ username->getFullSize().x + 40, 10 + username->getFullSize().y + timeLabel->getFullSize().y + msg->getFullSize().y + img->getFullSize().y + 10 });
			//the y parameter of the above is the sum of the heights of every element + 10px for the size of the box containing the full message

			//adds all of the elements to the message container panel 
			msgContainer->add(timeLabel);
			msgContainer->add(msg);
			msgContainer->add(img);
			msgContainer->add(username);

			//and returns it
			return msgContainer;
		}
		else { //if the message was specified though do this
			//again, this just sets the sizes and positions of stuff, pretty self explanatory if you give it a read
			img->setPosition({ 10, username->getFullSize().y + timeLabel->getFullSize().y });
			imgRatio = img->getFullSize().y / img->getFullSize().x; //gets y/x of image

			if (img->getFullSize().y > maxWidth) {
				img->setSize(percentOfScreenX - 20, (percentOfScreenX - 20) * imgRatio); //if the height is greater than half the screen, then resize the width instead
			}
			else {
				img->setSize(1 / imgRatio * maxWidth, maxWidth); //otherwise resize the height
			}

			auto msgContainer = tgui::Panel::create({ username->getFullSize().x + 40, username->getFullSize().y + timeLabel->getFullSize().y + img->getFullSize().y + 10 });
			//the y parameter of the above is the sum of the heights of every element + 10px for the size of the box containing the full message

			//adds all of the elements to the message container panel 
			msgContainer->add(timeLabel);
			msgContainer->add(img);
			msgContainer->add(username);

			//and returns it
			return msgContainer;
		}
	}
	else { //in the case that an image was not specified
		//below just positions and sizes everything to appear right
		msg->setPosition(10, username->getFullSize().y + timeLabel->getFullSize().y - 3);
		msg->setMaximumTextWidth(percentOfScreenX - 40);

		auto msgContainer = tgui::Panel::create({ float(sf::VideoMode::getDesktopMode().width)*0.01*percentWidth - chatBox->getScrollbarWidth(), msg->getFullSize().y + username->getFullSize().y + timeLabel->getFullSize().y });
		//the y parameter of the above is the sum of the heights of every element + 10px for the size of the box containing the full message

		//adds all of the elements to the message container panel 
		msgContainer->add(timeLabel);
		msgContainer->add(msg);
		msgContainer->add(username);

		//and returns it
		return msgContainer;
	}
}

void chat::addMessages(int time, std::string usernameText, std::string messageText, std::string imgLocation, int msgID) {
	tgui::Panel::Ptr msg;

	if (lastTimeInSeconds + 600 >= time && usernameText == lastUsername) {
		msg = msgMaker(time, usernameText, messageText, imgLocation, true);
	}
	else {
		msg = msgMaker(time, usernameText, messageText, imgLocation, false);
	}
	//above just makes the msg panel

	//sets it's y position to be at the maximum current y position, so at the bottom of the chat box scrollable panel
	msg->setPosition(0, currentMaxHeight);

	currentMaxHeight += msg->getFullSize().y; //updates the maximum y pos for the next message (assuming there is one)

	//and adds the message to the chat box scrollable panel
	chatBox->add(msg);
	
	if (chatBox->getWidgets().size() >= 50) { //checks if the current amount of elements is more than 50
		float firstElemSizeY = chatBox->getWidgets()[0]->getFullSize().y; //if it is retrieve the height of the oldest element
		for (int i = 1; i < 50; i++) {
			chatBox->getWidgets()[i]->setPosition(chatBox->getWidgets()[i]->getPosition().x, chatBox->getWidgets()[i]->getPosition().y - firstElemSizeY);
			//and loop through and set the height of the other elements as their y positions minus the y size
		}
		currentMaxHeight -= firstElemSizeY; //and decrease the size of the current max height variable so that it is still correct

		chatBox->remove(chatBox->getWidgets()[0]); //and finally remove the oldest element
	}

	tgui::Theme theme("Game.txt");
	//the above loads the colours and stuff for the widgets from the file "Game.txt"
	msg->setRenderer(theme.getRenderer("msg"));

	chatBox->setVerticalScrollbarValue(currentMaxHeight);
	//and the above sets the scrollbar to be the new maximum y value so we see the new message
}

void chat::liveUpdate(networking* networkObject, sf::Clock* globalClock){ //called during the main game loop every frame
	if (enterMessage->isFocused() == true) { //checks if the enter message input box is focused
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) && globalClock->getElapsedTime().asMilliseconds()-lastMsgTime > 1000) { //checks if the enter key has been pressed
			lastMsgTime = globalClock->getElapsedTime().asMilliseconds();
			std::string msgContents = enterMessage->getText().toAnsiString(); //gets the contents of the enter message input box

			if (msgContents.length() < 3000 && msgContents.length() > 0) { //if the contents of the input box weren't empty
				networkObject->sendMessage(msgContents); //then use the network object's sendMessage method to send the contents as a message

				enterMessage->setText(""); //and sets the input boxes contents to be empty

				//and adds the message to the chat box scrollable panel with the current time as the time bit
			}
		}
	}
}

void chat::flushMessages() {
	chatBox->removeAllWidgets();
	lastUsername = "";
	currentMaxHeight = 0;
}