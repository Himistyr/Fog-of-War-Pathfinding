# Fog of War Pathfinding
How I believe Fog of War and Pathfinding should be handled.

## The goal of this project
It always intrigued me how pathfinding worked for games that use fog of war, since the player does not know what is going on outside his/her field of view. This made me want to figure out how some of the already existing solutions for this problem work and what my own take on this would be.  
So, I set out to do exactly that.  
In this project, I will do some research on how, and potentially why, developers handled fog of war pathfinding and use this information to create my own implementation of this. My workflow for this implementation will also be documented right here for you to enjoy! Please note that all code within this file will be pseudocode. All the code is availabe right [here](https://github.com/Himistyr/Fog-of-War-Pathfinding/tree/master/Project/source/projects/App_PathfindingAStar) or by going to Project/source/projects/App_PathfindingAStar.

## How do already existing games handle this?
To start off, I believe it is only natural that I do some research on how already existing solutions for this work. I decided to just look up some of the more well-known games that use a similar kind of pathfinding and see what they did.  
The first example I found was *Supreme Commander 2*. They decided that using a flowfield algorithm was the best option for their game. So, what is a flowfield? A flowfield is, explained in a very simple way, a grid that has a direction for every tile that updates whenever the destination is changed. When an Actor wants to know where to go to reach their destination, they just need to follow the direction of the tile they are currently on. This flowfield has a few extra implementations though. For example, whenever a path is generated, every tile that has a direct line of sight to the destination will color blue and point directly towards the destination. If the tile does not have a direct line of sight, it takes 1 of 12 basic directions. Also, whenever multiple units are selected, the units will move in formation. If the formation cannot stay in the main formation in the destination area, units outside of the destination area will cram up against the edges.  

![SupremeCommander2FlowField](https://github.com/Himistyr/Fog-of-War-Pathfinding/blob/master/Images/OtherGameExamples/SupremeCommander2FlowField.png "SupremeCommander2FlowField")

When it comes to something like *StarCraft*, a more basic version of A* seems to be used. The world is split up into a giant grid which is used to calculate the most efficient path towards the destination. If the unit's path gets obstructed by other units, it waits a set amount of time and checks if he can continue. If this check fails too many times, a new path is generated. Since the world in StarCraft is constantly changing by the players building their bases, it is very much possible for the path to become obstructed as well. When this happens, a helper function triggers and sees the path no longer consists of walkable terrain. This also triggers a new path calculation.  
  
So what about *StarCraft 2*? In this iteration, the developers made the units rely less on the A* algorithm and allow a lot more flexibility with steering behaviours. Since the units are no longer bound by the world grid, they could make small movements to the sides to walk past other units. This made it possible to cram more units through chokepoints instead of having a lot of units wait until they could continue. Although some people hated this because of chokepoints becoming a lot more dangerous since the units would pack together, making them easy targets for area of effect attacks.  
  
The final one I want to talk about is *Command & Conquer: Tiberian Sun*. The developers of this game were known to have "solved" pathfinding. They also made a mini documentary explaining what their thought process was behind the making of this algorithm. Basically, they say that the most important task of making a pathfinding algorithm is making sure it does not do anything stupid. It does not have to be perfect, just ensure that the decisions it makes seem logical to the user. For example, if you let a unit move inside a 4-wall compound, make sure it does not run around it before entering.

(information found at following links)
* [Supreme Commander 2](https://wildfiregames.com/forum/topic/16018-supreme-commander-2-pathfinding/)
* [StarCraft 1](http://striketactics.net/devblog/starcraft-1-pathfinding-technical-analysis)
* [StarCraft 1 & 2](https://tl.net/forum/starcraft-2/132171-the-mechanics-of-sc2-part-1)
* [Command & Conquer: Tiberian Sun](https://www.youtube.com/watch?v=S-VAL7Epn3o&ab_channel=ArsTechnica)

## So, what is my view on the "correct" implementation?
Maybe you have noticed it, but none of the mentioned examples took field of view in mind. The units always know how to get from point A to B, no exceptions. When their path is interrupted, they either stop moving, make a path through interactions, or just calculate a new path. So, I am left with the question, how does an AI work that can only use what he sees and remembers? I will try to create a pathfinding algorithm that does exactly that and see for myself if this is a feasible option. Of course, I will document my progress and mention any problems I find with this kind of pathfinding down below.

## My idea put into practice
### The framework I will use 
To make this idea an actual project I can show off, I will need to program it of course _**Duh**_. Since, at the time of making of this project, I am still studying at the university of Howest, I already have a handy implementation of A* to utilise. This is a simple grid that currently calculates the most optimal way to reach the red point, starting from the out-most green point.

![A*\_Grid](https://github.com/Himistyr/Fog-Of-War-Pathfinding/blob/master/Images/Explanation/Explanation1.png "Explanation1")

Of course, it is can do more than just this. It is also possible to change the terrain type, changing up the weight values of each tile, to influence the way A* calculates the path. Also, whenever a weight value of 200000+ is found, this is automatically marked as an unpassable Node. In its current form the tiles can be one of 3 options:
 * Ground, value of 1
 * Mud, value of 2
 * Water, value of 200001 (cannot be walked over)
 <!-- end list -->
To give you an idea of how this can be used in practice, I have made the following scenario in the app:

![A*\_Grid2](https://github.com/Himistyr/Fog-Of-War-Pathfinding/blob/master/Images/Explanation/Explanation2.png "Explanation2")

As you can see, the calculated path is within the boundaries of the water, yet it takes the path that avoids the mud. The algorithm decides this based on the weight values presented before (normal ground has a lower weight than mud). Now, we can influence the path it will take by adding some extra mud, or a single block of water, to the upper path. This would increase the weight of the upper path by a high enough margin to make the lower path a faster option. By doing this, we have successfully convinced the algorithm to take the lower path. An example of this:

![A*\_Grid3](https://github.com/Himistyr/Fog-Of-War-Pathfinding/blob/master/Images/Explanation/Explanation3.png "Explanation3")

Now that I have explained how the base framework works, I can start going through everything I have done to implement my idea. The final product should function almost completely different as I will need to add characters that traverse this grid using the calculated path. This is going to be fun!

### Implementation of the idea
#### Adding an Agent to represent a unit 
Since I am working with the idea of moving characters around, I will need some code to represent this first. Luckily, the framework I am using has a basic implementation of this called Agents. Agents are simple pieces of code that can be given custom-made movement algorithms to make them do exactly what you want them to do. I will be using a simple Seek behaviour, as you can see in the following code-snippet.

```c++
class Seek : public ISteeringBehaviour
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(deltaT, agent) override
	{
		SteeringOutput steering{};
		
		Calculate movement direction;
		Apply speed to direction;

		if (renderDebug)
			Render debug;

		return steering;
	}
}
```
Basically, Seek just tells the Agent to move to a certain target in 1 straight line (the target being anything from a position to another Agent). Now that this is established, let us move on to using this Seek behaviour. To simplify the code, I just let the Agent move to the first point the algorithm found that is not the current point the Agent is at. Also, I check if the path is still more than 2 Nodes long. In case this is false, the Agent moves towards the final Node. An example of this:
```c++
if (path.size() > 2)
	Move towards the next node in the path;
else
	Move towards destination;
```
Okay, so now that we understand the basic premise of how I get this to work, here is a very simple example of this in action:

![See You Soon.gif](https://github.com/Himistyr/Fog-Of-War-Pathfinding/blob/master/Images/ProgressGifs/FirstAgentAdded.gif "FirstAgentAdded")

#### Separating the world and the Agent's view
Right now, every Agent knows exactly what the world looks like at all times. So, let us change that!  
Currently, the world exists out of 1 big grid, which is also used to calculate every path for every Agent. To change this we just need to, in theory, add an extra grid per Agent or per team (a team being multiple Agents sharing the same view).
```c++
using WorldGrid = Elite::GridGraph<Elite::GridTerrainNode, Elite::GraphConnection>;
WorldGrid* World;
WorldGrid* AgentView;

void App_PathfindingAStar::MakeGridGraph()
{
	Initialize World;
	Initialize AgentView exactly like World;
}
```
Of course, this is far from everything we need to do. Since I want the Agents to have a certain range of view, we need to update the Agent's view with just the Nodes that are currently within the Agents field of view. Originally, because every Node is connected to his surrounding Nodes through a Connection, I looped over every Connection and checked if the TerrainType of the world matched the Agent's view.  
Yeah, that did not work.  
I forgot that the water TerrainType gets disconnected from the other Nodes as a simple way to prevent A* from using it as a possible path. So, how did I fix this? I manually checked the surrounding Nodes as long as they where within the field of view and still within the world grid.
```c++
bool CheckTerrainInRadius(world, actorView, start, steps)
{
	Check if Node is within world grid;

	Mark Node as visible;

	bool newTerrainFound = Check if terrain of world and Agent view are different;
	if (newTerrainFound)
		Update Agent view;
	
	if (steps+ 1 <= viewRadius){
		
		check other neighbors;
		if (no new terrain found already and neighbors found a change)
			newTerrainFound = result of other neighbors;
	}

	return newTerrainFound;
}
```
But there was still a problem. Whenever a TerrainType that was unpassable was added to the Agent's view, it was still passable. I seemed to have missed a part of the framework that updated the graph whenever you edited terrain with a mouse click. So I wrote a tiny little function that replicated this behaviour for a given Node in a given Grid.
```c++
void UpdateNode(world, nodeId)
{
	if (nodeId is within the world grid)
	{
		if (terrain is unpassable)
			disconnect Node from the grid;
		else
			connect node to the grid;
	}
}
```
And that is all! I now have a simple functioning implementation of pathfinding with Fog of War. So, let me show you how it looks in action. In the following demo I had an Agent with a field of view of 3 Nodes, meaning he can see up to 3 steps away from his current Node.

![AgentFieldOfView.gif](https://github.com/Himistyr/Fog-of-War-Pathfinding/blob/master/Images/ProgressGifs/AgentFieldOfView.gif "AgentFieldOfView")

#### However, I found a flaw
When testing some scenarios, I stumbled upon a major problem that could be the main reason why this type of pathfinding is not used by most games. Because the Agents remember the terrain they have already seen, it was possible for them to think there was no way to reach their destination even though there was one. Now, you could say this is fine as is, but for some games this would be frustrating and limit the player. To prevent this from happening, I changed up the code to do the following:  
1. If the Agent is unable to find a path, ask the world if it is possible to reach the desired destination.  
2. If the world tells us a path exists, give the player a hint in which direction he needs to go to find the closest path.  
3. If the world cannot find a path, do not move.
 <!-- end list -->
 Also, I added the option to toggle this on and off.
```c++
void Update(deltaT){

	...
	if (path should be updated
		&& start of path is within the world
		&& end of path is within the world){
		
		Reset variables if necesarry;

		Find a path using the Agent view and A*;
		if (no path found){
			
			if (world hints are allowed){
				
				Find a path using the world and A*;
				if (no path found){
					
					the Agent does not move;
				} else{
					
					render path as hint;
					create a path that guides the Agent towards the correct path, 
					without telling it the exact path;
				}
			}
			
		}
		
		Remove the update flag;
		if (path is found)
			print a "found" message;
		else
			print a "not found" message;
	}
	...
}
```
And of course, a visual demo of this in action.

![WorldHints.gif](https://github.com/Himistyr/Fog-of-War-Pathfinding/blob/master/Images/ProgressGifs/WorldHints.gif "WorldHints")

#### What about multiple Agents?
To make it more interesting I decided to add the option of adding more Agents to the world, all sharing the same view, to simulate a very basic idea of units in a game like *StarCraft*. To do this, I had to modify my code a little. Instead of using a single Agent, I now use a vector-list of Agents and Behaviours with an index for which Agent we are currently controlling.  
Very basic stuff, as you can see below:
```c++
//New Initialization
std::vector<Agent*> team;
std::vector<behaviour*> seekBehaviours;
std::vector<std::pair<int, int>> pathInformation;
std::vector<std::vector<nodes*>> paths;
int currentIndex = 0;
int viewRadius = 3;

void AddAgent(){

	Create new seek behaviour;
	Add it to the vector;

	Create new Agent;
	Add it to the vector;

	Create new pathInformation;
	Add it to the vector;

	Create new empty path;
	Add it to the vector;
}

void RemoveAgent(){
	
	if (team.size() <=1)
		return;

	if (currentIndex = the last possible index)
		--currentIndex;

	delete the last entry of every vector;
}
```
This worked as you would expect. Everything within the field of view of an Agent is visible for every Agent of the same team. Once again, a team being a group of Agents sharing the same world-view.  
Of course we also need to change quite a bit in our Update() function, since we want every Agent to be able to continue its pathing even if it is not selected. To achieve this, I have changed it to the following chunk of code:
```c++
void Update(deltaT)
{
	
	Check for middle mouse button release;
	if (Middle mouse button released){
		Get mouse position on the grid;
		if (Mouse is on the grid){
			
			if (Agent selected)
				Move Agent to selected Node;
			else
				move destination to selected Node;
		}
	}

	bool UpdateAllAgents = check if an update is called externally or, if the grid changed, force and update;

	Update GUI;

	for (every actor){
		
		if (UpdateAllAgents)
			update this Agent;

		Get Agent current Node;
		
		if (Agent current Node is walkable)
			update Agent's start index;
		if (Agent entered a new Node)
			update this Agent;

		if (terrain around the Agent changed)
			update this Agent;

		if (we should update this Agent
			&& agent's start is valid
			&& agent's destination is valid){

			Reset variables if necesarry;

			Find a path using the Agent view and A*;
			if (no path found){
			
				if (world hints are allowed){
				
					Find a path using the world and A*;
					if (no path found){
					
						the Agent does not move;
					} else{
					
						render path as hint;
						create a path that guides the Agent towards the correct path, 
						without telling it the exact path;
					}
				}
			
			}
			
			Remove the update flag;
			if (path is found)
				print a "found" message;
			else
				print a "not found" message;
		}

		if (path of current Agent.size() >= 2)
			move the current Agent towards the next node of itspath;
		else
			move the current Agent towards its destination; 

		Update the current Agent
	}
}
```
And once again, that is all! I can now use multiple Agents that act as if they share the same view, just like RTS games! If you want to see this in action, a demo of the final product will be shown after the debugging part (right below this).

#### Finally, some debugging tools
So, everything works. Now it is time to talk about the debugging tools I added. Some of them you've already seen in action before, like switching between the world-view and Agent-view, but I have added a few more since then. I will be listing them all here with a little demo showing them off at the end.
* A button to select whether the middle mouse button moves the selected Agent or the destination.
* A representation of the number of Agents in the world.
* A button to add an Agent.
* A button to remove an Agent.
* A representation of the current Agent selected.
* A button to select the next Agent.
* A button to select the previous Agent.
* 4 checkboxes to show the world grid and some information.
* 3 checkboxes to show the Agent's view, field of view and toggle world hints.
* A slider to control the view radius.
* A drop box to select the heuristic function used.
 <!-- end list -->

![FinishedShowcase.gif](https://github.com/Himistyr/Fog-of-War-Pathfinding/blob/master/Images/ProgressGifs/FinishedShowcase.gif "FinishedShowcase")

## My conclusion
In the end, I have parted from my idea to only use what the AI can see. In an RTS game, the AI getting stuck because he cannot "remember" a possible path being available would make it possible for the player to get stuck trying to reach a certain place of the map. Although, I could see it being attractive for a certain type of audience. People who would enjoy a slower pace RTS, that would focus more on exploring the map and using the map to your advantage, could be interested in this type of pathfinding, forcing the player to regularly scout the map to spot changes in the terrain to utilise. A system like this could also encourage ambush playstyles. Building bridges over waterways, invisible to the enemy that does not have vision there, to attack from unexpected angles. As I see it, both are valid options, and the limited vision could offer some interesting experiences.
