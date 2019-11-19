#include <gui.h>
#include <header.h>

loadingScreen::loadingScreen(sf::RenderWindow *window, sf::Clock *clock) : gameWindow(window), loadingScreenClock(clock) {
	gui = new tgui::Gui(*gameWindow);
	tgui::Theme theme("Loading.txt");
	//above sets the theme, which can load properties describing colour and stuff

	//below makes a circle centered at the center of the screen
	shape = new sf::CircleShape(50.0f);
	shape->setOrigin(50, 50);
	shape->setPosition(sf::VideoMode::getDesktopMode().width / 2, sf::VideoMode::getDesktopMode().height / 2);

	//below creates a label just below the center of the screen which just says "Loading"
	auto loading = tgui::Label::create();
	loading->setRenderer(theme.getRenderer("Label.Label1"));
	loading->setSize(200, 50);
	loading->setText("Loading");
	loading->setTextSize(40);
	loading->setPosition(sf::VideoMode::getDesktopMode().width / 2 - loading->getFullSize().x / 2.f, sf::VideoMode::getDesktopMode().height / 2 - loading->getFullSize().y / 2.f + 110);
	loading->setHorizontalAlignment(tgui::Label::HorizontalAlignment::Center);
	loading->setScrollbarPolicy(tgui::Scrollbar::Policy::Never);
	gui->add(loading);
}

void loadingScreen::liveUpdate() {
	//below uses time and a sin function to make a loading icon
	shape->setScale((sin(float(loadingScreenClock->getElapsedTime().asMilliseconds()) / 200) + 1.5) / 2, (sin(float(loadingScreenClock->getElapsedTime().asMilliseconds()) / 200) + 1.5) / 2);
	
	gui->draw(); //draws the loading label
	gameWindow->draw(*shape); //draws the loading icon
}

loadingScreen::~loadingScreen(){
	delete gui; //deletes the loading screen gui
	delete shape; //and deletes that circle thing
}