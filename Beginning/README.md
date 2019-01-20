I'll be going over the developers documentation, and their examples, getting to grips with both of those concepts.

------------------------------------

I've now gone through and gotten to grips with sockets and threads in SFML.
I'll post the basic syntax and rules below.

Firstly make a single socket which can be used as both client and server if need be:
```cpp
sf::TcpSocket socket;
```
Then you can make a TCP server (a listener) like this:
```cpp
//Then a sever construction in some function
sf::TcpListener listener;
listener.accept(PORT);
listener.accept(socket);
//and then you can do socket.getRemoteAddress() to get it's IP
```
Or you can make a client:
```cpp
if(socket.connect(IPADDRESS, PORT) == sf::Socket::Done){ //literally just connects via ipaddress and the port
  std::cout << "Successfully connected" << std::endl;
}
```
Then you have a bit of a problem, you need to do 2 things at once, get the input and also check if there is any incoming messages/send the messages if one needs to be sent.
Now you could do this in one large loop, but this would not only introduce a slight extra latency in sending/receiving messages, but would also make the code look pretty bad.
So a solution to that is to use an additional thread.

So first you'd make a mutex to lock down parts of the program:
```cpp
sf::Mutex globalMutex; //kinda like a flag, if it already 'locked', then any other part of the program that tries to 'lock' it will pause
```

Then you should construct your function which needs to run on that thread, so let's say the sending/receiving loop:
```cpp
func(void){ //to ensure no arguments can be taken
  static std::string oldMsg; //static variable initialised once and retains it's value, used to prevent multiple of the same message sent consecutively (blocks spam)
  while(program has not quited){
    sf::Packet packetSend; //an sfml packet
    
    globalMutex.lock(); //any other part of the program trying to lock will now pause
    packetSend << msgSend; //this adds the message to the new packet
    globalMutex.unlock(); //unlocks it, so other parts can lock it down now
    
    socket.send(packetSend); //send the packet
    
    std::string msg; //temp var for holding receiving message
    sf::Packet packetReceive; //temp var for holding the receiving packet
    
    socket.receive(packetReceive); //receives a packet, so if there isn't one, will just skip over
    
    if(successfully desposit packet contents into string and string isn't empty and isn't the same as the old one){
     print the message
     ...and make the oldmsg variable equal to this
    }
}
```

And then you can make a function to run in the main program loop:
```cpp
std::string s; //temps tring
std::cout << "\nEnter \"exit\" to quit or message to send: ";
getline(std::cin,s); //gets input
if(s == "exit") //exit if asked for
	quit = true;
globalMutex.lock();
msgSend = s; //otherwise lock and set the message you recieved as the one that is to be sent as a packet
globalMutex.unlock();
```

After this simply make a thread pointer in the main program, and then do the following later on to launch it:
```cpp
thread = new sf::Thread(&DoStuff); //makes a thread run that function
thread->launch(); //launches the thread, constantly looping
```

Then once you want to end the program:
```cpp
if(thread) //it is initially set to 0, so if a new thread isn't made, it is 0, so false, so doesn't need to be deleted
{
	thread->wait(); //wait until the current bit of code the thread is going through finishes
	delete thread; //then delete the thread
}
```
