#pragma once

#include "ca/map/map/gridbase.hpp"
#include "ca/map/map/iterator.hpp"

namespace ca { namespace map {

	// Path search helper structs and functions
	namespace details {
		// Discovered but unevaluated nodes
		struct OpenNode {
			GridCoord pos;		// Axial-hex coordinate
			float score;		// Heuristical length of a path from start to goal through this node
			bool operator < (OpenNode& _rhs)
			{
				return score < _rhs.score;
			}
			explicit operator float () const
			{
				return score;
			}
			// Special assignment for new priorities
			void operator = (float _newScore)
			{
				score = _newScore;
			}
		};
		// (Partially) evaluated nodes.
		struct VisitedNode {
			GridCoord cameFrom;
			float minCost;						// Costs up to that point of the path
			typename ca::pa::PriorityQueue<OpenNode>::Handle openHandle;	// Constant handle into the openSet. Can be invalid is this node is fully evaluated. 
		};
		struct FastCoordHash {
			uint32_t operator () (const GridCoord& _v) {
				return _v.x * 198491329 + _v.y * 879190747;
			}
		};
	}

	/// A* path finding algorithm.
	/// \param [out] _path Output buffer for the path. Previous contents of the buffer are removed.
	///		The path contains all nodes from _to (inclusive) to _from (exclusive).
	///		The order is reversed such that pop_back() can be used to efficently process the path.
	///		If no path could be found the output contains a path which comes closest to the target.
	/// \param [in] _cost A functor which assignes (entity specific) costs for each cell. Negative
	///		costs mark obstacles.
	///		The arguments of the cost function are the cell position and
	///		the comeFrom position. Therefore anisotropic functions are possible.
	/// \param [in] _validGoal Predicate function to test if a cell can be used as final target.
	///		This allows that some nodes can be passed temporarly.
	/// \param [in] _maxCostOffset Paths are not expanded if they cost more than
	///		_maxCostOffset + _maxCostFactor * dist(start, goal). This criterial is to prevent
	///		very long path searches for impossible paths.
	/// \param [in] _maxCostFactor See _maxCostOffset.
	/// \returns False if no path is possible.
	template<typename CellT, unsigned GridT>
	bool findPath(
		std::vector<GridCoord>& _path,
		const GridCoord& _from,
		const GridCoord& _to,
		std::function<float(const GridCoord&, const GridCoord&)> _cost,
		std::function<bool(const GridCoord&, const GridCoord&)> _validGoal,
		float _maxCostOffset = 100.0f,
		float _maxCostFactor = 2.5f,
		bool _findApproximateGoal = true
	) {
		using namespace details;
		_path.clear();
		GridCoord bestPossiblePos = _from;

		// Discovered but unevaluated nodes
		pa::PriorityQueue<OpenNode> openSet;
		// A second map with (partially) evaluated nodes.
		pa::HashMap<GridCoord, VisitedNode, FastCoordHash> evalSet;

		// Add the start point (later backtracking will unroll the path in reversed order).
		VisitedNode newVNode;
		newVNode.openHandle = openSet.add(OpenNode{_from, (float)distance<GridT>(_from, _to)});
		newVNode.cameFrom = _from;
		newVNode.minCost = 0.0f;
		evalSet.add(_from, newVNode);

		float maxCost = (float)distance<GridT>(_from, _to) * _maxCostFactor + _maxCostOffset;

		while(!openSet.empty())
		{
			OpenNode currentMinON = openSet.popMin();
			VisitedNode& currentVN = evalSet.find(currentMinON.pos).data();
			// The shortest path to be evaluated contains the start position? -> finish
			if(currentMinON.pos == _to) {
				if(_validGoal(currentMinON.pos, currentVN.cameFrom))
				{
					bestPossiblePos = _to;
					goto ReconstructPath;
				} else if(_findApproximateGoal) // The real target is not possible. Explore more to find a near neighbor
					continue;
				// End method, but do not go anywhere
				else goto ReconstructPath;
			}
			// Mark the node as closed.
			currentVN.openHandle = ca::pa::PriorityQueue<OpenNode>::INVALID_HANDLE;
			// Store costs, because adding neighbors invalidates the memory address.
			float currentMinCost = currentVN.minCost;
			// For each neighbor
			auto neighborIt = NeighborIterator<GridT>(currentMinON.pos, 1);
			for(; neighborIt; ++neighborIt) if(neighborIt.coord() != currentMinON.pos)
			{
				// Is it an obstacle?
				float cost = _cost(neighborIt.coord(), currentMinON.pos);
				if(cost >= 0.0f)
				{
					// Use the hashmap to find out if the neighbor is open, evaluated or new.
					auto neighborVN = evalSet.find(neighborIt.coord());
					float currentCost = currentMinCost + cost;
					float newScore = currentCost + distance<GridT>(neighborIt.coord(), _to);
					if(currentCost > maxCost) continue;
					if(!neighborVN)
					{
						// Node never seen. Add to open set and then to the map.
						VisitedNode newVNode;
						newVNode.openHandle = openSet.add(OpenNode{neighborIt.coord(), newScore});
						newVNode.cameFrom = currentMinON.pos;
						newVNode.minCost = currentCost;
						evalSet.add(neighborIt.coord(), newVNode);
					} else if(neighborVN.data().openHandle != ca::pa::PriorityQueue<OpenNode>::INVALID_HANDLE)
					{
						// The node is already in the open set. Did the path improve?
						if(newScore < openSet.get(neighborVN.data().openHandle).score)
						{
							neighborVN.data().cameFrom = currentMinON.pos;
							neighborVN.data().minCost = currentCost;
							openSet.changePriority(neighborVN.data().openHandle, newScore);
						}
					} // else the node is closed and can be ignored.
				}
			}
		}

		// There is no path from start to goal, but maybe we come close.
		if(_findApproximateGoal)
		{
			float closestDistance = ei::predecessor((float)distance<GridT>(_from, _to));
			float closestDistanceCost = maxCost;
			for(auto&& it : evalSet)
			{
				float currentPathDistanceToGoal = (float)distance<GridT>(it.key(), _to);
				float currentPathCost = it.data().minCost;
				if(currentPathDistanceToGoal <= closestDistance && _validGoal(it.key(), it.data().cameFrom))
				{
					if(currentPathDistanceToGoal < closestDistance || currentPathCost < closestDistanceCost)
					{
						closestDistance = currentPathDistanceToGoal;
						closestDistanceCost = currentPathCost;
						bestPossiblePos = it.key();
					}
				}
			}
		}

	ReconstructPath:
		// Go through the map and find all predecessors in order.
		// Since we started at the start this backtracking goes from the goal
		// to the start.
		GridCoord pos = bestPossiblePos;
		while(pos != _from)
		{
			_path.push_back(pos);
			auto h = evalSet.find(pos);
			pos = h.data().cameFrom;
		}
		return bestPossiblePos == _to;
	}

}} // namespace ca::map
