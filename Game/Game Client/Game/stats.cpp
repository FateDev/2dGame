#include <gui.h>
#include <network.h>

std::string roundForString2dp(float number) {
	char str[15]; //15 character array
	sprintf_s(str, "%.2f", number); //print value of number into str, to 2dp
	return std::string(str);
}

stats::stats(tgui::Gui &gui, networking* networkingObj) : networkingObj(networkingObj) {
	tgui::Theme theme("Game.txt");

	statsBox = tgui::Panel::create({ "15%", "12%" });
	statsBox->setPosition("85%", "10%");
	statsBox->setRenderer(theme.getRenderer("Panel_Stats"));

	mpBarRight = tgui::Panel::create({"15%", "4%"});
	mpBarRight->setPosition("85%", "14%");
	mpBarRight->setRenderer(theme.getRenderer("mpBarRight"));
	mpBarLeft = tgui::Panel::create({ "15%", "4%" });
	mpBarLeft->setPosition("85%", "14%");
	mpBarLeft->setRenderer(theme.getRenderer("mpBarLeft"));
	hpBarRight = tgui::Panel::create({ "15%", "4%" });
	hpBarRight->setPosition("85%", "10%");
	hpBarRight->setRenderer(theme.getRenderer("hpBarRight"));
	hpBarLeft = tgui::Panel::create({ "15%", "4%" });
	hpBarLeft->setPosition("85%", "10%");
	hpBarLeft->setRenderer(theme.getRenderer("hpBarLeft"));

	hp = tgui::Label::create("HP: " + roundForString2dp(mpVal));
	hp->setSize("15%", "4%");
	hp->setPosition("85%", "10%");
	hp->setRenderer(theme.getRenderer("Label_HP"));
	hp->setTextSize(18);
	hp->setVerticalAlignment(tgui::Label::VerticalAlignment::Center);
	hp->setHorizontalAlignment(tgui::Label::HorizontalAlignment::Center);

	mp = tgui::Label::create("MP: " + roundForString2dp(mpVal));
	mp->setSize("15%", "4%");
	mp->setTextSize(18);
	mp->setPosition("85%", "14%");
	mp->setRenderer(theme.getRenderer("Label_MP"));
	mp->setVerticalAlignment(tgui::Label::VerticalAlignment::Center);
	mp->setHorizontalAlignment(tgui::Label::HorizontalAlignment::Center);

	balance = tgui::Label::create("Balance: $" + roundForString2dp(balanceVal));
	balance->setSize("15%", "4%");
	balance->setTextSize(18);
	balance->setPosition("85%", "18%");
	balance->setRenderer(theme.getRenderer("Label_Balance"));
	balance->setVerticalAlignment(tgui::Label::VerticalAlignment::Center);
	balance->setHorizontalAlignment(tgui::Label::HorizontalAlignment::Center);

	userStats->add(statsBox);
	userStats->add(hpBarLeft);
	userStats->add(hpBarRight);
	userStats->add(mpBarLeft);
	userStats->add(mpBarRight);
	userStats->add(mp);
	userStats->add(hp);
	userStats->add(balance);

	gui.add(userStats);
	userStats->setVisible(false); //want it invisible while the loading screen is active
}

void stats::setBalance(int balanceSet) {
	balanceVal = balanceSet;
}

void stats::setHP(int hpSet) {
	hpVal = hpSet;
}

void stats::setMP(int mpSet) {
	mpVal = mpSet;
}

void stats::setMaxHp(int maxHpSet) {
	//mp->setText("HP: " + roundForString2dp(maxHpSet));
	max_hpVal = maxHpSet;
}

void stats::setMaxMp(int maxMpSet) {
	//hp->setText("MP: " + roundForString2dp(maxMpSet));
	max_mpVal = maxMpSet;
}

void stats::updateStats() {
	float mpRatio = float(mpVal) / float(max_mpVal);
	float hpRatio = float(hpVal) / float(max_hpVal);
	mpBarLeft->setSize(std::to_string(mpRatio * 15) + "%", "4%");
	mpBarRight->setSize(std::to_string((1 - mpRatio) * 15) + "%", "4%");
	mpBarRight->setPosition(std::to_string(85 + (mpRatio * 15)) + "%", "14%");
	hpBarLeft->setSize(std::to_string(hpRatio * 15) + "%", "4%");
	hpBarRight->setSize(std::to_string((1 - hpRatio) * 15) + "%", "4%");
	hpBarRight->setPosition(std::to_string(85 + (hpRatio * 15)) + "%", "10%");

	balance->setText("Balance: $" + roundForString2dp(float(balanceVal) / 100));
	hp->setText("HP: " + roundForString2dp(float(hpVal) / 100));
	mp->setText("MP: " + roundForString2dp(float(mpVal) / 100));
}