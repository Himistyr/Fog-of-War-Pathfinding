#include "stdafx.h"
#include "App_AgarioInfluence.h"
#include "IStatesAndTransitions.h"
#include "../Shared/Agario/AgarioFood.h"
#include "../Shared/Agario/AgarioAgent.h"
#include "../Shared/Agario/AgarioContactListener.h"

using namespace Elite;
App_AgarioInfluence::App_AgarioInfluence()
{
}

App_AgarioInfluence::~App_AgarioInfluence()
{
	for (auto& f : m_pFoodVec)
	{
		SAFE_DELETE(f);
	}
	m_pFoodVec.clear();

	for (auto& a : m_pAgentVec)
	{
		SAFE_DELETE(a);
	}
	m_pAgentVec.clear();

	SAFE_DELETE(m_pContactListener);
	for (auto& s : m_pStates)
	{
		SAFE_DELETE(s);
	}

	for (auto& t : m_pTransitions)
	{
		SAFE_DELETE(t);
	}
	SAFE_DELETE(m_pCustomAgent);
	
	//Delete InfluenceGrid
	SAFE_DELETE(m_pInfluenceGrid);
}

void App_AgarioInfluence::Start()
{
	//Create food items
	m_pFoodVec.reserve(m_AmountOfFood);
	for (int i = 0; i < m_AmountOfFood; i++)
	{
		Elite::Vector2 randomPos = randomVector2(0 , m_TrimWorldSize * 2);
		m_pFoodVec.push_back(new AgarioFood(randomPos));
	}

	//Create agents
	m_pAgentVec.reserve(m_AmountOfAgents);
	for (int i = 0; i < m_AmountOfAgents; i++)
	{
		Elite::Vector2 randomPos = randomVector2(0, m_TrimWorldSize * 2);
		AgarioAgent* newAgent = new AgarioAgent(randomPos);

		Blackboard* pB = new Blackboard();
		pB->AddData("Agent", newAgent);

		WanderState* pWanderState = new WanderState();
		m_pStates.push_back(pWanderState);

		FiniteStateMachine* simpleFSM = new FiniteStateMachine(pWanderState, pB);
		newAgent->SetDecisionMaking(simpleFSM);

		m_pAgentVec.push_back(newAgent);
	}

	//Creating the world contact listener that informs us of collisions
	m_pContactListener = new AgarioContactListener();

	//Create Custom Agent
	Elite::Vector2 randomPos = randomVector2(0 , m_TrimWorldSize * 2);
	Color customColor = Color{ randomFloat(), randomFloat(), randomFloat() };
	m_pCustomAgent = new AgarioAgent(randomPos, customColor);

	//MAKE NEW FSM
	//MAKE MORE COMPLEX FSM
	InfluenceState* pInfluenceState = new InfluenceState();
	m_pStates.push_back(pInfluenceState);
	
	//Initilize Influence map
	m_pInfluenceGrid = new InfluenceMap<InfluenceGrid>(false);
	m_pInfluenceGrid->SetMomentum(1.f);
	m_pInfluenceGrid->SetDecay(0.1);
	m_pInfluenceGrid->InitializeGrid(int(m_TrimWorldSize / 2), int(m_TrimWorldSize / 2), int((m_TrimWorldSize * 2) / (m_TrimWorldSize / 2)), false, true);
	m_pInfluenceGrid->InitializeBuffer();

	//Create SeekState
	Blackboard* pB = new Blackboard();
	pB->AddData("Agent", m_pCustomAgent);
	pB->AddData("InfluenceMap", m_pInfluenceGrid);

	FiniteStateMachine* pComplexFSM = new FiniteStateMachine(pInfluenceState, pB);
	m_pCustomAgent->SetDecisionMaking(pComplexFSM);

	m_GraphRenderer.SetNumberPrintPrecision(0);
}

void App_AgarioInfluence::Update(float deltaTime)
{
	UpdateImGui();

	//Check if agent is still alive
	if (m_pCustomAgent->CanBeDestroyed())
	{
		m_GameOver = true;
		return;
	}
	//Update the custom agent
	m_pCustomAgent->Update(deltaTime);
	//m_pCustomAgent->TrimToWorld(m_TrimWorldSize); // <- Nutteloos!!
	m_pCustomAgent->TrimToWorld({ 0, 0 }, { m_TrimWorldSize * 2,  m_TrimWorldSize * 2 });

	//Update the other agents and food
	UpdateAgarioEntities(m_pFoodVec, deltaTime);
	UpdateAgarioEntities(m_pAgentVec, deltaTime);

	//Check if we need to spawn new food
	m_TimeSinceLastFoodSpawn += deltaTime;
	if (m_TimeSinceLastFoodSpawn > m_FoodSpawnDelay)
	{
		m_TimeSinceLastFoodSpawn = 0.f;
		m_pFoodVec.push_back(new AgarioFood(randomVector2(0, m_TrimWorldSize * 2)));
	}

	//Update InfluenceGrid
		//Food
	for (auto food : m_pFoodVec) {

		float foodInfluence = 50.f;
		m_pInfluenceGrid->SetInfluenceAtPosition(food->GetPosition(), foodInfluence);
	}

		//Agents
	for (auto agent : m_pAgentVec) {
		
		float radiusDifferenceAmplifier = 15.f;
		float danger = (m_pCustomAgent->GetRadius() - agent->GetRadius()) * radiusDifferenceAmplifier;
		m_pInfluenceGrid->SetInfluenceAtPosition(agent->GetPosition(), danger);
	}

	m_pInfluenceGrid->PropagateInfluence(deltaTime);
	m_pInfluenceGrid->SetNodeColorsBasedOnInfluence();
}

void App_AgarioInfluence::Render(float deltaTime) const
{
	//Render InfluenceGrid
	if (m_RenderGraph)
		m_GraphRenderer.RenderGraph(m_pInfluenceGrid, true, false, false, true);

	std::vector<Elite::Vector2> points =
	{
		{ 0, m_TrimWorldSize * 2 },
		{ m_TrimWorldSize * 2, m_TrimWorldSize * 2 },
		{ m_TrimWorldSize * 2, 0 },
		{ 0, 0 }
	};
	DEBUGRENDERER2D->DrawPolygon(&points[0], 4, { 1,0,0,1 }, 0.4f);

	for (AgarioFood* f : m_pFoodVec)
	{
		f->Render(deltaTime);
	}

	for (AgarioAgent* a : m_pAgentVec)
	{
		a->Render(deltaTime);
	}

	m_pCustomAgent->Render(deltaTime);
}

void App_AgarioInfluence::UpdateImGui()
{
	//------- UI --------
#ifdef PLATFORM_WINDOWS
#pragma region UI
	{
		//Setup
		int menuWidth = 150;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 90));
		ImGui::Begin("Agario", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);
		ImGui::SetWindowFocus();
		ImGui::PushItemWidth(70);
		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();
		
		ImGui::Text("Agent Info");
		ImGui::Text("Radius: %.1f",m_pCustomAgent->GetRadius());
		ImGui::Text("Survive Time: %.1f", TIMER->GetTotal());

		ImGui::Checkbox("Render as graph", &m_RenderGraph);
		
		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
	if(m_GameOver)
	{
		//Setup
		int menuWidth = 300;
		int menuHeight = 100;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2(width/2.0f- menuWidth, height/2.0f - menuHeight));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)menuHeight));
		ImGui::Begin("Game Over", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		ImGui::Text("Final Agent Info");
		ImGui::Text("Radius: %.1f", m_pCustomAgent->GetRadius());
		ImGui::Text("Survive Time: %.1f", TIMER->GetTotal());
		ImGui::End();
	}
#pragma endregion
#endif

}
