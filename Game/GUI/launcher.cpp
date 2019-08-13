//you need to define this to make sure <windows.h> doesn't conflict with any min()/max() functions, 
//as this header file has min() and max() functions as macros, so would otherwise cause conflix
#define NOMINMAX

#include <gui.h>
#include <TGUI/TGUI.hpp>
#include <windows.h>

launcher::launcher(networking *networkObject, sf::RenderWindow* mainWindow, sf::Thread *ping, sf::Thread *receive, sf::Thread *inputThread, sf::Clock *globalClock) : networkBit(networkObject), pingThread(ping), receiveThread(receive), window(mainWindow), input(inputThread), notifClock(globalClock) {
	//all this stuff is using TGUI and draws the window
	gui = new tgui::Gui(*window);

	tgui::Theme theme( "Launcher.txt" );

	auto background = tgui::Panel::create();
	background->setRenderer(theme.getRenderer("Panel.Panel2"));
	background->setSize(800, 600);
	gui->add(background);

	auto credentialBox = tgui::Panel::create();
	credentialBox->setRenderer(theme.getRenderer("Panel.Panel1"));
	credentialBox->setSize(370, 90);
	credentialBox->setPosition(20, 480);
	gui->add(credentialBox);

	auto titleBox = tgui::Panel::create();
	titleBox->setRenderer(theme.getRenderer("Panel.Panel3"));
	titleBox->setPosition(20, 23);
	titleBox->setSize(760, 97);
	gui->add(titleBox);

	auto title = tgui::Label::create();
	title->setRenderer(theme.getRenderer("Label.Label0"));
	title->setPosition(30, 21.4);
	title->setSize(738.4, 100);
	title->setText("window");
	title->setTextSize(60);
	title->setVerticalAlignment(tgui::Label::VerticalAlignment::Center);
	title->setHorizontalAlignment(tgui::Label::HorizontalAlignment::Center);
	title->ignoreMouseEvents(true);
	title->setScrollbarPolicy(tgui::Scrollbar::Policy::Never);
	gui->add(title);

	auto passwordLabel = tgui::Label::create();
	passwordLabel->setRenderer(theme.getRenderer("Label.Label2"));
	passwordLabel->setPosition(30, 530);
	passwordLabel->setSize(97.2, 30);
	passwordLabel->setText("Password:");
	passwordLabel->setTextSize(16);
	passwordLabel->setVerticalAlignment(tgui::Label::VerticalAlignment::Center);
	passwordLabel->setScrollbarPolicy(tgui::Scrollbar::Policy::Never);
	gui->add(passwordLabel);

	auto usernameLabel = tgui::Label::create();
	usernameLabel->setRenderer(theme.getRenderer("Label.Label1"));
	usernameLabel->setPosition(30, 490);
	usernameLabel->setSize(97.2, 30);
	usernameLabel->setText("Username:");
	usernameLabel->setTextSize(16);
	usernameLabel->setVerticalAlignment(tgui::Label::VerticalAlignment::Center);
	usernameLabel->setScrollbarPolicy(tgui::Scrollbar::Policy::Never);
	gui->add(usernameLabel);

	auto picture = tgui::Picture::create();
	picture->setRenderer(theme.getRenderer("Picture.Picture1"));
	picture->setSize(760, 310);
	picture->setPosition(20, 140);
	gui->add(picture);

	auto usernameField = tgui::EditBox::create();
	usernameField->setRenderer(theme.getRenderer("EditBox"));
	usernameField->setSize(245, 30);
	usernameField->setTextSize(13);
	usernameField->setPosition(135, 490);
	usernameField->setDefaultText("");
	gui->add(usernameField);

	auto passwordField = tgui::EditBox::create();
	passwordField->setRenderer(theme.getRenderer("EditBox"));
	passwordField->setSize(245, 30);
	passwordField->setTextSize(13);
	passwordField->setPosition(135, 530);
	passwordField->setDefaultText("");
	passwordField->setPasswordCharacter('*');
	gui->add(passwordField);

	playButton->setRenderer(theme.getRenderer("Button"));
	playButton->setSize(240, 90);
	playButton->setTextSize(30);
	playButton->setPosition(410, 480);
	playButton->setText("Play");
	gui->add(playButton);

	auto helpButton = tgui::Button::create();
	helpButton->setRenderer(theme.getRenderer("Button"));
	helpButton->setSize(110, 90);
	helpButton->setTextSize(35);
	helpButton->setPosition(670, 480);
	helpButton->setText("?");
	gui->add(helpButton);

	loginUnsuccessful->setRenderer(theme.getRenderer("Panel.Panel4"));
	loginUnsuccessful->setSize(500, 70);
	loginUnsuccessful->setPosition(140, 40);
	gui->add(loginUnsuccessful);

	unsuccessfulLabel->setRenderer(theme.getRenderer("Label.Label3"));
	unsuccessfulLabel->setPosition(140, 41.42);
	unsuccessfulLabel->setSize(497.2, 66.38);
	unsuccessfulLabel->setText("Login Unsuccessful");
	unsuccessfulLabel->setTextSize(18);
	unsuccessfulLabel->setVerticalAlignment(tgui::Label::VerticalAlignment::Center);
	unsuccessfulLabel->setHorizontalAlignment(tgui::Label::HorizontalAlignment::Center);
	unsuccessfulLabel->setScrollbarPolicy(tgui::Scrollbar::Policy::Never);
	gui->add(unsuccessfulLabel);

	//makes the notification of wrong password and stuff invisible until needed
	loginUnsuccessful->setVisible(false);
	unsuccessfulLabel->setVisible(false);

	helpButton->connect("pressed", &launcher::helpFunction, this); //executres the function to open a help page in the browser

	//connects the playButton with the loginFunction
	playButton->connect("pressed", &launcher::loginFunctionFromWindow, this, usernameField, passwordField);
	//the 1st param is the event, the 2nd param is a reference (&) to the general function, the 3rd is a reference to the instance of the object for that function from 2nd param, and 3rd and 4th are the 2 text fields to get credentials from
}

void launcher::loginFunctionFromWindow(tgui::EditBox::Ptr usernameBox, tgui::EditBox::Ptr passwordBox) //function will take the text in username and password boxes and pass it to the login function
{
	//makes the input into text
	std::string username = usernameBox->getText().toAnsiString();
	std::string password = passwordBox->getText().toAnsiString();

	if (networkBit->login(username, password)) { //this will ask the verify login details, true or false depending on whether or not verified
		pingThread->launch(); //will maintain the connection by pinging every so often
		receiveThread->launch(); //will receive any server orders
		input->launch(); //launch the input thread so to talk to other users
		playButton->setEnabled(false); //disables the playButton as you've managed to log in at this point
	}
	else {
		loginError = 1; //uf you can login then it'll show the error message, rather part 1 of it
	}
}


void launcher::liveUpdate() {
	if (loginError == 1) { //this is part 1 of the error message
		//the error notification is show
		loginUnsuccessful->setVisible(true);
		unsuccessfulLabel->setVisible(true);

		notifTime = notifClock->getElapsedTime(); //and the current time is recorded
		loginError = 2; //and then it's set to go to part 2
	}
	else if (loginError == 2 && notifClock->getElapsedTime().asMilliseconds() - notifTime.asMilliseconds() >= 1000) { //there's a check to see if it's part 2's turn, if it is check if it's been 1 second (1000ms) or more since the time was last recorded
		//if the above condition is true, reset our time recordings for future use
		notifTime = sf::milliseconds(0);

		//and make the notifications disappear
		loginUnsuccessful->setVisible(false);
		unsuccessfulLabel->setVisible(false);

		//and reset the loginError int for use again later
		loginError = 0;
	}
	gui->draw(); // Draw all widgets
}

void launcher::helpFunction() { //the function to make the help button open the help setting in browser
	ShellExecute(0, 0, L"https://erewhon.xyz/game/help/", 0, 0, SW_SHOW);
	//opens URL in browser,
	//1st param is handle to parent windows, but we're using SFML rather than win32 windows so this is NULL or 0
	//2nd param is the action, which isnt necessary here
	//3rd is the thing to 'do' (the URL), so the default browser would be used on windows
	//4th would be parameters to pass to the file, but not necessary for opening a URL
	//5th is the working directory of the action, none specified so current one used
	//6th is how to show the application once opened, so it shows it
}

launcher::~launcher() {
	delete gui;
}