# What I'm going to do
I'm probably going to aim towards a 2D multiplayer pixel RPG game.
To do this I'll need to make use of TCP sockets and Threads, which I have no prior experience of (in C++, what I'll be using).
So at this point in time, I'll be going over the developers documentation, and their examples, getting to grips with both of those concepts.

I will also need to make use of databases, but I've decided I'd rather use plain text or binary files with certain structures which would make it easy to read/write from them.
Later on I'll need to implement audio to the game, as well as graphics for the visual component obviously, so I'll be using software such as GIMP for the visuals and Audacity for the music side of things (likely just sample some interesting tunes from other things, giving full credit of course).

I'll make it work on both Linux and Windows, as I'm using the cross-platform SFML library, from which I'll be using the graphics, network and audio modules to construct the game.

-------------------------------------
I've now gone through and gotten to grips with sockets and threads in SFML.
I'll post the basic syntax and rules below.

```
