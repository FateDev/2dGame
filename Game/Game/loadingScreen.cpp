#include <gui.h>
#include <header.h>

loadingScreen::loadingScreen(sf::RenderWindow *window, sf::Clock *clock) : gameWindow(window), loadingScreenClock(clock) {
	gui = new tgui::Gui(*gameWindow);
	tgui::Theme theme("Loading.txt");

	shape = new sf::CircleShape(50.0f);
	shape->setOrigin(50, 50);
	shape->setPosition(sf::VideoMode::getDesktopMode().width / 2, sf::VideoMode::getDesktopMode().height / 2);

	auto background = tgui::Panel::create();
	background->setRenderer(theme.getRenderer("Panel.Panel1"));
	background->setSize("100%", "100%");
	background->setPosition(0, 0);
	gui->add(background);

	auto loading = tgui::Label::create();
	loading->setRenderer(theme.getRenderer("Label.Label1"));
	loading->setSize(200, 50);
	loading->setText("Loading");
	loading->setTextSize(40);
	loading->setPosition(sf::VideoMode::getDesktopMode().width / 2 - loading->getSize().x / 2.f, sf::VideoMode::getDesktopMode().height / 2 - loading->getSize().y / 2.f + 110);
	loading->setHorizontalAlignment(tgui::Label::HorizontalAlignment::Center);
	loading->setScrollbarPolicy(tgui::Scrollbar::Policy::Never);
	gui->add(loading);
}

void loadingScreen::liveUpdate() {
	shape->setScale((sin(float(loadingScreenClock->getElapsedTime().asMilliseconds()) / 200) + 1.5) / 2, (sin(float(loadingScreenClock->getElapsedTime().asMilliseconds()) / 200) + 1.5) / 2);
	
	gui->draw();
	gameWindow->draw(*shape);
}

loadingScreen::~loadingScreen(){
	delete gui; //deletes the loading screen gui
	delete shape; //and deletes that circle thing
}