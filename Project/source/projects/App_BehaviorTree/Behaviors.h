/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"
#include "../Shared/Agario/AgarioAgent.h"
#include "../Shared/Agario/AgarioFood.h"
#include "../App_Steering/SteeringBehaviors.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------

//Check for change of behaviour
//-----------------------------------------------------------------
	//Is Close To Food
bool IsCloseToFood(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	std::vector<AgarioFood*>* pFoodVec = nullptr;

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("FoodVec", pFoodVec);

	if (!pAgent || !pFoodVec)
		return false;

	//TODO: Check for food closeby and set target accordingly
	const float DetectionRange = 20.f;
	const float radiusToCheck{ DetectionRange + pAgent->GetRadius() };
	auto foodIt = std::find_if(pFoodVec->begin(), pFoodVec->end(), [&pAgent, &radiusToCheck](AgarioFood* f) {
		return DistanceSquared(pAgent->GetPosition(), f->GetPosition()) < (radiusToCheck * radiusToCheck);
		});

	if (foodIt != pFoodVec->end()) {
		pBlackboard->ChangeData("Target", (*foodIt)->GetPosition());
		return true;
	}
	
	return false;
}

bool IsBiggerClose(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	std::vector<AgarioAgent*>* pOtherAgents = nullptr;

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("AgentsVec", pOtherAgents);

	if (!pAgent || !pOtherAgents)
		return false;

	//TODO: Check for food closeby and set target accordingly
	const float DetectionRange = 20.f ;
	
	bool found = false;
	float distance = 0.f;
	float closestDistance = FLT_MAX;
	Elite::Vector2 pClosestAgent = {};
	for (AgarioAgent* otherAgent : *pOtherAgents)
		if (pAgent->GetRadius() <= otherAgent->GetRadius() - 2) {

			float radiusToCheck = pAgent->GetRadius() + otherAgent->GetRadius() + DetectionRange;
			if ((distance = Elite::DistanceSquared(pAgent->GetPosition(), otherAgent->GetPosition())) <= (radiusToCheck * radiusToCheck)
				&& distance < closestDistance) {

				found = true;
				closestDistance = distance;
				pClosestAgent = otherAgent->GetPosition();
			}
		}

	if (found) {

		pBlackboard->ChangeData("Target", pClosestAgent);
		return true;
	}

	return false;
}

bool IsSmallerClose(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	std::vector<AgarioAgent*>* pOtherAgents = nullptr;

	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("AgentsVec", pOtherAgents);

	if (!pAgent || !pOtherAgents)
		return false;

	//TODO: Check for food closeby and set target accordingly
	const float DetectionRange = 20.f;

	bool found = false;
	float distance = 0.f;
	float closestDistance = FLT_MAX;
	Elite::Vector2 pClosestAgent = {};
	for (AgarioAgent* otherAgent : *pOtherAgents)
		if (pAgent->GetRadius() - 2 >= otherAgent->GetRadius()) {

			float radiusToCheck = pAgent->GetRadius() + otherAgent->GetRadius() + DetectionRange;
			if ((distance = Elite::DistanceSquared(pAgent->GetPosition(), otherAgent->GetPosition())) <= (radiusToCheck * radiusToCheck)
				&& distance < closestDistance) {

				found = true;
				closestDistance = distance;
				pClosestAgent = otherAgent->GetPosition();
			}
		}

	if (found) {

		pBlackboard->ChangeData("Target", pClosestAgent);
		return true;
	}

	return false;
}

//Behaviour States
//-----------------------------------------------------------------
	//Wander
BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	auto dataAvailable = pBlackboard->GetData("Agent", pAgent);

	if (!pAgent)
		return Failure;

	pAgent->SetToWander();
	//std::cout << "Going to wander\n";

	return Success;
}

	//Seek Food Behaviour
BehaviorState ChangeToSeek(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	Elite::Vector2 seekTarget{};
	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) && pBlackboard->GetData("Target", seekTarget);

	if (!dataAvailable)
		return Failure;
	
	//TODO: Implement Change to seek (Target)
	pAgent->SetToSeek(seekTarget);
	std::cout << "Seeking food\n";

	return Success;
}

	//Flee Behaviour
BehaviorState ChangeToFlee(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	Elite::Vector2 seekTarget{};
	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) && pBlackboard->GetData("Target", seekTarget);

	if (!dataAvailable)
		return Failure;

	//TODO: Implement Change to seek (Target)
	Elite::Vector2 direction = pAgent->GetPosition() + (pAgent->GetPosition() - seekTarget);
	pAgent->SetToSeek(direction);
	std::cout << "Fleeing\n";

	return Success;
}

	//Chase Behaviour
BehaviorState ChangeToChase(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent = nullptr;
	Elite::Vector2 seekTarget{};
	auto dataAvailable = pBlackboard->GetData("Agent", pAgent) && pBlackboard->GetData("Target", seekTarget);

	if (!dataAvailable)
		return Failure;

	//TODO: Implement Change to seek (Target)
	pAgent->SetToSeek(seekTarget);
	std::cout << "Chasing\n";

	return Success;
}

#endif