//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "SteeringAgent.h"

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
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

//WANDER (base> SEEK)
//******
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
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

//Flee
//****
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = pAgent->GetPosition() - m_Target.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	//Debug rendering
	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0.f, 1.f, 0.f, 0.5f }, 0.4f);

	return steering;
}

//Arrive
//****
SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	float VectorLength = steering.LinearVelocity.Magnitude();
	if (VectorLength > m_SlowRadius && VectorLength > m_ArrivalRadius) {
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	}
	else if (VectorLength > m_ArrivalRadius){
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= ((VectorLength / m_SlowRadius) * pAgent->GetMaxLinearSpeed());
	}
	else {
		steering.LinearVelocity *= 0;
	}
	//Debug rendering
	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0.f, 1.f, 0.f, 0.5f }, 0.4f);

	return steering;
}

//FACE
//****
SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	pAgent->SetAutoOrient(false);

	//Create a vector between the target and the agent position 
	Elite::Vector2 targetDirection = m_Target.Position - pAgent->GetPosition();
	//Normalize the vector to use it as a direction instead of a velocity
	targetDirection.Normalize();
	
	//Find the angle of the target using atan2 to compensate if target is in 4th quadrant
	float AngleofTarget = atan2(targetDirection.y, targetDirection.x);
	AngleofTarget += float(M_PI / 2.f);
	
	//Calculate the needed change of angle
	float currentAngle = pAgent->GetRotation();
	float rotation = AngleofTarget - currentAngle;
	//Subtract 360° if the angle is greater than 180° to stop agent from turning the wrong direction
	while (rotation > float(M_PI))
		rotation -= float(2.f * M_PI);
	while (rotation < -float(M_PI))
		rotation += float(2.f * M_PI);

	//Limit the turning speed tot he maximing speed of the agent
	float maxSpeed = pAgent->GetMaxAngularSpeed();
	steering.AngularVelocity = Clamp(ToDegrees(rotation), -maxSpeed, maxSpeed);

	if (pAgent->CanRenderBehavior())
	{
		Elite::Vector2 pos = pAgent->GetPosition();
		DEBUGRENDERER2D->DrawSegment(pos, m_Target.Position, { 0,0,1,0.5f }, 0.4f);
		std::cout << "target angle: " << round(ToDegrees(AngleofTarget)) << "   agent angle: " << round(ToDegrees(currentAngle))
			<< "   needed rotation: " << round(ToDegrees(rotation)) << "                                                  \r";
	}

	return steering;
}

//Evade
//****
SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	auto distanceToTarget = Distance(pAgent->GetPosition(), m_Target.Position);

	if (distanceToTarget > m_EvadeRadius) {
		steering.IsValid = false;
		return steering;
	}

	steering.LinearVelocity = pAgent->GetPosition() - m_Target.Position;
	steering.LinearVelocity += m_Target.LinearVelocity;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	//Debug rendering
	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0.f, 1.f, 0.f, 0.5f }, 0.4f);

	return steering;
}

//Pursuit
//****
SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity += m_Target.LinearVelocity;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	//Debug rendering
	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0.f, 1.f, 0.f, 0.5f }, 0.4f);
		DEBUGRENDERER2D->DrawSolidCircle(pAgent->GetPosition() + steering.LinearVelocity, 0.5f, { 0, 0 }, { 1.f, 0.f, 0.f, 0.5f }, 0.2f);
	return steering;
}