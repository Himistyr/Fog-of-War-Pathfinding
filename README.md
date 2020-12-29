# Fog of War Pathfinding
How I believe Fog of War and Pathfinding should be handled.

## The goal of this project
It always intrigued me how pathfinding worked for games that use fog of war, since the player doesn't know what is going on outside his/her field of view. This made me want to figure out how some of the already existing solutions for this problem work and what my own take on this would be.  
So, I set out to do exactly that.  
In this project, I will do some research on how, and potentially why, developers handled fog of war pathfinding and use this information to create my own implementation of this. My workflow for this implementation will also be documented right here for you to enjoy!

## How do already existing games handle this?
To start off, I believe it is only natural that I do some research on how already existing solutions for this work. I decided to just look up some of the more well-known games that and see what they did.  
The first example I found was Supreme Commander 2. They decided that using a flowfield algorithm was the best option for their game. So, what is a flowfield? A flowfield is, explained in a very simple way, a grid that has a direction for every tile that updates whenever the destination is changed. When an Actor wants to know where to go to reach their destination, they just need to follow the direction of the tile they are currently on. This flowfield has a few extra implementations though. For example, whenever a path is generated, every tile that has a direct line of sight to the destination will color blue and point directly towards the destination. If the tile does not have a direct line of sight, it takes 1 of 12 basic directions. Also, whenever multiple units are selected, the units will move in formation. If the formation cannot stay in the main formation in the destination area, units outside of the destination area will cram up against the edges.  

![SupremeCommander2FlowField](https://github.com/Himistyr/Fog-Of-War-Pathfinding/blob/master/Images/Explanation/SupremeCommander2FlowField.png "SupremeCommander2FlowField")

When it comes to something like StarCraft, a more basic version of A* seems to be used. The world is split up into a giant grid which is used to calculate the most efficient path towards the destination. If the unit's path gets obstructed by other units, he waits a set amount of time and checks if he can continue. If this check fails to many times, a new path is generated. Since the world in StarCraft is constantly changing by the players building their bases, it is very much possible for the path to become obstructed aswell. When this happens, a helper function triggers and sees the path no longer exists out of walkable terrain. This also triggers a new path calculation.  
  
So how about StarCraft 2? In this iteration, the developers made the units rely way less on the A* algorithm and allow way more flexibility with steering behaviours. Since the units are no longer bound by the worldgrid, they could make small movements to the sides to walk past other units. This made it possible to cram way more units trough chokepoints instead of having a lot of units wait until they could continue. Although some people hated this due to chokepoints becoming way more dangerous because of the units being packed together.  
  
The final one I want to talk about is Command & Conquer: Tiberian Sun. The developers of this game where known to have "solved" pathfinding. They also made a mini-documentary explaining what their tought process was behind the making of this algorithm. Basically, they say that the most important task of making a pathfinding algorithm is making sure it does not do anything stupid. It does not have to be perfect, just make sure that the decisions it makes seem logical to the user. For example, if you let a unit move inside a 4 wall compound, make sure it does not run around it before entering.

(information found at following links)
* Supreme Commander 2: https://wildfiregames.com/forum/topic/16018-supreme-commander-2-pathfinding/
* StarCraft 1 : http://striketactics.net/devblog/starcraft-1-pathfinding-technical-analysis
* StarCraft 1 & 2: https://tl.net/forum/starcraft-2/132171-the-mechanics-of-sc2-part-1
* Command & Conquer: Tiberian Sun : https://www.youtube.com/watch?v=S-VAL7Epn3o&ab_channel=ArsTechnica

## So, what is my view on the "correct" implementation?
Maybe you have noticed it, but none of the mentioned examples actually took field of view in mind. The units always know how to get from point A to B, no exceptions. When their path gets interupted they either stop moving, make a path trough interactions or just calculate a new path. So, I am left with the question, how does an AI work that can only use what he sees and remembers? I will try to create a pathfinding algortihm that does exactly that and see for myself if this is a feasible option. Of course, I will document my progress and mention any problems I find with this kind of pathfinding down below.

## My idea put into practice
### The framework I will use 
To make this idea an actual project I can show off, I will need to program it of course _**Duh**_. Since, at the time of making of this project, I am still studying at the university of Howest, I already have a handy implementation of A* to utilise. This is a simple grid that currently calculates the most optimal way to reach the red point, starting from the out-most green point.

![A*\_Grid](https://github.com/Himistyr/Fog-Of-War-Pathfinding/blob/master/Images/Explanation/Explanation1.png "Explanation1")

Of course, it is can do more than just this. It is also possible to change the terrain type, changing up the weight values of each tile, to influence the way A* calculates the path. Also, whenever a weight value of 200000+ is found, this is automatically marked as an unpassable Node. In its current form the tiles can be one of 3 options:
 * Ground, value of 1
 * Mud, value of 2
 * Water, value of 200001 (can not be walked over)
 <!-- end list -->
To give you an idea of how this can be used in practice, I have made the following scenario in the app:

![A*\_Grid2](https://github.com/Himistyr/Fog-Of-War-Pathfinding/blob/master/Images/Explanation/Explanation2.png "Explanation2")

As you can see, the calculated path is within the boundaries of the water, yet it takes the path that avoids the mud. The algorithm decides this based on the weight values presented before (normal ground has a lower weight than mud). Now, we can influence the path it will take by adding some extra mud, or a single block of water, to the upper path. This would increase the weight of the upper path by a high enough margin to make the lower path a faster option. By doing this, we have successfully convinced the algorithm to take the lower path. An example of this:

![A*\_Grid3](https://github.com/Himistyr/Fog-Of-War-Pathfinding/blob/master/Images/Explanation/Explanation3.png "Explanation3")

Now that I have explained how the base framework works, I can start going trough everything I've done to implement my idea. The final product should function almost completely different as I will need to add characters that actually traverse this grid using the calculated path. This is going to be fun!

### Implementation of the idea
#### Adding an Actor to represent a unit 
Since I am working with the idea of moving characters around, I will need some code to represent this first. Luckily, the framework I am using has a basic implementation of this called Actors. Actors are simple pieces of code that can be given custom-made movement algorithms to make them do exactly what you want them to do. I will be using a simple Seek behaviour, as you can see in the following code-snippet.

```c++
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override
	{
		SteeringOutput steering{};
		
		steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
		
		//Debug rendering
		if (pAgent->CanRenderBehavior())
			DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0.f, 1.f, 0.f, 0.5f }, 0.4f);

		return steering;
	}
};
```

Basically, Seek just tells the actor to move to a certain target in 1 straight line (The target being anything from a position to another actor). Now that this is established, let us move on to actually using this Seek behaviour. To simplify the code, I just let the actor move to the first point the algorithm found that isn't the current point the agent is at. Also, I check if the path is still more than 2 nodes long. In case this is false, the agent just moves towards the final node. An example of this:
```c++
if (m_vPath.size() > 2)
		m_pSeekBehavior->SetTarget(TargetData{ m_pGridGraph->GetNodeWorldPos(m_vPath[1]) });
	else 
		m_pSeekBehavior->SetTarget(TargetData{ m_pGridGraph->GetNodeWorldPos(endPathIdx) });
```
Okay, so now that we understand the basic premise of how I get this to work, here is a very simple example of this in action:

![See You Soon.gif](https://github.com/Himistyr/Fog-Of-War-Pathfinding/blob/master/Images/ProgressGifs/FirstAgentAdded.gif "FirstAgentAdded")

#### Seperating the world and the Actor's view
Right now, every Actor knows exactly what the world looks like at all times. So, let us change that!  
Currently, the world exists out of 1 big grid, which is also used to calculate every path for every agent. To change this we just need to, in theory, add an extra grid per agent or per team (a team being multiple agents sharing the same view).
```c++
using WorldGrid = Elite::GridGraph<Elite::GridTerrainNode, Elite::GraphConnection>;
WorldGrid* m_pGridGraph;
WorldGrid* m_pAgentView;

void App_PathfindingAStar::MakeGridGraph()
{
	m_pGridGraph = new WorldGrid(COLUMNS, ROWS, m_SizeCell, false, true, 1.f, 1.5f);
	m_pAgentView = new WorldGrid(COLUMNS, ROWS, m_SizeCell, false, true, 1.f, 1.5f);
}

```
Of course, this is far from everything we need to do. Since I want the agents to have a certain range of view, we need to update the agent his view with only the nodes that are currently withing the agents field of view. Originally, because every Node is connected to his surrounding Nodes trough a Connection, I just looped over every Connection and checked if the TerrainType of the world matched the Actor's view.  
Yeah, that did not work.  
I forgot that the water TerrainType gets disconnected from the other Nodes as a simple way to prevent A* from using it as a possible path. So, how did I fix this? I manually checked the surrounding Nodes as long as they where within the field of view and still within the world grid.
```c++
bool App_PathfindingAStar::CheckTerrainInRadius(WorldGrid* world, WorldGrid* actorView, int startNodeIndex, int stepsTaken)
{
	//if the startNode is outside of the world-grid, stop looking in this direction.
	if (startNodeIndex >= world->GetColumns() * world->GetRows() || startNodeIndex < 0)
		return false;

	//Make the node render as "InVision"
	actorView->GetNode(startNodeIndex)->SetIsVisible(true);

	//If a Node with a different TerrainType than our agent remembers is found,
	//update our actors memory to remember this new TerrainType.
	bool newTerrainFound = (world->GetNode(startNodeIndex)->GetTerrainType() != actorView->GetNode(startNodeIndex)->GetTerrainType());
	if (newTerrainFound)
	{
		actorView->GetNode(startNodeIndex)->SetTerrainType(world->GetNode(startNodeIndex)->GetTerrainType());
		UpdateNode(actorView, startNodeIndex);
	}
		
	//If we still have not reached our ViewRadius,
	//check all Nodes around the current Node.
	if (stepsTaken + 1 <= m_ViewRadius) {
		
		//Usage of connections is not possible due to unpassable terrain getting disconnected from the graph,
		//blocking vision for no reason at all.
		bool neighborUp{}, neighborDown{}, neighborLeft{}, neighborRight{};
		int rowCheck = startNodeIndex / world->GetColumns();
		
		neighborUp = CheckTerrainInRadius(world, actorView, startNodeIndex + world->GetColumns(), stepsTaken + 1);
		neighborDown = CheckTerrainInRadius(world, actorView, startNodeIndex - world->GetColumns(), stepsTaken + 1);

		//Because our Grid is a 1D-array used as a 2D array,
		//we need to check if the next index isn't the start of a new row
		if ((startNodeIndex - 1) / world->GetColumns() == rowCheck)
			neighborLeft = CheckTerrainInRadius(world, actorView, startNodeIndex - 1, stepsTaken + 1);
		if ((startNodeIndex + 1) / world->GetColumns() == rowCheck)
			neighborRight = CheckTerrainInRadius(world, actorView, startNodeIndex + 1, stepsTaken + 1);

		//If we haven't found a difference in TerrainType yet,
		//check if the neighboring nodes did have a difference.
		if (!newTerrainFound)
			newTerrainFound = (neighborUp || neighborDown || neighborLeft || neighborRight);
	}

	return newTerrainFound;
}
```
But there was still a problem. Whenever a TerrainType that was unpassable was added to the agent his view, it was still passable. I seemed to have missed a part of the framework that updated the graph whenever you edited terrain with a mouseclick. So I wrote a tiny little function that replicated this behaviour for a given Nodes in a given Grid.
```c++
void App_PathfindingAStar::UpdateNode(WorldGrid* pGraph, int idx)
{
	//If terrain is unpassable, set it here
	if (idx != invalid_node_index)
	{
		if (int(pGraph->GetNode(idx)->GetTerrainType()) > 200000)
			pGraph->IsolateNode(idx);
		else
			pGraph->UnIsolateNode(idx);
	}
}
```
And that is it! I now have a simple functioning implementation of pathfinding with fog of war. So, let me show you how it looks in action. In the following demo I had an agent with a field of view of 3 Nodes, meaning he can see up to 3 steps away from his current Node.

![AgentFieldOfView.gif](https://github.com/Himistyr/Fog-of-War-Pathfinding/blob/master/Images/ProgressGifs/AgentFieldOfView.gif "AgentFieldOfView")

#### But I found a flaw
When testing some scenarios, I stumbled upon a mayor problem that could be the main reason this type of pathfinding is not used by most games. Because the Actors remember the terrain they have already seen, it was possible for them to think there was no way to reach their destination even though there was one. Now, you could say this is fine as is, but for some games this would be frustrating and limmit the player. To prevent this from happening, I changed up the code to do the following:  
If the Actor is unable to find a path, ask the world if it is possible to reach our desired destination.  
If the world tells us a path exists, give our player a hint in which direction he needs to go to find the closest path.  
If the world can not find a path, don't move.

```c++
//CALCULATEPATH
//If we should find a new path and the start and end points are within the world, find a path!
if (m_UpdatePath
	&& startPathIdx != invalid_node_index
	&& endPathIdx != invalid_node_index)
{
	//Reset variables
	m_RenderPathAsHint = false;

	//AStar Pathfinding
	auto pathfinder = AStar<GridTerrainNode, GraphConnection>(m_pAgentView, m_pHeuristicFunction);
	
	auto startNode = m_pAgentView->GetNode(startPathIdx);
	auto endNode = m_pAgentView->GetNode(endPathIdx);

	bool PathFound = false;
	m_vPath = pathfinder.FindPath(startNode, endNode, PathFound);

	if (!PathFound) {

		pathfinder = AStar<GridTerrainNode, GraphConnection>(m_pGridGraph, m_pHeuristicFunction);

		startNode = m_pGridGraph->GetNode(startPathIdx);
		endNode = m_pGridGraph->GetNode(endPathIdx);

		PathFound = false;
		m_vPath = pathfinder.FindPath(startNode, endNode, PathFound);

		if (!PathFound) {
			
			m_vPath = std::vector<Elite::GridTerrainNode*>{ m_pGridGraph->GetNode(startPathIdx), m_pGridGraph->GetNode(startPathIdx) };
		}
		else {

			m_RenderPathAsHint = true;
			m_vPath = std::vector<Elite::GridTerrainNode*>{ m_vPath[0], m_vPath[1] };
		}
	}

	m_UpdatePath = false;
	if (PathFound)
		std::cout << "New Path Calculated" << std::endl;
	else 
		std::cout << "No Path Found" << std::endl;
}
```
And of course, a visual demo of this in action.

![WorldHints.gif](https://github.com/Himistyr/Fog-of-War-Pathfinding/blob/master/Images/ProgressGifs/WorldHints.gif "WorldHints")

#### How about multiple actors?
To make it more interesting I decided to add the option of adding more Agents to the world, all sharing the same view, to simulate units in a game like StarCraft. To do this I had to modify my code a little. Instead of using a single agent, I now use a vector-list of Agents and Behaviours with an index for which Agent we are currently controlling.  
Very basic stuff, as you can see below:
```c++
//New Initialization
std::vector<AgarioAgent*> m_Team;
std::vector<Seek*> m_SeekBehaviours;
std::vector<std::pair<int, int>> m_PathInformation;
std::vector<std::vector<Elite::GridTerrainNode*>> m_Paths;
int m_CurrentIndex = 0;
int m_ViewRadius = 3;

void App_PathfindingAStar::AddAgent()
{
	//Create new Seek for the new agent
	Seek* newSeek = new Seek{};
	newSeek->SetTarget(TargetData{ m_pGridGraph->GetNodeWorldPos(0) });

	m_SeekBehaviours.push_back(newSeek);

	//Create the new agent
	AgarioAgent* newAgent = new AgarioAgent{ Elite::Vector2{} };
	newAgent->SetSteeringBehavior(newSeek);
	newAgent->SetMaxLinearSpeed(20.f);
	newAgent->SetAutoOrient(true);
	newAgent->SetMass(0.1f);
	newAgent->SetBodyColor(Elite::Color{ 0.f, 0.f, 1.f });
	newAgent->SetPosition(m_pGridGraph->GetNodeWorldPos(0));

	//Increases the size of the agent
	newAgent->MarkForUpgrade(3);

	m_Team.push_back(newAgent);

	//Create path information for agent
	m_PathInformation.push_back(std::make_pair<>(0, 0));

	//Create path for agent
	std::vector<Elite::GridTerrainNode*> newPath;

	m_Paths.push_back(newPath);

	m_UpdatePath = true;
}

void App_PathfindingAStar::RemoveAgent()
{
	if (m_Team.size() <= 1)
		return;

	if (m_CurrentIndex == m_Team.size() - 1)
		--m_CurrentIndex;

	SAFE_DELETE(m_SeekBehaviours.back());
	m_SeekBehaviours.pop_back();

	SAFE_DELETE(m_Team.back());
	m_Team.pop_back();

	m_PathInformation.pop_back();

	m_Paths.pop_back();
}
```
This worked as you would expect. Everything withing the field of view of an Agent is visible for every Agent of the same team. Once again, a team being a group of Agents sharing the same world-view.  
Of course we also need to change quite a bit in our Update() function, since we want every agent to be able to continue its pathing even if it is not selected. To achieve this, I have changed it to the following chunk of code:
```c++
void App_PathfindingAStar::Update(float deltaTime)
{
	UNREFERENCED_PARAMETER(deltaTime);

	//INPUT
	//Check if the middle mousebutton was released
	bool const middleMouseReleased = INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eMiddle);
	if (middleMouseReleased)
	{
		//Get information of where the mouse was clicked
		MouseData mouseData = { INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eMiddle) };
		Elite::Vector2 mousePos = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ (float)mouseData.X, (float)mouseData.Y });

		//Find closest node to click pos and set the end node
		int closestNode = m_pGridGraph->GetNodeFromWorldPos(mousePos);

		if (closestNode != invalid_node_index) {

			//Check if we should move the agent or the destination
			if (m_AgentSelected)
			{
				m_Team[m_CurrentIndex]->SetPosition(m_pGridGraph->GetNodeWorldPos(closestNode));
				m_UpdatePath = true;
			}
			else
			{
				m_PathInformation[m_CurrentIndex].second = closestNode;
				m_UpdatePath = true;
			}
		}
	}

	//GRID INPUT + UPDATE ALL AGENTS
	bool UpdateAllAgents = m_UpdatePath || m_GraphEditor.UpdateGraph(m_pGridGraph);

	//IMGUI
	UpdateImGui();

	for (int i = 0; i < m_Team.size(); ++i) {

		//Update every actor if the graph changed
		if (UpdateAllAgents)
			m_UpdatePath = true;
		
		//Set startNode to agents current position
		int agentNode = m_pGridGraph->GetNodeFromWorldPos(m_Team[i]->GetPosition());

		//Check if the agent has entered a new node and if it's a valid node
		if (int(m_pGridGraph->GetNode(agentNode)->GetTerrainType()) <= 200000)
			m_PathInformation[i].first = agentNode;
		if (!m_Paths[i].empty() 
			&& m_PathInformation[i].first != m_Paths[i][0]->GetIndex() 
			&& m_PathInformation[i].first != m_PathInformation[i].second)
			m_UpdatePath = true;
		
		if (CheckTerrainInRadius(m_pGridGraph, m_pAgentView, agentNode))
			m_UpdatePath = true;
		
		//CALCULATEPATH
		//If we should find a new path and the start and end points are within the world, find a path!
		if (m_UpdatePath
			&& m_PathInformation[i].first != invalid_node_index
			&& m_PathInformation[i].second != invalid_node_index)
		{
			//Reset variables
			m_RenderPathAsHint = false;

			//AStar Pathfinding
			auto pathfinder = AStar<GridTerrainNode, GraphConnection>(m_pAgentView, m_pHeuristicFunction);

			auto startNode = m_pAgentView->GetNode(m_PathInformation[i].first);
			auto endNode = m_pAgentView->GetNode(m_PathInformation[i].second);

			bool PathFound = false;
			m_Paths[i] = pathfinder.FindPath(startNode, endNode, PathFound);

			if (!PathFound) {

				pathfinder = AStar<GridTerrainNode, GraphConnection>(m_pGridGraph, m_pHeuristicFunction);

				startNode = m_pGridGraph->GetNode(m_PathInformation[i].first);
				endNode = m_pGridGraph->GetNode(m_PathInformation[i].second);

				PathFound = false;
				m_Paths[i] = pathfinder.FindPath(startNode, endNode, PathFound);

				if (!PathFound) {

					auto startNode = m_pGridGraph->GetNode(m_PathInformation[i].first);
					m_Paths[i] = std::vector<Elite::GridTerrainNode*>{ startNode, startNode };
				}
				else {

					m_RenderPathAsHint = true;
					m_Paths[i] = std::vector<Elite::GridTerrainNode*>{ m_Paths[i][0], m_Paths[i][1] };
				}
			}

			m_UpdatePath = false;
			if (PathFound)
				std::cout << "New Path Calculated" << std::endl;
				//std::cout << "agent " << i << " found a path of size : " << m_Paths[i].first.size() << "\n";
			else
				std::cout << "No Path Found" << std::endl;
		}

		//Check if the path is still an actuall path.
		//If true, move to the next Node on the path.
		//If false, move to the last Node.
		if (m_Paths[i].size() >= 2)
			m_SeekBehaviours[i]->SetTarget(TargetData{ m_pAgentView->GetNodeWorldPos(m_Paths[i][1]) });
		else
			m_SeekBehaviours[i]->SetTarget(TargetData{ m_pAgentView->GetNodeWorldPos(m_PathInformation[i].second) });

		m_Team[i]->Update(deltaTime);
	}
}
```
And once again, that's it! I can now use multiple agents that act as if they share the same view, just like RTS games! If you want to see this in action, a demo of the final product will be shown after the debugging part (right below this).

#### Finally, some debugging tools
So, everything works. Now it is time to talk about the debugging tools I added. Some of them you've already seen in action before, like switching between the world-view and Actor-view, but I have added a few more since. I'll be listing them all here with a little demo showing them off at the end.
* A button to select whether the middle mousebutton moves the selected Actor or the destination.
* A representation of the amount of Agents in the world.
* A button to add an Agent.
* A button to remove an Agent.
* A representation of the current Agent selected.
* A button to select the next Agent.
* A button to select the previous Agent.
* 4 checkboxes to show the world grid and some information.
* 2 checkboxes to show the Actor view and field of view.
* A slider to controll the view radius.
* A dropbox to select the heuristic function used.
 <!-- end list -->

![Debug_Showcase.gif](https://github.com/Himistyr/Fog-Of-War-Pathfinding/blob/master/Images/Debug_Showcase.gif "Debug_Showcase")

## My conclusion
In the end, I have parted from my idea to only use what the AI can see. In an RTS game, the AI getting stuck because he cannot "remember" a possible path being available would make it possible for the player to get stuck trying to reach a certain place of the map. Although, I could see it attracting  for a certain type of audience. People who would enjoy a slower pace RTS that would focus more on exploring the map and using the map to your advantage could be interested in this type of pathfinding. Forcing the player to regularly scout the map to spot changes in the terrain to utilise. A system like this could also encourage ambush playstyles. Building bridges over waterways, not visible to the enemy that doesn't have vision there, to attack from unexpected angles. As I see it, both are a valid option and the limited vision could offer some interesting experiences.
