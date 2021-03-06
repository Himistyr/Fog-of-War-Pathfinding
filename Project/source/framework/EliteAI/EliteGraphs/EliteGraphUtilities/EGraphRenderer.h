#pragma once


#include "framework\EliteAI\EliteGraphs\EGraphNodeTypes.h"
#include "framework\EliteAI\EliteGraphs\EGraphConnectionTypes.h"
#include "framework\EliteAI\EliteGraphs\EGridGraph.h"
#include "framework\EliteAI\EliteGraphs\EGraph2D.h"

namespace Elite 
{
	class EGraphRenderer final
	{
	public:
		EGraphRenderer() = default;
		~EGraphRenderer() = default;

		template<class T_NodeType, class T_ConnectionType>
		void RenderGraph(Graph2D<T_NodeType, T_ConnectionType>* pGraph, bool renderNodes, bool renderConnections) const;

		template<class T_NodeType, class T_ConnectionType>
		void RenderGraph(GridGraph<T_NodeType, T_ConnectionType>* pGraph, bool renderNodes, bool renderNodeNumbers, bool renderConnections, bool renderConnectionsCosts, Elite::Color gridColor = DEFAULT_NODE_COLOR, bool UseFieldOfView = false) const;

		template<class T_NodeType>
		void RenderHighlighted(std::vector<T_NodeType*> path, Color col = HIGHLIGHTED_NODE_COLOR) const;

		template<class T_NodeType, class T_ConnectionType>
		void RenderHighlightedGrid(GridGraph<T_NodeType, T_ConnectionType>* pGraph, std::vector<T_NodeType*> path, Color col = HIGHLIGHTED_NODE_COLOR) const;

	private:
		void RenderCircleNode(Vector2 pos, std::string text = "", float radius = 3.0f, Elite::Color col = DEFAULT_NODE_COLOR, float depth = 0.0f) const;
		void RenderRectNode(Vector2 pos, std::string text = "", float width = 3.0f, Elite::Color col = DEFAULT_NODE_COLOR, float depth = 0.0f) const;
		void RenderConnection(GraphConnection* con, Elite::Vector2 toPos, Elite::Vector2 fromPos, std::string text, Elite::Color col = DEFAULT_CONNECTION_COLOR, float depth = 0.0f) const;
	
	
		//C++ make the class non-copyable
		EGraphRenderer(const EGraphRenderer&) = delete;
		EGraphRenderer& operator=(const EGraphRenderer&) = delete;
	};


	template<class T_NodeType, class T_ConnectionType>
	void EGraphRenderer::RenderGraph(Graph2D<T_NodeType, T_ConnectionType>* pGraph, bool renderNodes, bool renderConnections) const
	{
		for (auto node : pGraph->GetAllNodes())
		{
			if (node->GetIndex() == invalid_node_index)
				continue;

			if (renderNodes)
			{
				//Node
				RenderCircleNode(
					pGraph->GetNodePos(node),
					std::to_string(node->GetIndex()),
					3.0f, 
					node->GetColor()
				);
			}
		
			if (renderConnections)
			{
				//Connections
				for (auto con : pGraph->GetNodeConnections(node->GetIndex()))
				{
					std::stringstream ss;
					ss << std::fixed << std::setprecision(1) << con->GetCost();

					RenderConnection(con,
						pGraph->GetNodePos(con->GetTo()),
						pGraph->GetNodePos(con->GetFrom()),
						ss.str()
					);
				}
			}
		}
	}

	template<class T_NodeType, class T_ConnectionType>
	void EGraphRenderer::RenderGraph(GridGraph<T_NodeType, T_ConnectionType>* pGraph, bool renderNodes, bool renderNodeNumbers, bool renderConnections, bool renderConnectionsCosts, Elite::Color gridColor, bool UseFieldOfView) const
	{
		if (renderNodes)
		{
			//Nodes/Grid
			for (auto r = 0; r < pGraph->m_NrOfRows; ++r)
			{
				for (auto c = 0; c < pGraph->m_NrOfColumns; ++c)
				{
					int idx = r * pGraph->m_NrOfColumns + c;
					Vector2 cellPos{ pGraph->GetNodeWorldPos(idx) };

					int cellSize = pGraph->m_CellSize;
					Vector2 verts[4]
					{
						Vector2(cellPos.x - cellSize / 2.0f, cellPos.y - cellSize / 2.0f),
						Vector2(cellPos.x - cellSize / 2.0f, cellPos.y + cellSize / 2.0f),
						Vector2(cellPos.x + cellSize / 2.0f, cellPos.y + cellSize / 2.0f),
						Vector2(cellPos.x + cellSize / 2.0f, cellPos.y - cellSize / 2.0f)
					};

					//Grid
					DEBUGRENDERER2D->DrawPolygon(&verts[0], 4, gridColor, 0.0f);

					//Node
					std::string text{};
					if (renderNodeNumbers)
					{
						text = to_string(idx);
					}
					Elite::Color nodeColor = pGraph->GetNodeColor(pGraph->GetNode(idx));
					if (UseFieldOfView && pGraph->GetNode(idx)->GetIsVisible()) {

						nodeColor.r += .4f;
						nodeColor.g += .4f;
						nodeColor.b += .4f;
						pGraph->GetNode(idx)->SetIsVisible(false);
					}

					RenderRectNode(cellPos, text, float(cellSize), nodeColor, 0.1f);
				}
			}
		}

		if (renderConnections)
		{
			for (auto node : pGraph->GetAllNodes())
			{
				//Connections
				for (auto con : pGraph->GetNodeConnections(node->GetIndex()))
				{
					std::string text{ };
					if (renderConnectionsCosts)
					{
						std::stringstream ss;
						ss << std::fixed << std::setprecision(1) << con->GetCost();
						text = ss.str();
					}
					RenderConnection(con,
						pGraph->GetNodeWorldPos(con->GetTo()),
						pGraph->GetNodeWorldPos(con->GetFrom()),
						text
					);
				}
			}
		}
	}

	template<class T_NodeType>
	void EGraphRenderer::RenderHighlighted(std::vector<T_NodeType*> path, Color col /*= HIGHLIGHTED_NODE_COLOR*/) const
	{
		for (auto node : path)
		{
			//Node
			RenderCircleNode(
				node->GetPosition(),
				"",
				3.1f, 
				col,
				-0.2f
			);
		}
	}

	template<class T_NodeType, class T_ConnectionType>
	void EGraphRenderer::RenderHighlightedGrid(GridGraph<T_NodeType, T_ConnectionType>* pGraph, std::vector<T_NodeType*> path, Color col /*= HIGHLIGHTED_NODE_COLOR*/) const
	{
		for (auto node : path)
		{
			//Node
			RenderCircleNode(
				pGraph->GetNodeWorldPos(node),
				"",
				3.1f,
				col,
				-0.2f
			);
		}
	}


}
