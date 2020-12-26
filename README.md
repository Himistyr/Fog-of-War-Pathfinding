# Fog Of War Pathfinding
How I believe Fog Of War and Pathfinding should be handled.

## The goal of this project
It always intrigued me how pathfinding worked for games that use Fog Of War, since the player doesn't know whats going on outside his/her field of view.
This made me want to figure out how some of the already existing solutions for this problem work and what my own take on this would be. 
So I set out to do exactly that.
In this project, I will do some research on how, and potentially why, developers handled Fog Of War pathfinding and use this information to create my own implementation of this.
My workflow for this implementation will also be documented right here for you to enjoy!

## How do already existing games handle this?
*To be added...*

## So, what is my view on the "correct" implementation?
*To be added...*

## My idea put into practise
### The framework I will use
To make this idea an actual project I can show off, I will need to program it ofcourse _**Duh**_.
Since, at the time of making of this project, I'm still studying at the university of Howest, I already have a handy implementation of A* to utilise.
This is a simple grid that currently calculates the most optimal way to reach the red point, starting from the out-most green point.

![A*\_Grid](https://github.com/Himistyr/Fog-Of-War-Pathfinding/blob/master/Images/Explanation/Explanation1.png "Explanation1")

Ofcourse, it is capable of doing more than just this. 
It is also possible to change the terrain type, changing up the weight values of each tile, to influence the way A* calculates the path.
In its current form the tiles can be one of 3 options:
 * Ground, value of 1
 * Mud, value of 2
 * Water, not passable at all
 <!-- end list -->
To give you an idea of how this can be used in practise, I have made the following scenario in the app:

![A*\_Grid2](https://github.com/Himistyr/Fog-Of-War-Pathfinding/blob/master/Images/Explanation/Explanation2.png "Explanation2")

As you can see, the calculated path is within the boundaries of the water yet it takes the path that avoids the mud.
The algorithm decides this based on the weight values presented before (normal ground has a lower weight than mud).
Now, we can influence the path it will take by adding some extra mud, or a single block of water, to the upper path.
This would increase the weight of the upper path by a high enough margin to make the lower path a faster option.
By doing this, we have succesfully convinced the algorithm to take the lower path.
An example of this:

![A*\_Grid3](https://github.com/Himistyr/Fog-Of-War-Pathfinding/blob/master/Images/Explanation/Explanation3.png "Explanation3")

Now that I've explained how the base framework works, I can start going trough everything I've done to implement my idea.
The final product should function almost completely different as I will need to add characters that actually traverse this grid using the calculated path.
This is going to be fun!

### Implementation of the idea
*To be added...*

![See You Soon.gif](https://github.com/Himistyr/Fog-Of-War-Pathfinding/blob/master/Images/SeeYouSoon.gif "See You Soon")
