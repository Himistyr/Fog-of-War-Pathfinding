#include "stdafx.h"
#include "TheFlock.h"

#include "../SteeringAgent.h"
#include "../SteeringBehaviors.h"
#include "CombinedSteeringBehaviors.h"

using namespace Elite;

//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/, 
	float worldSize /*= 100.f*/, 
	SteeringAgent* pAgentToEvade /*= nullptr*/, 
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld { trimWorld }
	, m_pAgentToEvade{pAgentToEvade}
	, m_NeighborhoodRadius{ 15 }
	, m_NrOfNeighbors{0}
{
	m_pSeek = new Seek{};
	m_pEvade = new Evade{};
	m_pWander = new Wander{};

	m_pCohesion = new Cohesion{ this };
	m_pSeperation = new Seperation{ this, m_NeighborhoodRadius };
	m_pVelocity = new Velocity{ this };
	m_pBlendedSteering = new BlendedSteering({ {m_pCohesion, 0.2f}, {m_pSeperation, 0.2f}, {m_pVelocity, 0.2f}, {m_pSeek, 0.2f}, {m_pWander, 0.2f} });
	m_pPrioritySteering = new PrioritySteering({m_pEvade, m_pBlendedSteering});

	m_CellSpace = new CellSpace{ m_WorldSize, m_WorldSize, m_CellRows, m_CellCollums, m_FlockSize };

	m_Neighbors.resize(m_FlockSize);
	for (int i = 0; i < m_FlockSize; ++i) {
		m_Agents.push_back(new SteeringAgent{});
		m_Agents[i]->SetPosition(Elite::Vector2(randomFloat(worldSize), randomFloat(worldSize)));
		m_Agents[i]->SetSteeringBehavior(m_pPrioritySteering);
		m_Agents[i]->SetMaxLinearSpeed(40.f);
		m_Agents[i]->SetAutoOrient(true);
		m_Agents[i]->SetMass(0.1f);

		//For Partitioning
		previousPositions.push_back(m_Agents[i]->GetPosition());
		m_CellSpace->AddAgent(m_Agents[i]);
	}
}

Flock::~Flock()
{
	SAFE_DELETE(m_pSeek);
	SAFE_DELETE(m_pEvade);
	SAFE_DELETE(m_pCohesion);
	SAFE_DELETE(m_pSeperation);
	SAFE_DELETE(m_pVelocity);
	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);

	for (SteeringAgent * agent: m_Agents) {
		SAFE_DELETE(agent);
	}
}

void Flock::Update(float deltaT, TargetData mouse)
{
	// loop over all the boids
	// register its neighbors
	// update it
	// trim it to the world
	int agentIndex{ 0 };
	TargetData evadeTarget;
	evadeTarget.LinearVelocity = Elite::GetNormalized( m_pAgentToEvade->GetLinearVelocity()) * 5.f;
	evadeTarget.Position = m_pAgentToEvade->GetPosition();

	m_pSeek->SetTarget(mouse);
	m_pEvade->SetTarget(evadeTarget);

	if (m_TrimWorld) m_pAgentToEvade->TrimToWorld({ 0, 0 }, { m_WorldSize, m_WorldSize });
	for (SteeringAgent* agent : m_Agents) {
		if (!m_TogglePartitioning)
			//No Partitioning
			RegisterNeighbors(agent);
		else {
			//Partitioning
			m_CellSpace->UpdateAgentCell(agent, previousPositions[agentIndex]);
			m_CellSpace->RegisterNeighbors(agent->GetPosition(), m_NeighborhoodRadius);
			m_Neighbors = m_CellSpace->GetNeighbors();
			m_NrOfNeighbors = m_CellSpace->GetNrOfNeighbors();

			previousPositions[agentIndex++] = agent->GetPosition();
		}
		agent->Update(deltaT);
		if (m_TrimWorld) agent->TrimToWorld({ 0, 0 }, { m_WorldSize, m_WorldSize });
	}
}

void Flock::Render(float deltaT)
{
	if (m_VisualizeDebugAgents)
		DrawDebug();
	if (m_VisualizeDebugCells && m_TogglePartitioning)
		m_CellSpace->RenderCells();
}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
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

	ImGui::Text("Flocking");
	ImGui::Spacing();

	// Implement checkboxes and sliders here
	ImGui::Checkbox("Trim World", &m_TrimWorld);
	ImGui::Spacing();

	ImGui::Checkbox("Debug Agent", &m_VisualizeDebugAgents);
	ImGui::Spacing();

	ImGui::Checkbox("Debug Cells", &m_VisualizeDebugCells);
	ImGui::Spacing();

	ImGui::Checkbox("Use Partitioning", &m_TogglePartitioning);
	ImGui::Spacing();

	ImGui::Text("Behaviour Weights");
	ImGui::Spacing();

	ImGui::SliderFloat("Cohesion", &m_pBlendedSteering->m_WeightedBehaviors[0].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Seperation", &m_pBlendedSteering->m_WeightedBehaviors[1].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Velocity", &m_pBlendedSteering->m_WeightedBehaviors[2].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Seek", &m_pBlendedSteering->m_WeightedBehaviors[3].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Wander", &m_pBlendedSteering->m_WeightedBehaviors[4].weight, 0.f, 1.f, "%.2");


	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
	
}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	// register the agents neighboring the currently evaluated agent
	// store how many they are, so you know which part of the vector to loop over
	m_NrOfNeighbors = 0;
	for (SteeringAgent* agent : m_Agents) {
		if (agent != pAgent
			&& DistanceSquared(agent->GetPosition(), pAgent->GetPosition()) <= pow(m_NeighborhoodRadius, 2)) {
			m_Neighbors[m_NrOfNeighbors] = agent;
			++m_NrOfNeighbors;
		}
	}
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	Elite::Vector2 Average{};

	for (int i = 0; i < m_NrOfNeighbors; i++) {
		Average += m_Neighbors[i]->GetPosition();
	}

	return Average/m_NrOfNeighbors;
}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	Elite::Vector2 Average{};

	for (int i = 0; i < m_NrOfNeighbors; i++) {
		Average += m_Neighbors[i]->GetLinearVelocity();
	}

	return Average / m_NrOfNeighbors;
}

float* Flock::GetWeight(ISteeringBehavior* pBehavior) 
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->m_WeightedBehaviors;
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if(it!= weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}

void Flock::DrawDebug() {
	DEBUGRENDERER2D->DrawCircle(m_Agents[m_Agents.size()-1]->GetPosition(), m_NeighborhoodRadius, { 1.f, 1.f, 1.f, 1.f }, 0.1f);
}