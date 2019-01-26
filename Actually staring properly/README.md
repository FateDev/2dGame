Sat 26th Jan:
Login system and registration system for the game has been completed.
The errors I came accross were mostly just me forgetting what I was thinking about last night, for example, not updating the varaible in the struct 'user' "loggedIn" to true when a person is logged in so that they can actually talk, and another one where I was making the client computer wait a few seconds for the server to process the credidentials they provided, which caused their connection to time out and be suspended, because the client side pinging of the server (which is supposed to happen in a range t seconds, where 3 seconds <= t < 5 seconds), was paused by the part of code waiting for a server response. There were no notable errors other than this.

I've basically constructed an in game messaging system, and it's quit flexible luckily, so I'll just add a GUI to the client (probably not the server though), and make the in-game chat in a drop-down or something.
