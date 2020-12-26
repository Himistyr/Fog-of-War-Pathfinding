/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// StatesAndTransitions.h: Implementation of the state/transition classes
/*=============================================================================*/
#ifndef ELITE_APPLICATION_FSM_STATES_TRANSITIONS
#define ELITE_APPLICATION_FSM_STATES_TRANSITIONS

#include "../Shared/Agario/AgarioAgent.h"
#include "../Shared/Agario/AgarioFood.h"
#include "../App_Steering/SteeringBehaviors.h"

//AGAIO AGENT STATE
//-----------------------
class WanderState : public Elite::FSMState
{
public:
	WanderState() : FSMState() {};
	virtual void OnEnter(Blackboard* pB) {
		// Agent opvragen uit blackboard
		AgarioAgent* pAgent = nullptr;
		bool dataAvailabe = pB->GetData("Agent", pAgent);

		if (!dataAvailabe)
			return;

		//Agent wandering steering behavior actief zetten
		pAgent->SetToWander();
	}
};

class SeekFoodState : public Elite::FSMState
{
public:
	SeekFoodState() : FSMState() {};
	virtual void OnEnter(Blackboard* pB) {

		// Agent opvragen uit blackboard
		AgarioAgent* pAgent = nullptr;
		AgarioFood* pTarget = nullptr;

		bool dataAvailabe = pB->GetData("Agent", pAgent);;
		dataAvailabe = pB->GetData("FoodTarget", pTarget);

		if (!dataAvailabe)
			return;

		pAgent->SetToSeek(pTarget->GetPosition());
	}
};

class FleeState : public Elite::FSMState
{
public:
	FleeState() : FSMState() {};
	virtual void OnEnter(Blackboard* pB) {

		// Agent opvragen uit blackboard
		m_DataAvailabe = pB->GetData("Agent", m_pAgent);;
		m_DataAvailabe = pB->GetData("AgentTarget", m_pTarget);
	}

	virtual void Update(Blackboard* pB, float deltaTime) override {
		Elite::FSMState::Update(pB, deltaTime);

		if (!m_DataAvailabe)
			return;

		Elite::Vector2 direction = m_pAgent->GetPosition() + (m_pAgent->GetPosition() - m_pTarget->GetPosition());
		m_pAgent->SetToSeek(direction);
	}
private:
	AgarioAgent* m_pAgent = nullptr;
	AgarioAgent* m_pTarget = nullptr;
	bool m_DataAvailabe;
};

class ChaseState : public Elite::FSMState
{
public:
	ChaseState() : FSMState() {};
	virtual void OnEnter(Blackboard* pB) {

		// Agent opvragen uit blackboard
		m_DataAvailabe = pB->GetData("Agent", m_pAgent);;
		m_DataAvailabe = pB->GetData("AgentTarget", m_pTarget);
	}

	virtual void Update(Blackboard* pB, float deltaTime) override {
		Elite::FSMState::Update(pB, deltaTime);

		if (!m_DataAvailabe)
			return;

		m_pAgent->SetToSeek(m_pTarget->GetPosition());
	}
private:
	AgarioAgent* m_pAgent = nullptr;
	AgarioAgent* m_pTarget = nullptr;
	bool m_DataAvailabe;
};

//AFARIO AGENT TRANSITIONS
//------------------------
	//Move To Food
class CloseToFood : public Elite::FSMTransition
{
public:
	CloseToFood() : FSMTransition() {};
	virtual bool ToTransition(Blackboard* pB) const override {
		//Is food Close by?
		//return true
		//else return false
		AgarioAgent* pAgent = nullptr;
		std::vector<AgarioFood*>* pFoodVec;
		float pDetectionRange = 0.f;

		bool dataAvailabe = pB->GetData("Agent", pAgent);
		dataAvailabe = pB->GetData("FoodVec", pFoodVec);
		dataAvailabe = pB->GetData("DetectionRange", pDetectionRange);

		if (!dataAvailabe)
			return false;

		bool found{};
		AgarioFood* closest = nullptr;
		float radiusToCheck = pAgent->GetRadius() + pDetectionRange;

		for (AgarioFood* food : *pFoodVec)
			if (Elite::DistanceSquared(pAgent->GetPosition(), food->GetPosition()) <= (radiusToCheck * radiusToCheck)) {
				found = true;
				closest = food;
			}

		if (found) {
			pB->ChangeData("FoodTarget", closest);
			return true;
		}

		return false;
	}
};

//Stop moving towards target food
class FoodEaten : public Elite::FSMTransition
{
public:
	FoodEaten() : FSMTransition() {};
	virtual bool ToTransition(Blackboard* pB) const override {

		AgarioAgent* pAgent = nullptr;
		AgarioFood* pTarget = nullptr;
		std::vector<AgarioFood*>* pFoodVec;

		bool dataAvailabe = pB->GetData("Agent", pAgent);
		dataAvailabe = pB->GetData("FoodTarget", pTarget);
		dataAvailabe = pB->GetData("FoodVec", pFoodVec);

		if (!dataAvailabe)
			return false;

		if (std::find(pFoodVec->begin(), pFoodVec->end(), pTarget) == pFoodVec->end())
			return true;

		return false;
	}
};

//Flee from bigger enemy 
class FleeBigger : public Elite::FSMTransition
{
public:
	FleeBigger() : FSMTransition() {};
	virtual bool ToTransition(Blackboard* pB) const override {

		AgarioAgent* pAgent = nullptr;
		std::vector<AgarioAgent*>* pOtherAgents = nullptr;
		std::vector<AgarioFood*>* pFoodVec;
		float pDetectionRange = 0.f;

		bool dataAvailabe = pB->GetData("Agent", pAgent);
		dataAvailabe = pB->GetData("OtherAgents", pOtherAgents);
		dataAvailabe = pB->GetData("FoodVec", pFoodVec);
		dataAvailabe = pB->GetData("DetectionRange", pDetectionRange);

		if (!dataAvailabe)
			return false;

		for (AgarioAgent* otherAgent : *pOtherAgents)
			if (pAgent->GetRadius() <= otherAgent->GetRadius() - 2) {

				float radiusToCheck = pAgent->GetRadius() + otherAgent->GetRadius() + pDetectionRange;
				if (Elite::DistanceSquared(pAgent->GetPosition(), otherAgent->GetPosition()) <= (radiusToCheck * radiusToCheck)) {

					pB->ChangeData("AgentTarget", otherAgent);
					return true;
				}
			}

		return false;
	}
};

//Stop Fleeing
class StopFleeing : public Elite::FSMTransition
{
public:
	StopFleeing() : FSMTransition() {};
	virtual bool ToTransition(Blackboard* pB) const override {
		FleeBigger fb{};
		return !fb.ToTransition(pB);
	}
};

//Chase Smaller
class ChaseSmaller : public Elite::FSMTransition
{
public:
	ChaseSmaller() : FSMTransition() {};
	virtual bool ToTransition(Blackboard* pB) const override {

		AgarioAgent* pAgent = nullptr;
		std::vector<AgarioAgent*>* pOtherAgents = nullptr;
		float pDetectionRange = 0.f;

		bool dataAvailabe = pB->GetData("Agent", pAgent);
		dataAvailabe = pB->GetData("OtherAgents", pOtherAgents);
		dataAvailabe = pB->GetData("DetectionRange", pDetectionRange);

		if (!dataAvailabe)
			return false;

		for (AgarioAgent* otherAgent : *pOtherAgents)
			if (pAgent->GetRadius() - 2 >= otherAgent->GetRadius()) {

				float radiusToCheck = pAgent->GetRadius() + otherAgent->GetRadius() + pDetectionRange;
				if (Elite::DistanceSquared(pAgent->GetPosition(), otherAgent->GetPosition()) <= (radiusToCheck * radiusToCheck)) {

					pB->ChangeData("AgentTarget", otherAgent);
					return true;
				}
			}

		return false;
	}
};

//Stop Chasing
class StopChasing : public Elite::FSMTransition
{
public:
	StopChasing() : FSMTransition() {};
	virtual bool ToTransition(Blackboard* pB) const override {

		AgarioAgent* pAgent = nullptr;
		AgarioAgent* pTarget = nullptr;
		std::vector<AgarioAgent*>* pOtherAgents = nullptr;
		float pDetectionRange = 0.f;

		bool dataAvailabe = pB->GetData("Agent", pAgent);
		dataAvailabe = pB->GetData("AgentTarget", pTarget);
		dataAvailabe = pB->GetData("OtherAgents", pOtherAgents);
		dataAvailabe = pB->GetData("DetectionRange", pDetectionRange);

		if (!dataAvailabe)
			return false;

		if (std::find(pOtherAgents->begin(), pOtherAgents->end(), pTarget) == pOtherAgents->end())
			return true;

		if (pAgent->GetRadius() <= pTarget->GetRadius() - 1)
			return true;

		float ExtraChaseRange = 5.f;
		float radiusToCheck = pAgent->GetRadius() + pTarget->GetRadius() + pDetectionRange;
		if (Elite::DistanceSquared(pAgent->GetPosition(), pTarget->GetPosition()) > (radiusToCheck * radiusToCheck))
			return true;

		return false;
	}
};
#endif