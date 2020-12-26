#pragma once
#include <stack>

namespace Elite
{
	enum class Eulerianity
	{
		notEulerian,
		semiEulerian,
		eulerian,
	};

	template <class T_NodeType, class T_ConnectionType>
	class EulerianPath
	{
	public:

		EulerianPath(IGraph<T_NodeType, T_ConnectionType>* pGraph);

		Eulerianity IsEulerian() const;
		vector<T_NodeType*> FindPath(Eulerianity& eulerianity) const;

	private:
		void VisitAllNodesDFS(int startIdx, vector<bool>& visited) const;
		bool IsConnected() const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
	};

	template<class T_NodeType, class T_ConnectionType>
	inline EulerianPath<T_NodeType, T_ConnectionType>::EulerianPath(IGraph<T_NodeType, T_ConnectionType>* pGraph)
		: m_pGraph(pGraph)
	{
	}

	template<class T_NodeType, class T_ConnectionType>
	inline Eulerianity EulerianPath<T_NodeType, T_ConnectionType>::IsEulerian() const
	{
		// If the graph is not connected, there can be no Eulerian Trail
		if (IsConnected() == false)
			return Eulerianity::notEulerian;

		// Count nodes with odd degree 
		int nrOfNodes = m_pGraph->GetNrOfNodes();
		int oddCount = 0;
		for (int i = 0; i < nrOfNodes; i++)
			if (m_pGraph->IsNodeValid(i) && (m_pGraph->GetNodeConnections(i).size() & 1))
				oddCount++;

		// A connected graph with more than 2 nodes with an odd degree (an odd amount of connections) is not Eulerian
		if (oddCount > 2)
			return Eulerianity::notEulerian;

		// A connected graph with exactly 2 nodes with an odd degree is Semi-Eulerian (an Euler trail can be made, but only starting and ending in these 2 nodes)
		else if (oddCount == 2 && nrOfNodes != 2)
			return Eulerianity::semiEulerian;

		// A connected graph with no odd nodes is Eulerian
		else
			return Eulerianity::eulerian;

	}

	template<class T_NodeType, class T_ConnectionType>
	inline vector<T_NodeType*> EulerianPath<T_NodeType, T_ConnectionType>::FindPath(Eulerianity& eulerianity) const
	{
		// Get a copy of the graph because this algorithm involves removing edges
		shared_ptr<IGraph<T_NodeType, T_ConnectionType>> graphCopy = m_pGraph->Clone();
		int nrOfNodes = graphCopy->GetNrOfNodes();
		vector<T_NodeType*> path = vector<T_NodeType*>();

		int index{ 0 };
		T_NodeType* currentNode{ nullptr };
		std::stack <T_NodeType*> stack{};

		// algorithm...
		switch (eulerianity) {
		case Eulerianity::eulerian:
			currentNode = graphCopy.get()->GetNode(0);
			break;
		case Eulerianity::semiEulerian:
			for (int i = 0; i < nrOfNodes; i++) {
				if (m_pGraph->IsNodeValid(i) && (m_pGraph->GetNodeConnections(i).size() & 1)) {
					currentNode = graphCopy.get()->GetNode(i);
					break;
				}
			}
			break;
		}

		if (currentNode != nullptr) {
			do {
				if (graphCopy.get()->GetNodeConnections(currentNode->GetIndex()).size() == 0) {
					path.push_back(currentNode);
					currentNode = stack.top();
					stack.pop();
				}
				else {
					stack.push(currentNode);
					currentNode = graphCopy.get()->GetNode(graphCopy.get()->GetNodeConnections(currentNode->GetIndex()).front()->GetTo());
					graphCopy.get()->RemoveConnection(graphCopy.get()->GetNodeConnections(stack.top()->GetIndex()).front());
				}
			} while (stack.size() != 0);
			path.push_back(currentNode);
		}
		return path;
	}

	template<class T_NodeType, class T_ConnectionType>
	inline void EulerianPath<T_NodeType, T_ConnectionType>::VisitAllNodesDFS(int startIdx, vector<bool>& visited) const
	{
		// mark the visited node
		visited[startIdx] = true;

		// recursively visit any valid connected nodes that were not visited before
		for (T_ConnectionType* connection : m_pGraph->GetNodeConnections(startIdx))
			if (m_pGraph->IsNodeValid(connection->GetTo()) && !visited[connection->GetTo()])
				VisitAllNodesDFS(connection->GetTo(), visited);
	}

	template<class T_NodeType, class T_ConnectionType>
	inline bool EulerianPath<T_NodeType, T_ConnectionType>::IsConnected() const
	{
		int nrOfNodes = m_pGraph->GetNrOfNodes();
		vector<bool> visited(nrOfNodes, false);

		// find a valid starting node that has connections
		int connectedIdx = invalid_node_index;
		for (int i = 0; i < nrOfNodes; i++) {
			if (m_pGraph->IsNodeValid(i) && m_pGraph->GetNodeConnections(i).size()) {
				connectedIdx = i;
				break;
			}
		}

		// if no valid node could be found, return false
		if (connectedIdx == invalid_node_index)
			return false;

		// start a depth-first-search traversal from a node that has connections
		VisitAllNodesDFS(connectedIdx, visited);

		// if a node was never visited, this graph is not connected
		for (int i = 0; i < nrOfNodes; i++)
			if (m_pGraph->IsNodeValid(i) && visited[i] == false)
				return false;

		return true;
	}

}