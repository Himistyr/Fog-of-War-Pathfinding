//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_PathfindingAStar.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAstar.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EBFS.h"

//Agent & steering includes
#include "../Shared/Agario/AgarioAgent.h"
#include "../App_Steering/SteeringBehaviors.h"

using namespace Elite;

//Destructor
App_PathfindingAStar::~App_PathfindingAStar()
{
	SAFE_DELETE(m_pGridGraph);
	SAFE_DELETE(m_pAgentView);
	SAFE_DELETE(m_pAgent);
	SAFE_DELETE(m_pSeekBehavior);
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

	startPathIdx = 0;
	endPathIdx = 4;

	m_pSeekBehavior = new Seek{};
	m_pSeekBehavior->SetTarget(TargetData{ m_pGridGraph->GetNodeWorldPos(startPathIdx) });
	m_pAgent = new AgarioAgent{ Elite::Vector2{} };
	m_pAgent->SetSteeringBehavior(m_pSeekBehavior);
	m_pAgent->SetMaxLinearSpeed(20.f);
	//m_pAgent->SetMaxAngularSpeed(40.f);
	m_pAgent->SetAutoOrient(true);
	m_pAgent->SetMass(0.1f);
	m_pAgent->SetBodyColor(Elite::Color{0.f, 0.f, 1.f});
	//Increases the size of the agent
	m_pAgent->MarkForUpgrade(3);

	m_pAgent->SetPosition(m_pGridGraph->GetNodeWorldPos(startPathIdx));
}

void App_PathfindingAStar::Update(float deltaTime)
{
	UNREFERENCED_PARAMETER(deltaTime);

	//INPUT
	//Set startNode to agents current position
	int agentNode = m_pGridGraph->GetNodeFromWorldPos(m_pAgent->GetPosition());

	//Check if the agent has entered a new node and if it's a valid node
	if (int(m_pGridGraph->GetNode(agentNode)->GetTerrainType()) <= 200000)
		startPathIdx = agentNode;
	if (m_vPath.size() > 2 && m_pAgentView->GetNode(startPathIdx) != m_vPath[0])
		m_UpdatePath = true;
		
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
				m_pAgent->SetPosition(m_pGridGraph->GetNodeWorldPos(closestNode));
				m_UpdatePath = true;
			}
			else
			{
				endPathIdx = closestNode;
				m_UpdatePath = true;
			}
		}
	}

	//GRID INPUT
	m_GraphEditor.UpdateGraph(m_pGridGraph);
	bool hasGridChanged = CheckTerrainInRadius(m_pGridGraph, m_pAgentView, startPathIdx);
	if (hasGridChanged)
		m_UpdatePath = true;

	//IMGUI
	UpdateImGui();

	//CALCULATEPATH
	//If we have nodes and the target is not the startNode, find a path!
	if (m_UpdatePath
		&& startPathIdx != invalid_node_index
		&& endPathIdx != invalid_node_index
		&& startPathIdx != endPathIdx)
	{
		//AStar Pathfinding
		auto pathfinder = AStar<GridTerrainNode, GraphConnection>(m_pAgentView, m_pHeuristicFunction);

		auto startNode = m_pAgentView->GetNode(startPathIdx);
		auto endNode = m_pAgentView->GetNode(endPathIdx);

		m_vPath = pathfinder.FindPath(startNode, endNode);

		m_UpdatePath = false;
		std::cout << "New Path Calculated" << std::endl;
	}

	//Check if the path is still an actuall path.
	//If true, move to the next Node on the path.
	//If false, move to the last Node.
	if (m_vPath.size() > 2)
		m_pSeekBehavior->SetTarget(TargetData{ m_pAgentView->GetNodeWorldPos(m_vPath[1]) });
	else 
		m_pSeekBehavior->SetTarget(TargetData{ m_pAgentView->GetNodeWorldPos(endPathIdx) });

	m_pAgent->Update(deltaTime);
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
		Elite::Color{ 1.f, 1.f, 1.f }
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

	
	//Render start node on top if applicable
	if (startPathIdx != invalid_node_index)
	{
		m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, { m_pGridGraph->GetNode(startPathIdx) }, START_NODE_COLOR);
	}

	//Render end node on top if applicable
	if (endPathIdx != invalid_node_index)
	{
		m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, { m_pGridGraph->GetNode(endPathIdx) }, END_NODE_COLOR);
	}

	//render path below if applicable
	if (m_vPath.size() > 0)
	{
		m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, m_vPath);
	}

	m_pAgent->Render(deltaTime);
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
		int menuWidth = 162;
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

		//World Debug
		ImGui::Text("World Debug: ");

		ImGui::Checkbox("Grid", &m_bDrawGrid);
		ImGui::Checkbox("NodeNumbers", &m_bDrawNodeNumbers);
		ImGui::Checkbox("Connections", &m_bDrawConnections);
		ImGui::Checkbox("Connections Costs", &m_bDrawConnectionsCosts);

		//Actor Debug
		ImGui::Text("Actor View Debug: ");
		ImGui::Checkbox("Actor View", &m_DrawActorView);

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

bool App_PathfindingAStar::CheckTerrainInRadius(WorldGrid* world, WorldGrid* actorView, int startNodeIndex, int stepsTaken)
{
	//if the startNode is outside of the world-grid, stop looking futher in this direction.
	if (startNodeIndex >= world->GetColumns() * world->GetRows() || startNodeIndex < 0)
		return false;

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
		
		//Usage of connections is not possible due to unpassabel terrain getting disconnected from the graph,
		//blocking vision for no reason at all.
		bool neighborUp = CheckTerrainInRadius(world, actorView, startNodeIndex + world->GetColumns(), stepsTaken + 1);
		bool neighborDown = CheckTerrainInRadius(world, actorView, startNodeIndex - world->GetColumns(), stepsTaken + 1);
		bool neighborLeft = CheckTerrainInRadius(world, actorView, startNodeIndex - 1, stepsTaken + 1);
		bool neighborRight = CheckTerrainInRadius(world, actorView, startNodeIndex + 1, stepsTaken + 1);

		//If we haven't found a difference in TerrainType yet,
		//check if the neighboring nodes did have a difference.
		if (!newTerrainFound)
			newTerrainFound = (neighborUp || neighborDown || neighborLeft || neighborRight);
	}

	return newTerrainFound;
}

void App_PathfindingAStar::UpdateNode(WorldGrid* pGraph, int idx)
{
	//If something has to change when a certain TerrainType is set,
	//change it here.
	if (idx != invalid_node_index)
	{
		switch (pGraph->GetNode(idx)->GetTerrainType())
		{
		case TerrainType::Water:
			pGraph->IsolateNode(idx);
			break;
		default:
			pGraph->UnIsolateNode(idx);
			break;
		}
	}
}

