#include <gui.h>
#include <header.h>

toolbar::toolbar(sf::RenderWindow *window, tgui::Gui &gui) : window(window) {
	tgui::Theme theme("Game.txt");
	//the above loads the colours and stuff for the widgets from the file "Game.txt"

	//the below makes 7 adjacent buttons with different icons
	for (int i = 0; i < 7; i++) {
		buttons.push_back(tgui::BitmapButton::create());
		buttons[i]->setRenderer(theme.getRenderer("Button"));
		buttons[i]->setSize("5%", "8%");
		buttons[i]->setPosition(std::to_string(5 * i) + "%", 0);
		buttons[i]->setImage(icons[i]);

		toolbarGroup->add(buttons[i]); //adds current one to the main group
	}

	//bottom 2 connect 2 buttons with functions, there will be more
	buttons[5]->connect("pressed", &helpFunction); //executres the function to open a help page in the browser
	buttons[6]->connect("pressed", &toolbar::exitFunction, this); //executes the function to open a help page in the browser

	gui.add(toolbarGroup);
	toolbarGroup->setVisible(false);
}

void toolbar::exitFunction() { //this is triggered by the exit button
	window->close(); //program is closed when the button is clicked
}