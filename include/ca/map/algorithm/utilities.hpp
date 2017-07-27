#pragma once

#include "ca/map/map/gridbase.hpp"
#include "ca/map/map/iterator.hpp"
#include <queue>

namespace details {

	// Discovered but unevaluated nodes
	struct OpenNode {
		ca::map::GridCoord coord;
		float expectedCost;						// Heuristical length of a path from start to goal through this node
		// Less comparison for queues
		bool operator < (const OpenNode& _rhs)
		{
			return expectedCost < _rhs.expectedCost;
		}
		// Cast to cost for priority updates
		explicit operator float () const
		{
			return expectedCost;
		}
		// Special assignment for new priorities
		void operator = (float _newCost)
		{
			expectedCost = _newCost;
		}
	};

	// (Partially) evaluated nodes (for path searches).
	struct VisitedNode {
		ca::map::GridCoord cameFrom;
		float minCost;						// Costs up to that point of the path
		typename ca::pa::PriorityQueue<OpenNode>::Handle openHandle;	// Constant handle into the openSet. Can be invalid is this node is fully evaluated. 
	};

	// Coordinate hash
	struct FastCoordHash {
		uint32_t operator () (const ca::map::GridCoord& _v) {
			return _v.x * 198491329 + _v.y * 879190747;
		}
	};

} // namespace details

namespace ca { namespace map {

	typedef pa::HashSet<GridCoord, ::details::FastCoordHash> ReachableSet;

	/// Simple reachable method, in which each tile has a constant cost if not empty.
	/// The range is equal to the grid distance.
	template<typename MapT>
	void findReachable(const MapT& _map, ReachableSet& _reachableTiles,
		const GridCoord& _from, int _maxDistance,
		std::function<bool(typename const MapT::TCellType&)> _isEmpty
	) {
		using namespace ::details;
		_reachableTiles.clear();
		// Breadth first search with a queue
		std::queue<OpenNode> queue;
		queue.push({_from, 0.0f});
		_reachableTiles.add(_from);
		while(!queue.empty())
		{
			// Are neighbors close enough?
			float newCost = queue.front().expectedCost + 1.0f;
			if(newCost <= _maxDistance)
			{
				for(NeighborIterator<MapT::TGridType> it( queue.front().coord, 1 ); it; ++it) if(it.coord() != queue.front().coord)
				{
					// Not evaluated yet?
					auto cell = _map.find(it.coord());
					if(cell && !_isEmpty(*cell) && !_reachableTiles.find(it.coord()))
					{
						queue.push({it.coord(), newCost});
						_reachableTiles.add(it.coord());
					}
				}
			}
			queue.pop();
		}
	}

	/// More involved method to find reachable tiles with a cost function.
	/// \param [in] _reachableTiles Set which is cleared and then filled with the results.
	///		It contains all reachable tiles excluding the start tile.
	/// \param [in] _maxCost Find all tiles which have at most _maxCost path costs.
	/// \param [in] _cost A functor which assignes (entity specific) costs for each cell. Negative
	///		costs mark obstacles.
	///		The arguments of the cost function are the cell position and the comeFrom position.
	///		Therefore anisotropic functions are possible.
	template<unsigned GridT>
	void findReachable(ReachableSet& _reachableTiles,
		const GridCoord& _from, float _maxCost,
		const std::function<float(const GridCoord&, const GridCoord&)>& _costFunc)
	{
		using namespace ::details;
		_reachableTiles.clear();
		//_reachableTiles.add({_from, 0.0f});
		// Add central tile to helper search data structures
		pa::PriorityQueue<OpenNode> openSet;
		pa::HashMap<GridCoord, typename ca::pa::PriorityQueue<OpenNode>::Handle, FastCoordHash> evalSet;
		evalSet.add(_from, openSet.add(OpenNode{_from, 0.0f}));

		while(!openSet.empty())
		{
			OpenNode node = openSet.popMin();
			_reachableTiles.add(node.coord);
			for(NeighborIterator<GridT> it(node.coord, 1); it; ++it) if(it.coord() != node.coord)
			{
				// Only explore if in range
				float stepCost = _costFunc(node.coord, it.coord());
				float cost = node.expectedCost + stepCost;
				if(stepCost >= 0.0f && cost <= _maxCost)
				{
					// Did we evaluate the neighbor before?
					auto neighborVN = evalSet.find(it.coord());
					if(neighborVN && !_reachableTiles.find(it.coord()))
					{
						if(neighborVN.data()->expectedCost > cost)
							openSet.changePriority(neighborVN.data(), cost);
					} else {
						// Not seen this neighbor before, add it
						evalSet.add(it.coord(), openSet.add(OpenNode{it.coord(), cost}));
					}
				}
			}
		}
	}

}} // namespace ca::map