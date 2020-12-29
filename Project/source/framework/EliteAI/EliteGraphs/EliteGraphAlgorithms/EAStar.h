#pragma once

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);
		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode, bool& foundPath);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		//hier implementeren we AStar
		vector<T_NodeType*> path;
		vector<NodeRecord> openList;
		vector<NodeRecord> closedList;
		NodeRecord currentRecord;

		NodeRecord startRecord{ pStartNode };
		startRecord.estimatedTotalCost = GetHeuristicCost(pStartNode, pGoalNode);

		openList.push_back(startRecord);
		while (!openList.empty()) {
			//set currentRecord to lowest F-Cost
			currentRecord = openList.front();
			for (const NodeRecord& NR : openList)
				if (NR < currentRecord)
					currentRecord = NR;

			//Check if this leads to the EndNode
			if (currentRecord.pNode == pGoalNode)
				break;

			//Loop over the connections
			for (auto connection : m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex())) {

				//Calculate cost so far
				float currentCost = currentRecord.costSoFar + connection->GetCost();

				//Check if this connection is cheaper than a connection on the closed list.
				//If false, do the same for the open list.
				vector<NodeRecord>::iterator closedListIt{ std::find_if(closedList.begin(), closedList.end(), [&connection](NodeRecord value) {
					return(connection->GetTo() == value.pNode->GetIndex());
					}) };
				vector<NodeRecord>::iterator openListIt{ std::find_if(openList.begin(), openList.end(), [&connection](NodeRecord value) {
					return(connection->GetTo() == value.pNode->GetIndex());
					}) };
				if (closedListIt != closedList.end())
					if ((*closedListIt).costSoFar <= currentCost)
						continue;
					else
						closedList.erase(closedListIt);
				else if (openListIt != openList.end())
					if ((*openListIt).costSoFar <= currentCost)
						continue;
					else
						openList.erase(openListIt);

				//Create a new node from the next connection and add it to the open list
				NodeRecord nextNode{};
				nextNode.pNode = m_pGraph->GetNode(connection->GetTo());
				nextNode.pConnection = connection;
				nextNode.costSoFar = currentCost;
				nextNode.estimatedTotalCost = currentCost + GetHeuristicCost(nextNode.pNode, pGoalNode);

				openList.push_back(nextNode);
			}
			//remove the currentRecord from the open list and add it to the closed list
			vector<NodeRecord>::iterator findCurrentRecord{ std::find(openList.begin(), openList.end(), currentRecord) };
			if (findCurrentRecord != openList.end())
				openList.erase(findCurrentRecord);

			closedList.push_back(currentRecord);
		}

		while (currentRecord.pNode != pStartNode){

			path.push_back(currentRecord.pNode);
			currentRecord = *std::find_if(closedList.begin(), closedList.end(), [currentRecord, this](NodeRecord value) {
				return value.pNode == m_pGraph->GetNode(currentRecord.pConnection->GetFrom());
				});
		}

		path.push_back(pStartNode);
		std::reverse(path.begin(), path.end());
		return path;
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode, bool& foundPath)
	{
		//hier implementeren we AStar
		vector<T_NodeType*> path;
		vector<NodeRecord> openList;
		vector<NodeRecord> closedList;
		NodeRecord currentRecord;

		NodeRecord startRecord{ pStartNode };
		startRecord.estimatedTotalCost = GetHeuristicCost(pStartNode, pGoalNode);

		openList.push_back(startRecord);
		while (!openList.empty()) {
			//set currentRecord to lowest F-Cost
			currentRecord = openList.front();
			for (const NodeRecord& NR : openList)
				if (NR < currentRecord)
					currentRecord = NR;

			//Check if this leads to the EndNode
			//if true, set the parameter bool to true
			if (currentRecord.pNode == pGoalNode) {
				foundPath = true;
				break;
			}

			//Loop over the connections
			for (auto connection : m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex())) {

				//Calculate cost so far
				float currentCost = currentRecord.costSoFar + connection->GetCost();

				//Check if this connection is cheaper than a connection on the closed list.
				//If false, do the same for the open list.
				vector<NodeRecord>::iterator closedListIt{ std::find_if(closedList.begin(), closedList.end(), [&connection](NodeRecord value) {
					return(connection->GetTo() == value.pNode->GetIndex());
					}) };
				vector<NodeRecord>::iterator openListIt{ std::find_if(openList.begin(), openList.end(), [&connection](NodeRecord value) {
					return(connection->GetTo() == value.pNode->GetIndex());
					}) };
				if (closedListIt != closedList.end())
					if ((*closedListIt).costSoFar <= currentCost)
						continue;
					else
						closedList.erase(closedListIt);
				else if (openListIt != openList.end())
					if ((*openListIt).costSoFar <= currentCost)
						continue;
					else
						openList.erase(openListIt);

				//Create a new node from the next connection and add it to the open list
				NodeRecord nextNode{};
				nextNode.pNode = m_pGraph->GetNode(connection->GetTo());
				nextNode.pConnection = connection;
				nextNode.costSoFar = currentCost;
				nextNode.estimatedTotalCost = currentCost + GetHeuristicCost(nextNode.pNode, pGoalNode);

				openList.push_back(nextNode);
			}
			//remove the currentRecord from the open list and add it to the closed list
			vector<NodeRecord>::iterator findCurrentRecord{ std::find(openList.begin(), openList.end(), currentRecord) };
			if (findCurrentRecord != openList.end())
				openList.erase(findCurrentRecord);

			closedList.push_back(currentRecord);
		}

		while (currentRecord.pNode != pStartNode) {

			path.push_back(currentRecord.pNode);
			currentRecord = *std::find_if(closedList.begin(), closedList.end(), [currentRecord, this](NodeRecord value) {
				return value.pNode == m_pGraph->GetNode(currentRecord.pConnection->GetFrom());
				});
		}

		path.push_back(pStartNode);
		std::reverse(path.begin(), path.end());
		return path;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}

