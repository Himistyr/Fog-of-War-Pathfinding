#include "stdafx.h"
#include "ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

using namespace Elite;

Elite::NavGraph::NavGraph(const Polygon& contourMesh, float playerRadius = 1.0f) :
	Graph2D(false),
	m_pNavMeshPolygon(nullptr)
{
	//Create the navigation mesh (polygon of navigatable area= Contour - Static Shapes)
	m_pNavMeshPolygon = new Polygon(contourMesh); // Create copy on heap

	//Get all shapes from all static rigidbodies with NavigationCollider flag
	auto vShapes = PHYSICSWORLD->GetAllStaticShapesInWorld(PhysicsFlags::NavigationCollider);

	//Store all children
	for (auto shape : vShapes)
	{
		shape.ExpandShape(playerRadius);
		m_pNavMeshPolygon->AddChild(shape);
	}

	//Triangulate
	m_pNavMeshPolygon->Triangulate();

	//Create the actual graph (nodes & connections) from the navigation mesh
	CreateNavigationGraph();
}

Elite::NavGraph::~NavGraph()
{
	delete m_pNavMeshPolygon; 
	m_pNavMeshPolygon = nullptr;
}

int Elite::NavGraph::GetNodeIdxFromLineIdx(int lineIdx) const
{
	auto nodeIt = std::find_if(m_Nodes.begin(), m_Nodes.end(), [lineIdx](const NavGraphNode* n) { return n->GetLineIndex() == lineIdx; });
	if (nodeIt != m_Nodes.end())
	{
		return (*nodeIt)->GetIndex();
	}

	return invalid_node_index;
}

Elite::Polygon* Elite::NavGraph::GetNavMeshPolygon() const
{
	return m_pNavMeshPolygon;
}

void Elite::NavGraph::CreateNavigationGraph()
{
	//1. Go over all the edges of the navigationmesh and create nodes
	for (Line* currentLine : m_pNavMeshPolygon->GetLines()) {

		//Check if it is connected to 2 triangles (front and back) and if the line is valid
		if (m_pNavMeshPolygon->GetTrianglesFromLineIndex(currentLine->index).size() == 2 && currentLine->index != -1)
			AddNode(new NavGraphNode{GetNextFreeNodeIndex(), currentLine->index, currentLine->p1 + ((currentLine->p2 - currentLine->p1) / 2)});
	}

	//2. Create connections now that every node is created
	for (Triangle* currentTriangle : m_pNavMeshPolygon->GetTriangles()) {

		vector<int> foundNodes;
		for (int lineIndex : currentTriangle->metaData.IndexLines) {

			int nodeIndex{ GetNodeIdxFromLineIdx(lineIndex) };
			if (lineIndex != -1 && nodeIndex != -1)
				foundNodes.push_back(nodeIndex);
		}

		if (foundNodes.size() == 2)
			AddConnection(new GraphConnection2D{ foundNodes[0], foundNodes[1] });
		else if (foundNodes.size() == 3){
			AddConnection(new GraphConnection2D{ foundNodes[0], foundNodes[1] });
			AddConnection(new GraphConnection2D{ foundNodes[1], foundNodes[2] });
			AddConnection(new GraphConnection2D{ foundNodes[2], foundNodes[0] });
		}
	}

	//3. Set the connections cost to the actual distance
	SetConnectionCostsToDistance();
}

