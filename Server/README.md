# Server Changes
## Server v0.0
I've added in fixes for the following:
-All the code in game.cpp
    -All 4 functions now run on separate threads, all use mutexs for synchronisation
    -Performance is actually better now due to this
    -Chunks are now generated every 0.1s, which means users will occasionally see chunks being generated, but not a huge issue
    -Need to find a better method than this (better code in general is needed)
-The code in updateActiveChunkData.cpp
    -The chunks now generate with their own mobs
    -There is a new flag in the chunkData (chunks[coordinate].first.generated), which indicates whether or not a chunk has been generated
        -If set to true, it's run through this subroutine otherwise it's accidentally come into existence likely due to mobs
        -In which case it is flagged for deletion
    -Overall, roughly 25 chunks are maintained at all times, which is not an issue
-itemSystems.cpp contains a bunch of the item related code that was previously in physics.cpp
-mobSystem.cpp currently just contains the function for generating mobs, called by the generateChunks(...) function in updateActiveChunkData.cpp
-I've updated some constants in ecs.h to make the overall gameplay feel smoother
-Collision detection for platforms now works the same as for anything else (coordinates + local coordinates for collision box vertices)

## Server v0.1
I've added in movement for the mobs, they currently more or less randomly move side to side, and that is reflected on the client side.
I've removed mutexs from places and placed them elsewhere to make sure I'm locking for less time, and have increased the overall performance of the program by rewriting part of gameBroadcast.cpp (it was checking every entity in every chunk to send data)

## Server v0.2
I've prevented mobs from spawning in cities, and they also can't move into cities, so later on the cities themselves will be safe spaces.
I've purposely made it so that if mobs collide with cities, their x-velocity reverses, but not their direction (as if they were backing off).
I had more mutex related issues I believe, so I've temporarily moved a mutex back into game.cpp, but still saw an error - so I'll have to be on the lookout for errors around physics::moveEntities() and mutexs.

# Errors and Stuff
Ok so basically you might get some errors about hashtable stuff or whatever when using unordered stl containers, so basically to resolve this you need to make your custom type have some equality testing functionality, for example, making an overload of the == operator, and then also need to make a custom hashing function, which should be easily achievable by using a unique id or something on your struct, and just returning that for the hash function.