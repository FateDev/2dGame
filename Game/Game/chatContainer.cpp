#define _CRT_SECURE_NO_WARNINGS
//the above so that I can use the time stuff

#include <time.h>
#include <network.h>
#include <gui.h>

chat::chat(float percentWidth, float percentHeight) {
	tgui::Theme theme("Game.txt");
	chatBox->setSize(float(sf::VideoMode::getDesktopMode().width)*0.25, float(sf::VideoMode::getDesktopMode().height)*0.5);
	chatBox->setHorizontalScrollbarPolicy(tgui::ScrollablePanel::ScrollbarPolicy::Never);
	chatBox->setRenderer(theme.getRenderer("chatBox"));

	enterMessage->setPosition(10, float(sf::VideoMode::getDesktopMode().height)*0.5+7);
	enterMessage->setSize(float(sf::VideoMode::getDesktopMode().width)*0.23, 30);
	enterMessage->setRenderer(theme.getRenderer("EditBox"));
	enterMessage->setDefaultText("Press enter to send a message...");

	chatBoxContainer->setSize(float(sf::VideoMode::getDesktopMode().width)*0.25, float(sf::VideoMode::getDesktopMode().height)*0.5 + 50);
	chatBoxContainer->setTitleButtons(tgui::ChildWindow::TitleButton::None);
	chatBoxContainer->setPosition(float(sf::VideoMode::getDesktopMode().width)*0.02, float(sf::VideoMode::getDesktopMode().height)*0.1);
	chatBoxContainer->setRenderer(theme.getRenderer("chatBoxContainer"));

	chatBoxContainer->add(enterMessage);
	chatBoxContainer->add(chatBox);
}

tgui::Panel::Ptr chat::msgMaker(int time, std::string usernameText, std::string messageText, std::string imgLocation) {
	float imgRatio = 0;
	float percentOfScreenX = (float(sf::VideoMode::getDesktopMode().width) / 100) * percent; //max set by params
	//float halfScreen = (float(sf::VideoMode::getDesktopMode().height) / 100) * 50; //max anything should be, set right here
	float maxWidth = 0;

	auto username = tgui::Label::create(usernameText);
	username->setSize({ percentOfScreenX, 20 });
	username->setPosition({ 10, 10 });
	username->setVerticalAlignment(tgui::Label::VerticalAlignment::Center);

	auto msg = tgui::Label::create(messageText);
	auto img = tgui::Picture::create(imgLocation);

	std::time_t result = time;
	auto timeLabel = tgui::Label::create(std::asctime(std::localtime(&result)));
	timeLabel->setPosition({ 10, 10 + username->getSize().y });

	maxWidth = username->getSize().x - 10; //so the maximum any image can be, is the text size

	tgui::Theme theme("Game.txt");
	username->setRenderer(theme.getRenderer("msg.username"));
	username->setTextSize(15);
	msg->setRenderer(theme.getRenderer("msg.content"));
	timeLabel->setTextSize(10);
	timeLabel->setRenderer(theme.getRenderer("timeLabel"));

	if (messageText != "") {
		msg->setPosition(10, username->getSize().y + timeLabel->getSize().y - 5);
		msg->setMaximumTextWidth(percentOfScreenX - 40);
		img->setPosition({ 10, username->getSize().y + timeLabel->getSize().y + msg->getSize().y});
		imgRatio = img->getSize().y / img->getSize().x; //gets y/x of image

		if (img->getSize().y > maxWidth) {
			img->setSize(percentOfScreenX - 20, (percentOfScreenX - 20) * imgRatio); //if the height is greater than half the screen, then resize the width instead
		}
		else {
			img->setSize(1 / imgRatio * maxWidth, maxWidth); //otherwise resize the height
			img->setSize(percentOfScreenX - 20, (percentOfScreenX - 20) * imgRatio); //if the height is greater than half the screen, then resize the width instead
		}

		auto msgContainer = tgui::Panel::create({ username->getSize().x + 40, 10 + username->getSize().y + timeLabel->getSize().y + msg->getSize().y + img->getSize().y + 10 });

		msgContainer->add(timeLabel);
		msgContainer->add(msg);
		msgContainer->add(img);
		msgContainer->add(username);

		return msgContainer;
	}
	else {
		img->setPosition({ 10, username->getSize().y + timeLabel->getSize().y });
		imgRatio = img->getSize().y / img->getSize().x; //gets y/x of image

		if (img->getSize().y > maxWidth) {
			img->setSize(percentOfScreenX - 20, (percentOfScreenX - 20) * imgRatio); //if the height is greater than half the screen, then resize the width instead
		}
		else {
			img->setSize(1 / imgRatio * maxWidth, maxWidth); //otherwise resize the height
		}

		auto msgContainer = tgui::Panel::create({ username->getSize().x + 40, username->getSize().y + timeLabel->getSize().y + img->getSize().y + 10});

		msgContainer->add(timeLabel);
		msgContainer->add(img);
		msgContainer->add(username);

		return msgContainer;
	}
}

tgui::Panel::Ptr chat::msgMaker(int time, std::string usernameText, std::string messageText) {
	float percentOfScreenX = (float(sf::VideoMode::getDesktopMode().width) / 100) * percent;

	auto username = tgui::Label::create(usernameText);
	username->setSize({ percentOfScreenX, 20 });
	username->setPosition({ 10, 10 });
	username->setVerticalAlignment(tgui::Label::VerticalAlignment::Center);

	std::time_t result = time;
	auto timeLabel = tgui::Label::create(std::asctime(std::localtime(&result)));
	timeLabel->setPosition({ 10, 10 + username->getSize().y });

	auto msg = tgui::Label::create(messageText);
	msg->setPosition(10, username->getSize().y + timeLabel->getSize().y - 5);
	msg->setMaximumTextWidth(percentOfScreenX - 40);

	auto msgContainer = tgui::Panel::create({ msg->getSize().x + 40, msg->getSize().y + username->getSize().y + timeLabel->getSize().y });

	msgContainer->add(timeLabel);
	msgContainer->add(username);
	msgContainer->add(msg);

	tgui::Theme theme("Game.txt");
	username->setRenderer(theme.getRenderer("msg.username"));
	username->setTextSize(15);
	msg->setRenderer(theme.getRenderer("msg.content"));
	timeLabel->setTextSize(10);
	timeLabel->setRenderer(theme.getRenderer("timeLabel"));

	return msgContainer;
}

void chat::addMessages(int time, std::string usernameText, std::string messageText, std::string imgLocation) {
	tgui::Panel::Ptr msg = msgMaker(time, usernameText, messageText, imgLocation);

	msg->setPosition(0, currentMaxHeight);
	currentMaxHeight += msg->getSize().y;

	chatBox->add(msg);

	tgui::Theme theme("Game.txt");
	msg->setRenderer(theme.getRenderer("msg"));

	chatBox->m_horizontalScrollbar->setValue(chatBox->m_horizontalScrollbar->getMaximum());
}

void chat::addMessages(int time, std::string usernameText, std::string messageText) {
	tgui::Panel::Ptr msg = msgMaker(time, usernameText, messageText);

	msg->setPosition(0, currentMaxHeight);
	currentMaxHeight += msg->getSize().y;

	chatBox->add(msg);

	tgui::Theme theme("Game.txt");
	msg->setRenderer(theme.getRenderer("msg"));

	chatBox->m_horizontalScrollbar->setValue(chatBox->m_horizontalScrollbar->getMaximum());

}

void chat::liveUpdate(networking* networkObject){
	if (enterMessage->isFocused() == true && active == true) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
			std::string msgContents = enterMessage->getText().toAnsiString();

			if (msgContents != "") {
				networkObject->sendMessage(msgContents);

				enterMessage->setText("");
				addMessages(std::time(nullptr), networkObject->usernameReal, msgContents);
			}
		}
	}
}