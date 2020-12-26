#include "stdafx.h"
#include "App_AgarioGame.h"
#include "StatesAndTransitions.h"
#include "../Shared/Agario/AgarioFood.h"
#include "../Shared/Agario/AgarioAgent.h"
#include "../Shared/Agario/AgarioContactListener.h"

using namespace Elite;
App_AgarioGame::App_AgarioGame()
{
}

App_AgarioGame::~App_AgarioGame()
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
}

void App_AgarioGame::Start()
{
	//Create food items
	m_pFoodVec.reserve(m_AmountOfFood);
	for (int i = 0; i < m_AmountOfFood; i++)
	{
		Elite::Vector2 randomPos = randomVector2(-m_TrimWorldSize, m_TrimWorldSize);
		m_pFoodVec.push_back(new AgarioFood(randomPos));
	}

	//Create agents
	m_pAgentVec.reserve(m_AmountOfAgents);
	for (int i = 0; i < m_AmountOfAgents; i++)
	{
		Elite::Vector2 randomPos = randomVector2(-m_TrimWorldSize, m_TrimWorldSize);
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
	Elite::Vector2 randomPos = randomVector2(-m_TrimWorldSize, m_TrimWorldSize);
	Color customColor = Color{ randomFloat(), randomFloat(), randomFloat() };
	m_pCustomAgent = new AgarioAgent(randomPos, customColor);

	//MAKE NEW FSM
	//MAKE MORE COMPLEX FSM
	WanderState* pWanderState = new WanderState();
	m_pStates.push_back(pWanderState);

	SeekFoodState* pSeekFoodState = new SeekFoodState();
	m_pStates.push_back(pSeekFoodState);

	FleeState* pFleeState = new FleeState();
	m_pStates.push_back(pFleeState);

	ChaseState* pChaseState = new ChaseState();
	m_pStates.push_back(pChaseState);
	
	//Create SeekState
	Blackboard* pB = new Blackboard();
	pB->AddData("Agent", m_pCustomAgent);
	pB->AddData("OtherAgents", &m_pAgentVec);
	pB->AddData("FoodVec", &m_pFoodVec);
	pB->AddData("DetectionRange", 15.f);
	pB->AddData("AgentTarget", m_pAgentVec[0]);
	pB->AddData("FoodTarget", m_pFoodVec[0]);

	CloseToFood* pCloseToFoodTrans = new CloseToFood();
	m_pTransitions.push_back(pCloseToFoodTrans);

	FoodEaten* pFoodEatenTrans = new FoodEaten();
	m_pTransitions.push_back(pFoodEatenTrans);

	FleeBigger* pFleeBiggerTrans = new FleeBigger();
	m_pTransitions.push_back(pFleeBiggerTrans);

	StopFleeing* pStopFleeingTrans = new StopFleeing();
	m_pTransitions.push_back(pStopFleeingTrans);

	ChaseSmaller* pChaseSmallerTrans = new ChaseSmaller();
	m_pTransitions.push_back(pChaseSmallerTrans);

	StopChasing* pStopChasingTrans = new StopChasing();
	m_pTransitions.push_back(pStopChasingTrans);

	FiniteStateMachine* pComplexFSM = new FiniteStateMachine(pWanderState, pB);
	//Add transition wander-seek when food nearby = true
	//Sequence of added transitions matter! first added means more importance!
	//If flee is added before moveToFood, flee will always take priority!
		//Flee Checks
	pComplexFSM->AddTransition(pFleeState, pWanderState, pStopFleeingTrans);
	pComplexFSM->AddTransition(pWanderState, pFleeState, pFleeBiggerTrans);
	//Chase Checks
	pComplexFSM->AddTransition(pChaseState, pFleeState, pFleeBiggerTrans);
	pComplexFSM->AddTransition(pChaseState, pWanderState, pStopChasingTrans);
	pComplexFSM->AddTransition(pWanderState, pChaseState, pChaseSmallerTrans);
	//Seek Checks
	pComplexFSM->AddTransition(pSeekFoodState, pFleeState, pFleeBiggerTrans);
	pComplexFSM->AddTransition(pSeekFoodState, pChaseState, pChaseSmallerTrans);
	pComplexFSM->AddTransition(pSeekFoodState, pWanderState, pFoodEatenTrans);
	pComplexFSM->AddTransition(pWanderState, pSeekFoodState, pCloseToFoodTrans);

	m_pCustomAgent->SetDecisionMaking(pComplexFSM);
}

void App_AgarioGame::Update(float deltaTime)
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
	m_pCustomAgent->TrimToWorld(m_TrimWorldSize);

	//Update the other agents and food
	UpdateAgarioEntities(m_pFoodVec, deltaTime);
	UpdateAgarioEntities(m_pAgentVec, deltaTime);

	
	//Check if we need to spawn new food
	m_TimeSinceLastFoodSpawn += deltaTime;
	if (m_TimeSinceLastFoodSpawn > m_FoodSpawnDelay)
	{
		m_TimeSinceLastFoodSpawn = 0.f;
		m_pFoodVec.push_back(new AgarioFood(randomVector2(-m_TrimWorldSize, m_TrimWorldSize)));
	}
}

void App_AgarioGame::Render(float deltaTime) const
{
	std::vector<Elite::Vector2> points =
	{
		{ -m_TrimWorldSize, m_TrimWorldSize },
		{ m_TrimWorldSize, m_TrimWorldSize },
		{ m_TrimWorldSize, -m_TrimWorldSize },
		{ -m_TrimWorldSize, -m_TrimWorldSize }
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

void App_AgarioGame::UpdateImGui()
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
