#pragma once
#include <stack>
#include <algorithm>
#include "Shared/Graph/Graph.h"

namespace GameAI
{
	enum class Eulerianity
	{
		notEulerian,
		semiEulerian,
		eulerian,
	};

	template <typename TNode>
	class EulerianPath final
	{
	public:
		EulerianPath(Graph* const pGraph);

		Eulerianity IsEulerian() const;
		std::vector<TNode*> FindPath() const;

	private:
		void VisitAllNodesDFS(int nodeIdx, const std::vector<TNode*>& nodes, std::vector<bool>& visited) const;
		bool IsConnected() const;

		Graph* m_pGraph;
	};

	template <typename TNode>
	inline EulerianPath<TNode>::EulerianPath(Graph* const pGraph)
		: m_pGraph(pGraph)
	{
	}

	template <typename TNode>
	inline Eulerianity EulerianPath<TNode>::IsEulerian() const
	{
		if (!IsConnected())
			return Eulerianity::notEulerian;

		int oddCount = 0;
		auto nodes = m_pGraph->GetActiveNodes();

		for (auto* pNode : nodes)
		{
			auto connections = m_pGraph->GetConnections(pNode->GetId());
			if (connections.size() % 2 != 0)
			{
				oddCount++;
			}
		}

		if (oddCount > 2)
			return Eulerianity::notEulerian;

		if (oddCount == 2)
			return Eulerianity::semiEulerian;

		return Eulerianity::eulerian;
	}

	template <typename TNode>
	inline std::vector<TNode*> EulerianPath<TNode>::FindPath() const
	{
		Eulerity eulerianity = IsEulerian();
		if (eulerianity == Eulerianity::notEulerian)
			return {};

		Graph graphCopy = m_pGraph->Clone();
		std::vector<TNode*> path = {};
		auto nodes = graphCopy.GetActiveNodes();

		if (nodes.empty()) return {};

		int startNodeId = nodes[0]->GetId();
		if (eulerianity == Eulerianity::semiEulerian)
		{
			for (auto* n : nodes)
			{
				if (graphCopy.GetConnections(n->GetId()).size() % 2 != 0)
				{
					startNodeId = n->GetId();
					break;
				}
			}
		}

		std::stack<int> nodeStack;
		nodeStack.push(startNodeId);

		while (!nodeStack.empty())
		{
			int currId = nodeStack.top();
			auto connections = graphCopy.GetConnections(currId);

			if (!connections.empty())
			{
				int neighborId = connections[0].ToNodeId;
				graphCopy.RemoveConnection(currId, neighborId);
				nodeStack.push(neighborId);
			}
			else
			{
				path.push_back(static_cast<TNode*>(m_pGraph->GetNode(currId)));
				nodeStack.pop();
			}
		}

		std::reverse(path.begin(), path.end());
		return path;
	}

	template <typename TNode>
	inline void EulerianPath<TNode>::VisitAllNodesDFS(int nodeIdx, const std::vector<TNode*>& nodes, std::vector<bool>& visited) const
	{
		visited[nodeIdx] = true;
		int nodeId = nodes[nodeIdx]->GetId();
		auto connections = m_pGraph->GetConnections(nodeId);

		for (auto& conn : connections)
		{
			for (int i = 0; i < (int)nodes.size(); ++i)
			{
				if (nodes[i]->GetId() == conn.ToNodeId && !visited[i])
				{
					VisitAllNodesDFS(i, nodes, visited);
				}
			}
		}
	}

	template <typename TNode>
	inline bool EulerianPath<TNode>::IsConnected() const
	{
		auto nodes = m_pGraph->GetActiveNodes();
		if (nodes.empty()) return true;

		int startIdx = -1;
		for (int i = 0; i < (int)nodes.size(); ++i)
		{
			if (!m_pGraph->GetConnections(nodes[i]->GetId()).empty())
			{
				startIdx = i;
				break;
			}
		}

		if (startIdx == -1)
			return nodes.size() <= 1;

		std::vector<bool> visited(nodes.size(), false);
		VisitAllNodesDFS(startIdx, nodes, visited);

		for (int i = 0; i < (int)nodes.size(); ++i)
		{
			if (!visited[i] && !m_pGraph->GetConnections(nodes[i]->GetId()).empty())
				return false;
		}

		return true;
	}
}