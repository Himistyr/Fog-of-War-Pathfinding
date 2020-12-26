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

class InfluenceState : public Elite::FSMState
{
public:
	using InfluenceGrid = Elite::GridGraph<Elite::InfluenceNode, Elite::GraphConnection>;
	
	InfluenceState() : FSMState() {};
	virtual void OnEnter(Blackboard* pB) {
		// Agent opvragen uit blackboard
		AgarioAgent* pAgent = nullptr;
		Elite::InfluenceMap<InfluenceGrid>* pInfluenceGrid = nullptr;
		bool dataAvailabe = pB->GetData("Agent", pAgent);
		dataAvailabe = pB->GetData("InfluenceMap", pInfluenceGrid);

		if (!dataAvailabe)
			return;

		//auto agentNode = pInfluenceGrid->GetNodeAtWorldPos(pAgent->GetPosition());
		const Elite::InfluenceNode* agentNode = pInfluenceGrid->GetNodeAtWorldPos(pAgent->GetPosition());
		if (agentNode != nullptr) {

			const std::list<GraphConnection*>& connections = pInfluenceGrid->GetConnections(agentNode->GetIndex());

			Elite::InfluenceNode* highestPositiveInfluence = pInfluenceGrid->GetNode((*connections.begin())->GetTo());
			for (auto connection : connections) {

				Elite::InfluenceNode* neighbor = pInfluenceGrid->GetNode(connection->GetTo());
				if (neighbor->GetInfluence() > highestPositiveInfluence->GetInfluence())
					highestPositiveInfluence = neighbor;
			}

			pAgent->SetToSeek(highestPositiveInfluence->GetPosition());
		}
	}
};
#endif