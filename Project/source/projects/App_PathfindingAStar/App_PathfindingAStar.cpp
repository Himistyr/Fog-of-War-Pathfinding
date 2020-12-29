//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_PathfindingAStar.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAstar.h"

//Agent & steering includes
#include "../Shared/Agario/AgarioAgent.h"
#include "../App_Steering/SteeringBehaviors.h"

using namespace Elite;

//Destructor
App_PathfindingAStar::~App_PathfindingAStar()
{
	SAFE_DELETE(m_pGridGraph);
	SAFE_DELETE(m_pAgentView);
	for (AgarioAgent* agent : m_Team)
		SAFE_DELETE(agent);
	for (Seek* seek : m_SeekBehaviours)
		SAFE_DELETE(seek);
}

//Functions
void App_PathfindingAStar::Start()
{
	//Set Camera
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(39.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(73.0f, 35.0f));
	//DEBUGRENDERER2D->GetActiveCamera()->SetMoveLocked(true);
	//DEBUGRENDERER2D->GetActiveCamera()->SetZoomLocked(true);

	//Create Graph
	MakeGridGraph();

	//Create Agent
	AddAgent();
	
	//m_Team[m_CurrentIndex]->SetPosition(m_pGridGraph->GetNodeWorldPos(startPathIdx));
}

void App_PathfindingAStar::Update(float deltaTime)
{
	UNREFERENCED_PARAMETER(deltaTime);

	//INPUT
	//Check if the middle mousebutton was released
	bool const middleMouseReleased = INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eMiddle);
	if (middleMouseReleased)
	{
		//Get information of where the mouse was clicked
		MouseData mouseData = { INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eMiddle) };
		Elite::Vector2 mousePos = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ (float)mouseData.X, (float)mouseData.Y });

		//Find closest node to click pos and set the end node
		int closestNode = m_pGridGraph->GetNodeFromWorldPos(mousePos);

		if (closestNode != invalid_node_index) {

			//Check if we should move the agent or the destination
			if (m_AgentSelected)
			{
				m_Team[m_CurrentIndex]->SetPosition(m_pGridGraph->GetNodeWorldPos(closestNode));
				m_UpdatePath = true;
			}
			else
			{
				m_PathInformation[m_CurrentIndex].second = closestNode;
				m_UpdatePath = true;
			}
		}
	}

	//GRID INPUT + UPDATE ALL AGENTS
	bool UpdateAllAgents = m_UpdatePath || m_GraphEditor.UpdateGraph(m_pGridGraph);

	//IMGUI
	UpdateImGui();

	for (int i = 0; i < m_Team.size(); ++i) {

		//Update every actor if the graph changed
		if (UpdateAllAgents)
			m_UpdatePath = true;
		
		//Set startNode to agents current position
		int agentNode = m_pGridGraph->GetNodeFromWorldPos(m_Team[i]->GetPosition());

		//Check if the agent has entered a new node and if it's a valid node
		if (int(m_pGridGraph->GetNode(agentNode)->GetTerrainType()) <= 200000)
			m_PathInformation[i].first = agentNode;
		if (!m_Paths[i].empty() 
			&& m_PathInformation[i].first != m_Paths[i][0]->GetIndex() 
			&& m_PathInformation[i].first != m_PathInformation[i].second)
			m_UpdatePath = true;
		
		if (CheckTerrainInRadius(m_pGridGraph, m_pAgentView, agentNode))
			m_UpdatePath = true;
		
		//CALCULATEPATH
		//If we should find a new path and the start and end points are within the world, find a path!
		if (m_UpdatePath
			&& m_PathInformation[i].first != invalid_node_index
			&& m_PathInformation[i].second != invalid_node_index)
		{
			//Reset variables
			m_RenderPathAsHint = false;

			//AStar Pathfinding
			auto pathfinder = AStar<GridTerrainNode, GraphConnection>(m_pAgentView, m_pHeuristicFunction);

			auto startNode = m_pAgentView->GetNode(m_PathInformation[i].first);
			auto endNode = m_pAgentView->GetNode(m_PathInformation[i].second);

			bool PathFound = false;
			m_Paths[i] = pathfinder.FindPath(startNode, endNode, PathFound);

			if (!PathFound) {

				if (m_AllowWorldHints) {

					pathfinder = AStar<GridTerrainNode, GraphConnection>(m_pGridGraph, m_pHeuristicFunction);

					startNode = m_pGridGraph->GetNode(m_PathInformation[i].first);
					endNode = m_pGridGraph->GetNode(m_PathInformation[i].second);

					PathFound = false;
					m_Paths[i] = pathfinder.FindPath(startNode, endNode, PathFound);

					if (!PathFound) {

						auto startNode = m_pGridGraph->GetNode(m_PathInformation[i].first);
						m_Paths[i] = std::vector<Elite::GridTerrainNode*>{ startNode, startNode };
					}
					else {

						m_RenderPathAsHint = true;
						m_Paths[i] = std::vector<Elite::GridTerrainNode*>{ m_Paths[i][0], m_Paths[i][1] };
					}
				}
				else {

					auto startNode = m_pGridGraph->GetNode(m_PathInformation[i].first);
					m_Paths[i] = std::vector<Elite::GridTerrainNode*>{ startNode, startNode };
				}
			}

			m_UpdatePath = false;
			if (PathFound)
				std::cout << "New Path Calculated" << std::endl;
			else
				std::cout << "No Path Found" << std::endl;
		}

		//Check if the path is still an actuall path.
		//If true, move to the next Node on the path.
		//If false, move to the last Node.
		if (m_Paths[i].size() >= 2)
			m_SeekBehaviours[i]->SetTarget(TargetData{ m_pAgentView->GetNodeWorldPos(m_Paths[i][1]) });
		else
			m_SeekBehaviours[i]->SetTarget(TargetData{ m_pAgentView->GetNodeWorldPos(m_PathInformation[i].second) });

		m_Team[i]->Update(deltaTime);
	}
}

void App_PathfindingAStar::Render(float deltaTime) const
{
	UNREFERENCED_PARAMETER(deltaTime);
	//Render grid
	//Actor View
	m_GraphRenderer.RenderGraph(
		m_pAgentView,
		m_DrawActorView,
		m_bDrawNodeNumbers,
		m_bDrawConnections,
		m_bDrawConnectionsCosts,
		Elite::Color{ 1.f, 1.f, 1.f },
		m_DrawActorFieldOfView
	);

	//World
	m_GraphRenderer.RenderGraph(
		m_pGridGraph,
		m_bDrawGrid,
		m_bDrawNodeNumbers,
		m_bDrawConnections,
		m_bDrawConnectionsCosts,
		Elite::Color{ 0.f, 1.f, 0.f }
	);

	
	//Render current agents node on top if applicable
	if (m_PathInformation[m_CurrentIndex].first != invalid_node_index)
	{
		m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, { m_pGridGraph->GetNode(m_PathInformation[m_CurrentIndex].first) }, START_NODE_COLOR);
	}

	//Render end node on top if applicable
	if (m_PathInformation[m_CurrentIndex].second != invalid_node_index)
	{
		m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, { m_pGridGraph->GetNode(m_PathInformation[m_CurrentIndex].second) }, END_NODE_COLOR);
	}

	//render path below if applicable
	if (m_Paths[m_CurrentIndex].size() > 2 || m_RenderPathAsHint)
	{
		if (!m_RenderPathAsHint)
			m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, m_Paths[m_CurrentIndex]);
		else
			m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, m_Paths[m_CurrentIndex], Elite::Color{ 0.f, 0.f, 1.f });
	}

	for (AgarioAgent* agent : m_Team)
		agent->Render(deltaTime);
}

void App_PathfindingAStar::MakeGridGraph()
{
	m_pGridGraph = new WorldGrid(COLUMNS, ROWS, m_SizeCell, false, true, 1.f, 1.5f);
	m_pAgentView = new WorldGrid(COLUMNS, ROWS, m_SizeCell, false, true, 1.f, 1.5f);
}

void App_PathfindingAStar::UpdateImGui()
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		int menuWidth = 178;
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
		ImGui::Text("LMB: target");
		ImGui::Text("RMB: start");
		ImGui::Unindent();

		/*Spacing*/ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		/*Spacing*/ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

		//Start Debug Options
		ImGui::Text("Fog Of War Pathfinding");
		ImGui::Spacing();

		//Mouse Controller Options
		ImGui::Text("Middle Mouse Controls: ");
		std::string buttonText{ "" };
		if (m_AgentSelected)
			buttonText += "Agent Pos";
		else
			buttonText += "End Node";

		if (ImGui::Button(buttonText.c_str()))
		{
			m_AgentSelected = !m_AgentSelected;
		}

		//Manage Agents
		ImGui::Text("Manage Agents: ");

		std::string AoA{ "Amount of Agents: " + std::to_string(m_Team.size()) };
		ImGui::Text(AoA.c_str());

		if (ImGui::Button("Add Agent"))
		{
			AddAgent();
		}
		if (ImGui::Button("Remove Agent"))
		{
			RemoveAgent();
		}

		std::string CurrA{ "Current Agent: " + std::to_string(m_CurrentIndex + 1) };
		ImGui::Text(CurrA.c_str());

		if (ImGui::Button("Next Agent"))
		{
			m_CurrentIndex = (m_CurrentIndex + 1) % m_Team.size();

			m_UpdatePath = true;
		}
		if (ImGui::Button("Previous Agent"))
		{
			--m_CurrentIndex;
			if (m_CurrentIndex < 0)
				m_CurrentIndex += m_Team.size();

			m_UpdatePath = true;
		}
		
		//World Debug
		ImGui::Text("World Debug: ");

		ImGui::Checkbox("Grid", &m_bDrawGrid);
		ImGui::Checkbox("NodeNumbers", &m_bDrawNodeNumbers);
		ImGui::Checkbox("Connections", &m_bDrawConnections);
		ImGui::Checkbox("Connections Costs", &m_bDrawConnectionsCosts);

		//Actor Debug
		ImGui::Text("Actor View Debug: ");
		ImGui::Checkbox("Actor View", &m_DrawActorView);
		ImGui::Checkbox("Actor FoV", &m_DrawActorFieldOfView);
		ImGui::Checkbox("Allow World Hints", &m_AllowWorldHints);

		ImGui::Text("View Radius: ");
		ImGui::SliderInt("", &m_ViewRadius, 1, 8);

		//Heuristic Function Selector
		ImGui::Text("Heuristic Function: ");

		if (ImGui::Combo("", &m_SelectedHeuristic, "Manhattan\0Euclidean\0SqrtEuclidean\0Octile\0Chebyshev", 4))
		{
			switch (m_SelectedHeuristic)
			{
			case 0:
				m_pHeuristicFunction = HeuristicFunctions::Manhattan;
				break;
			case 1:
				m_pHeuristicFunction = HeuristicFunctions::Euclidean;
				break;
			case 2:
				m_pHeuristicFunction = HeuristicFunctions::SqrtEuclidean;
				break;
			case 3:
				m_pHeuristicFunction = HeuristicFunctions::Octile;
				break;
			case 4:
				m_pHeuristicFunction = HeuristicFunctions::Chebyshev;
				break;
			default:
				m_pHeuristicFunction = HeuristicFunctions::Chebyshev;
				break;
			}
		}
		ImGui::Spacing();

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
}

void App_PathfindingAStar::AddAgent()
{
	//Create new Seek for the new agent
	Seek* newSeek = new Seek{};
	newSeek->SetTarget(TargetData{ m_pGridGraph->GetNodeWorldPos(0) });

	m_SeekBehaviours.push_back(newSeek);

	//Create the new agent
	AgarioAgent* newAgent = new AgarioAgent{ Elite::Vector2{} };
	newAgent->SetSteeringBehavior(newSeek);
	newAgent->SetMaxLinearSpeed(20.f);
	newAgent->SetAutoOrient(true);
	newAgent->SetMass(0.1f);
	newAgent->SetBodyColor(Elite::Color{ 0.f, 0.f, 1.f });
	newAgent->SetPosition(m_pGridGraph->GetNodeWorldPos(0));

	//Increases the size of the agent
	newAgent->MarkForUpgrade(3);

	m_Team.push_back(newAgent);

	//Create path information for agent
	m_PathInformation.push_back(std::make_pair<>(0, 0));

	//Create path for agent
	std::vector<Elite::GridTerrainNode*> newPath;

	m_Paths.push_back(newPath);

	m_UpdatePath = true;
}

void App_PathfindingAStar::RemoveAgent()
{
	if (m_Team.size() <= 1)
		return;

	if (m_CurrentIndex == m_Team.size() - 1)
		--m_CurrentIndex;

	SAFE_DELETE(m_SeekBehaviours.back());
	m_SeekBehaviours.pop_back();

	SAFE_DELETE(m_Team.back());
	m_Team.pop_back();

	m_PathInformation.pop_back();

	m_Paths.pop_back();
}

bool App_PathfindingAStar::CheckTerrainInRadius(WorldGrid* world, WorldGrid* actorView, int startNodeIndex, int stepsTaken)
{
	//if the startNode is outside of the world-grid, stop looking in this direction.
	if (startNodeIndex >= world->GetColumns() * world->GetRows() || startNodeIndex < 0)
		return false;

	//Make the node render as "InVision"
	actorView->GetNode(startNodeIndex)->SetIsVisible(true);

	//If a Node with a different TerrainType than our agent remembers is found,
	//update our actors memory to remember this new TerrainType.
	bool newTerrainFound = (world->GetNode(startNodeIndex)->GetTerrainType() != actorView->GetNode(startNodeIndex)->GetTerrainType());
	if (newTerrainFound)
	{
		actorView->GetNode(startNodeIndex)->SetTerrainType(world->GetNode(startNodeIndex)->GetTerrainType());
		UpdateNode(actorView, startNodeIndex);
	}
		
	//If we still have not reached our ViewRadius,
	//check all Nodes around the current Node.
	if (stepsTaken + 1 <= m_ViewRadius) {
		
		//Usage of connections is not possible due to unpassable terrain getting disconnected from the graph,
		//blocking vision for no reason at all.
		bool neighborUp{}, neighborDown{}, neighborLeft{}, neighborRight{};
		int rowCheck = startNodeIndex / world->GetColumns();
		
		neighborUp = CheckTerrainInRadius(world, actorView, startNodeIndex + world->GetColumns(), stepsTaken + 1);
		neighborDown = CheckTerrainInRadius(world, actorView, startNodeIndex - world->GetColumns(), stepsTaken + 1);

		//Because our Grid is a 1D-array used as a 2D array,
		//we need to check if the next index isn't the start of a new row
		if ((startNodeIndex - 1) / world->GetColumns() == rowCheck)
			neighborLeft = CheckTerrainInRadius(world, actorView, startNodeIndex - 1, stepsTaken + 1);
		if ((startNodeIndex + 1) / world->GetColumns() == rowCheck)
			neighborRight = CheckTerrainInRadius(world, actorView, startNodeIndex + 1, stepsTaken + 1);

		//If we haven't found a difference in TerrainType yet,
		//check if the neighboring nodes did have a difference.
		if (!newTerrainFound)
			newTerrainFound = (neighborUp || neighborDown || neighborLeft || neighborRight);
	}

	return newTerrainFound;
}

void App_PathfindingAStar::UpdateNode(WorldGrid* pGraph, int idx)
{
	//If terrain is unpassable, set it here
	if (idx != invalid_node_index)
	{
		if (int(pGraph->GetNode(idx)->GetTerrainType()) > 200000)
			pGraph->IsolateNode(idx);
		else
			pGraph->UnIsolateNode(idx);
	}
}

