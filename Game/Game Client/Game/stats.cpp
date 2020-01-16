#include <gui.h>
#include <network.h>

std::string roundForString2dp(float number) {
	char str[15]; //15 character array
	sprintf_s(str, "%.2f", number); //print value of number into str, to 2dp
	return std::string(str);
}

stats::stats(tgui::Gui &gui, networking* networkingObj, float trueWinHeight, float trueWinWidth) : networkingObj(networkingObj) {
	tgui::Theme theme("Game.txt");

	statsBox = tgui::Panel::create({ "15%", "12%" });
	statsBox->setPosition("85%", "10%");
	statsBox->setRenderer(theme.getRenderer("Panel_Stats"));

	hp = tgui::Label::create("HP: " + roundForString2dp(networkingObj->hp));
	hp->setSize("15%", "4%");
	hp->setPosition("85%", "10%");
	hp->setRenderer(theme.getRenderer("Label_HP"));
	hp->setTextSize(20);
	hp->setVerticalAlignment(tgui::Label::VerticalAlignment::Center);
	hp->setHorizontalAlignment(tgui::Label::HorizontalAlignment::Center);

	mp = tgui::Label::create("MP: " + roundForString2dp(networkingObj->mp));
	mp->setSize("15%", "4%");
	mp->setTextSize(20);
	mp->setPosition("85%", "14%");
	mp->setRenderer(theme.getRenderer("Label_MP"));
	mp->setVerticalAlignment(tgui::Label::VerticalAlignment::Center);
	mp->setHorizontalAlignment(tgui::Label::HorizontalAlignment::Center);

	balance = tgui::Label::create("Balance: $" + roundForString2dp(networkingObj->balance));
	balance->setSize("15%", "4%");
	balance->setTextSize(20);
	balance->setPosition("85%", "18%");
	balance->setRenderer(theme.getRenderer("Label_Balance"));
	balance->setVerticalAlignment(tgui::Label::VerticalAlignment::Center);
	balance->setHorizontalAlignment(tgui::Label::HorizontalAlignment::Center);

	userStats->add(statsBox);
	userStats->add(mp);
	userStats->add(hp);
	userStats->add(balance);

	gui.add(userStats);
}

void stats::setBalance(float balanceSet) {
	balance->setText("Balance: " + roundForString2dp(balanceSet));
}

void stats::setHP(float hpSet) {
	mp->setText("HP: " + roundForString2dp(hpSet));
}

void stats::setMP(float mpSet) {
	hp->setText("MP: " + roundForString2dp(mpSet));
}