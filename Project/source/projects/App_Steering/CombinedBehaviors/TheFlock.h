#pragma once
#include "../SteeringHelpers.h"
#include "FlockingSteeringBehaviors.h"
#include "SpacePartitioning.h"

class ISteeringBehavior;
class SteeringAgent;
class BlendedSteering;
class PrioritySteering;

class Flock
{
public:
	Flock(
		int flockSize = 50, 
		float worldSize = 100.f, 
		SteeringAgent* pAgentToEvade = nullptr, 
		bool trimWorld = false);

	~Flock();

	void Update(float deltaT, TargetData mouse);
	void UpdateAndRenderUI();
	void Render(float deltaT);

	void RegisterNeighbors(SteeringAgent* pAgent);
	int GetNrOfNeighbors() const { return m_NrOfNeighbors; }
	const vector<SteeringAgent*>& GetNeighbors() const { return m_Neighbors; }

	Elite::Vector2 GetAverageNeighborPos() const;
	Elite::Vector2 GetAverageNeighborVelocity() const;

private:
	// flock agents
	int m_FlockSize = 0;
	vector<SteeringAgent*> m_Agents;

	// neighborhood agents
	vector<SteeringAgent*> m_Neighbors;
	float m_NeighborhoodRadius = 10.f;
	int m_NrOfNeighbors = 0;

	// evade target

	SteeringAgent* m_pAgentToEvade = nullptr;

	// world info
	bool m_TrimWorld = false;
	float m_WorldSize = 0.f;
	
	// steering Behaviors
	Seek* m_pSeek = nullptr;
	Evade* m_pEvade = nullptr;
	Wander* m_pWander = nullptr;
	float m_EvadeDistance{ 5.f }; //How far ahead does the actor need to predict the target

	Cohesion* m_pCohesion = nullptr;
	Seperation* m_pSeperation = nullptr;
	Velocity* m_pVelocity = nullptr;
	BlendedSteering* m_pBlendedSteering = nullptr;
	PrioritySteering* m_pPrioritySteering = nullptr;

	// Debug
	bool m_VisualizeDebugAgents = true;
	bool m_VisualizeDebugCells = true;

	//Partitioning
	CellSpace* m_CellSpace = nullptr;
	int m_CellRows = 12;
	int m_CellCollums = 12;
	bool m_TogglePartitioning = true;
	std::vector<Elite::Vector2> previousPositions{};

	// private functions
	float* GetWeight(ISteeringBehavior* pBehaviour);
	void DrawDebug();

	Flock(const Flock& other);
	Flock& operator=(const Flock& other);
};