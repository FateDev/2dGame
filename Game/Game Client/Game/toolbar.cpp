#include <gui.h>
#include <header.h>

toolbar::toolbar(sf::RenderWindow *window, mainScreen* main_screen, socialTabClass* socialTabBit, tgui::Gui &gui) : window(window), main_screen(main_screen), socialTabBit(socialTabBit) {
	tgui::Theme theme("Game.txt");
	//the above loads the colours and stuff for the widgets from the file "Game.txt"

	//the below makes 7 adjacent buttons with different icons
	for (int i = 0; i < 8; i++) {
		buttons.push_back(tgui::BitmapButton::create());
		buttons[i]->setRenderer(theme.getRenderer("Button"));
		buttons[i]->setSize("5%", "8%");
		buttons[i]->setPosition(std::to_string(5 * i) + "%", 0);
		buttons[i]->setImage(icons[i]);
		buttons[i]->setImageScaling(0.7);

		toolbarGroup->add(buttons[i]); //adds current one to the toolbar group
	}


	//bottom 2 connect 2 buttons with functions, there will be more
	buttons[2]->connect("pressed", &toolbar::socialTab, this);
	buttons[0]->connect("pressed", &toolbar::returnToMain, this);
	buttons[6]->connect("pressed", &helpFunction); //executres the function to open a help page in the browser
	buttons[7]->connect("pressed", &toolbar::exitFunction, this); //executes the function to open a help page in the browser

	gui.add(toolbarGroup);
	toolbarGroup->setVisible(false);
}

void toolbar::exitFunction() { //this is triggered by the exit button
	window->close(); //program is closed when the button is clicked
}

void toolbar::returnToMain() {
	if (main_screen->active != true) {
		main_screen->setActive(true);
		socialTabBit->setActive(false);
	}
}

void toolbar::socialTab() {
	if (socialTabBit->active != true) {
		main_screen->setActive(false);
		socialTabBit->setActive(true);
	}
}