//this is an example file provided by a SFML dev

#include <iostream>
#include <SFML/Network.hpp>

const unsigned short PORT = 5000;
const std::string IPADDRESS("erewhon.xyz");//change to suit your needs

std::string msgSend; //message string, global variable so i can use in DoStuff() and GetInput()

sf::TcpSocket socket; //tcp socket
sf::Mutex globalMutex; //kinda like a flag, if it already 'locked', then any other part of the program that tries to 'lock' it will pause
bool quit = false; //bool to quite the program or not, singlalling the destruction of a thread

//using void as argument makes it possible for compiler to tell you youve made mistake if you pass arg to function, making debugging easier and stuff

void DoStuff(void) //runs on another thread than the main one
{
	static std::string oldMsg; //static variable initialised once and retains it's value, used to prevent multiple of the same message sent consecutively (blocks spam)
	while(!quit) //if so long as the quit flag isnt raised
	{
		sf::Packet packetSend; //a sfml packet
		globalMutex.lock(); //locks the mutex
		packetSend << msgSend; //adds the message to be sent to
		globalMutex.unlock(); //unlocks for use later
		
		socket.send(packetSend); //sends the packet
		
		std::string msg; //the recieving msg string
		sf::Packet packetReceive; //the recieving packet
		
		socket.receive(packetReceive); //recieves a packet
		if ((packetReceive >> msg) && oldMsg != msg && !msg.empty()) //if there is a message to recieve, and the previous message isnt equivalent to this one, and the message isnt empty, proceed
		{
			std::cout << msg << std::endl; //output the message
			oldMsg = msg; //make this the old message
		} //so basically proceeds as normal if it doesnt recieve a message
	}
}

void Server(void)
{
	sf::TcpListener listener; //open a listener if a server
	listener.listen(PORT); //listen on that port
	listener.accept(socket); //accept a socket when it connects
	std::cout << "New client connected: " << socket.getRemoteAddress() << std::endl; //output socket address
}

bool Client(void)
{
	if(socket.connect(IPADDRESS, PORT) == sf::Socket::Done) //if successfully connected
	{
		std::cout << "Connected\n";
		return true;
	}
	return false;
}

void GetInput(void)
{
	std::string s; //temps tring
	std::cout << "\nEnter \"exit\" to quit or message to send: ";
	getline(std::cin,s); //gets input
	if(s == "exit") //exit if asked for
		quit = true;
	globalMutex.lock();
	msgSend = s; //otherwise lock and set the message you recieved as the one that is to be sent as a packet
	globalMutex.unlock();
}


int main(int argc, char* argv[])
{
	sf::Thread* thread = 0; //a var for thread
		
	char who;
	std::cout << "Do you want to be a server (s) or a client (c) ? ";
	std::cin  >> who;

	if (who == 's')
		Server();
	else
		Client();

	thread = new sf::Thread(&DoStuff); //makes a thread run that function
	thread->launch(); //launches the thread, constantly looping
		
	while(!quit)
	{
		GetInput(); //gets this stuff in loop on main thread, at the same time as the other thread
	}

	if(thread) //it is initially set to 0, so if a new thread isn't made, it is 0, so false, so doesn't need to be deleted
	{
		thread->wait(); //wait until the current bit of code the thread is going through finishes
		delete thread; //then delete the thread
	}
	return 0;
}
