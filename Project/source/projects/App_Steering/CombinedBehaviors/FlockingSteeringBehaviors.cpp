#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "TheFlock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"

//*********************
//SEPARATION (FLOCKING)
Seperation::Seperation(const Flock* flock, float maxDistance)
	: m_Flock{ flock }
	, m_MaxDistance{ maxDistance }
{
}

SteeringOutput Seperation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	/*Cohesion cohesion{m_Flock};
	SteeringOutput steering{};
	steering.LinearVelocity = -cohesion.CalculateSteering(deltaT, pAgent).LinearVelocity;*/

	SteeringOutput steering{};
	Elite::Vector2 velocity{};

	for (int i = 0; i < m_Flock->GetNrOfNeighbors(); ++i) {
		velocity += Elite::GetNormalized(pAgent->GetPosition() - m_Flock->GetNeighbors()[i]->GetPosition()) * (m_MaxDistance - Elite::Distance(pAgent->GetPosition(), m_Flock->GetNeighbors()[i]->GetPosition()));
	}
	steering.LinearVelocity = velocity;
	steering.LinearVelocity.Clamp(pAgent->GetMaxLinearSpeed());

	//Debug rendering
	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0.f, 1.f, 0.f, 0.5f }, 0.4f);

	return steering;
}

//*******************
//COHESION (FLOCKING)
Cohesion::Cohesion(const Flock* flock)
	: m_Flock{flock}
{
}


SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	m_Target = TargetData{ m_Flock->GetAverageNeighborPos() };

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	//Debug rendering
	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0.f, 1.f, 0.f, 0.5f }, 0.4f);

	return steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)
Velocity::Velocity(const Flock* flock)
	: m_Flock{ flock }
{
}

SteeringOutput Velocity::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = m_Flock->GetAverageNeighborVelocity();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	//Debug rendering
	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0.f, 1.f, 0.f, 0.5f }, 0.4f);

	return steering;
}
