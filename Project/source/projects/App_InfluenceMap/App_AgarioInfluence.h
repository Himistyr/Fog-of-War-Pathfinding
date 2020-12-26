#ifndef AGARIO_GAME_APPLICATION_H
#define AGARIO_GAME_APPLICATION_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"
#include "framework\EliteAI\EliteGraphs\EInfluenceMap.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphRenderer.h"

class AgarioFood;
class AgarioAgent;
class AgarioContactListener;

class App_AgarioInfluence final : public IApp
{
public:
	App_AgarioInfluence();
	~App_AgarioInfluence();

	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

	using InfluenceGrid = Elite::GridGraph<Elite::InfluenceNode, Elite::GraphConnection>;
private:
	float m_TrimWorldSize = 70.f;
	const int m_AmountOfAgents{ 20 };
	std::vector<AgarioAgent*> m_pAgentVec{};

	AgarioAgent* m_pCustomAgent = nullptr;

	const int m_AmountOfFood{ 40 };
	const float m_FoodSpawnDelay{ 2.f };
	float m_TimeSinceLastFoodSpawn{ 0.f };
	std::vector<AgarioFood*> m_pFoodVec{};

	AgarioContactListener* m_pContactListener = nullptr;
	bool m_GameOver = false;

	//Influence map vars
	bool m_RenderGraph = false;
	Elite::EGraphRenderer m_GraphRenderer{};
	Elite::InfluenceMap<InfluenceGrid>* m_pInfluenceGrid = nullptr;

	//Decision making 
	std::vector<Elite::FSMState*> m_pStates{};
	std::vector<Elite::FSMTransition*> m_pTransitions{};

private:	
	template<class T_AgarioType>
	void UpdateAgarioEntities(vector<T_AgarioType*>& entities, float deltaTime);

	void UpdateImGui();
private:
	//C++ make the class non-copyable
	App_AgarioInfluence(const App_AgarioInfluence&) {};
	App_AgarioInfluence& operator=(const App_AgarioInfluence&) {};
};

#endif

template<class T_AgarioType>
inline void App_AgarioInfluence::UpdateAgarioEntities(vector<T_AgarioType*>& entities, float deltaTime)
{
	for (auto& e : entities)
	{
		e->Update(deltaTime);

		auto agent = dynamic_cast<AgarioAgent*>(e);
		if (agent)
			//agent->TrimToWorld(m_TrimWorldSize);
			agent->TrimToWorld({ 0, 0 }, { m_TrimWorldSize * 2,  m_TrimWorldSize * 2 });

		if (e->CanBeDestroyed())
			SAFE_DELETE(e);
	}

	auto toRemoveEntityIt = std::remove_if(entities.begin(), entities.end(),
		[](T_AgarioType* e) {return e == nullptr; });
	if (toRemoveEntityIt != entities.end())
	{
		entities.erase(toRemoveEntityIt, entities.end());
	}
}
