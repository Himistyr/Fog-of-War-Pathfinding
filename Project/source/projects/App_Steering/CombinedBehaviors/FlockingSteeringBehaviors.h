#pragma once
#include "../SteeringBehaviors.h"

class Flock;

//SEPARATION - FLOCKING
//*********************
class Seperation : public ISteeringBehavior
{
public:
	Seperation(const Flock* flock, float maxDistance);
	virtual ~Seperation() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	const Flock* m_Flock;
	float m_MaxDistance;
};

//COHESION - FLOCKING
//*******************
class Cohesion : public ISteeringBehavior
{
public:
	Cohesion(const Flock* flock);
	virtual ~Cohesion() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent * pAgent) override;

private:
	const Flock* m_Flock;
};

//VELOCITY MATCH - FLOCKING
//************************
class Velocity : public ISteeringBehavior
{
public:
	Velocity(const Flock* flock);
	virtual ~Velocity() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	const Flock* m_Flock;
};