#include <header.h>
#include <gui.h>

mainScreen::mainScreen(sf::RenderWindow *window, tgui::Gui &gui) : gameWindow(window) {
	
	tgui::Theme theme("Game.txt");

	for (int i = 0; i < 7; i++) {
		buttons.push_back(tgui::BitmapButton::create());
		buttons[i]->setRenderer(theme.getRenderer("Button"));
		buttons[i]->setSize("5%", "8%");
		buttons[i]->setPosition(std::to_string(5 * i) + "%", 0);
		buttons[i]->setImage(icons[i]);

		mainScreenGroup->add(buttons[i]);
	}

	for (int i = 7; i < 14; i++) {
		buttons.push_back(tgui::BitmapButton::create());
		buttons[i]->setRenderer(theme.getRenderer("Button2"));
		buttons[i]->setSize("5%", "8%");
		buttons[i]->setPosition(std::to_string(65 + 5 * (i - 7)) + "%", 0);

		mainScreenGroup->add(buttons[i]);
	}

	buttons[7]->setRenderer(theme.getRenderer("Button3"));
	buttons[13]->setRenderer(theme.getRenderer("Button4"));
	buttons[13]->setImage(icons[8]);

	buttons[5]->connect("pressed", &helpFunction); //executres the function to open a help page in the browser
	buttons[6]->connect("pressed", &mainScreen::exitFunction, this); //executes the function to open a help page in the browser

	/////////////////////////////
	////////ADD GROUP STUFF
	gui.add(mainScreenGroup);
	mainScreenGroup->setVisible(false);
}

void mainScreen::setActive(bool active) { //by setting the visibility through this, groups are really easy to manage so multiple screens are easy to manage
	if (active == true) {
		mainScreenGroup->setVisible(true);
	}
	else {
		mainScreenGroup->setVisible(false);
	}
}

void mainScreen::exitFunction() {
	gameWindow->close();
}
