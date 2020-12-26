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
#### Adding an Actor to represent a unit
Since I'm working with the idea of moving characters around, I'll need some code to represent this first.
Luckily, the framework I'm using has a basic implementation of this called Actors.
Actors are simple pieces of code that can be given custom-made movement algortihms to make them do exactly what you want them to do.
I'll be using a simple Seek behaviour, as you can see in the following code-snippet.

```c++
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override{
	SteeringOutput steering{};
	Elite::Vector2 circleCenter{};
	const Elite::Vector2 circleOffset{ pAgent->GetDirection() * m_OffsetDistance };
	circleCenter = pAgent->GetPosition() + circleOffset;

	m_WanderAngle += randomFloat() * m_MaxAngleChange - m_MaxAngleChange * 0.5f;
	const Elite::Vector2 randomPointOnCircle = { cos(m_WanderAngle) * m_CircleRadius, sin(m_WanderAngle) * m_CircleRadius };

	m_Target = TargetData(randomPointOnCircle + circleCenter);

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	//Debug rendering
	if (pAgent->CanRenderBehavior()) {
		//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0.f, 1.f, 0.f, 0.5f }, 0.4f);
		DEBUGRENDERER2D->DrawSegment(pAgent->GetPosition(), pAgent->GetPosition() + circleOffset, { 0.f, 0.f, 1.f, 0.5f }, 0.4f);
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition() + circleOffset, m_CircleRadius, { 0.f, 1.f, 0.f, 0.5f }, 0.3f);
		DEBUGRENDERER2D->DrawSolidCircle(pAgent->GetPosition() + circleOffset + randomPointOnCircle, 0.5f, { 0, 0 }, { 1.f, 0.f, 0.f, 0.5f }, 0.2f);
	}
	return steering;
}
};
```

Basically, Seek just tells the actor to move te a certain target in 1 straight line (The target being anything from a position to another actor).
Now that this is established, let's move on to actually using this Seek behaviour.
To simplify the code I just let the actor move to the first point the algorithm found that isn't the current point the agent is at.
Also, I check if the path is still more than 2 nodes long.
In case this is false, the agent just moves towards the final node.
An example of this:
```c++
if (m_vPath.size() > 2)
		m_pSeekBehavior->SetTarget(TargetData{ m_pGridGraph->GetNodeWorldPos(m_vPath[1]) });
	else 
		m_pSeekBehavior->SetTarget(TargetData{ m_pGridGraph->GetNodeWorldPos(endPathIdx) });
```
Okay, so now that we understand the basic premise of how I get this to work, here is very simple example of this in action:

![See You Soon.gif](https://github.com/Himistyr/Fog-Of-War-Pathfinding/blob/master/Images/ProgressGifs/FirstAgentAdded.gif "FirstAgentAdded")

![See You Soon.gif](https://github.com/Himistyr/Fog-Of-War-Pathfinding/blob/master/Images/SeeYouSoon.gif "See You Soon")
