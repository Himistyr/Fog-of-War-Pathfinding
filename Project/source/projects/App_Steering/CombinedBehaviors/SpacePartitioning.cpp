#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\App_Steering\SteeringAgent.h"

// --- Cell ---
// ------------
Cell::Cell(float left, float bottom, float width, float height)
	: inRange{ }
{
	boundingBox.bottomLeft = { left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_Neighbors(maxEntities)
	, m_NrOfNeighbors(0)
{
	m_CellHeight = height / rows;
	m_CellWidth = width / cols;

	for (size_t i = 0; i < rows; ++i) {
		for (size_t j = 0; j < cols; ++j) {
			m_Cells.push_back(Cell{ m_CellWidth * j, m_CellHeight * i, m_CellWidth, m_CellHeight });
		}
	}	
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	m_Cells[PositionToIndex(agent->GetPosition())].agents.push_back(agent);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, const Elite::Vector2& oldPos)
{
	int currentCell{ PositionToIndex(agent->GetPosition()) };
	int oldCell{ PositionToIndex(oldPos) };
	if (currentCell != oldCell) {
		m_Cells[currentCell].agents.push_back(agent);
		m_Cells[oldCell].agents.erase(std::remove(m_Cells[oldCell].agents.begin(), m_Cells[oldCell].agents.end(), agent));
	}
}

void CellSpace::RegisterNeighbors(const Elite::Vector2& pos, float queryRadius)
{
	m_NrOfNeighbors = 0;
	//Create square
	Elite::Rect BoundingBox{ Elite::Vector2{pos.x - queryRadius, pos.y - queryRadius}, queryRadius*2, queryRadius*2 };

	//Check if BoundingBox overlaps with cell
	for (Cell& cell : m_Cells) {
		
		cell.inRange = false;
		if (Elite::IsOverlapping(cell.boundingBox, BoundingBox)) {
			
			cell.inRange = true;
			for (SteeringAgent* agent : cell.agents) {

				if (Elite::DistanceSquared(agent->GetPosition(), pos) <= pow(queryRadius, 2)) {
					m_Neighbors[m_NrOfNeighbors++] = agent;
				}
			}
		}
	}
}

void CellSpace::RenderCells() const
{
	for (const Cell& cell : m_Cells) {
		
		Elite::Color color{ 1.f, 0.f, 0.f };
		float depth{-0.1f};
		if (cell.inRange == true) {
			depth = -0.2f;
			color = { 0.f, 1.f, 0.f };
		}

		DEBUGRENDERER2D->DrawSegment(cell.GetRectPoints()[0], cell.GetRectPoints()[1], color, depth);
		DEBUGRENDERER2D->DrawSegment(cell.GetRectPoints()[1], cell.GetRectPoints()[2], color, depth);
		DEBUGRENDERER2D->DrawSegment(cell.GetRectPoints()[2], cell.GetRectPoints()[3], color, depth);
		DEBUGRENDERER2D->DrawSegment(cell.GetRectPoints()[3], cell.GetRectPoints()[0], color, depth);
		DEBUGRENDERER2D->DrawString(cell.GetRectPoints()[1], std::to_string(cell.agents.size()).data());
	}
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	int row = int(pos.y / m_CellHeight) % m_NrOfRows;
	int col = int(pos.x / m_CellWidth) % m_NrOfCols;
	return Elite::Clamp(col + m_NrOfCols * row, 0, int(m_Cells.size() - 1));
}