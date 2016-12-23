#pragma once

#include <functional>

namespace ca { namespace map {
	
	/// A grid is a row wise sparse array with either quad- or hex- neigborhood.
	/// The origin is top-left andt coordinates increase towards east and south.
	template<typename T>
	class Grid
	{
	public:
		class Type {
			QUAD,
			HEX,
		};
		
		Grid(Type _type);
		
		/// Rotates a grid by 90° or 60° times the number of ticks dependent on the type.
		/// \details A rotation reorganizes the memory and is therefore expensive for large
		///		grids.
		void rotate(int _ticks);
		
		/// Compare occupied cells of _other and AND all the results.
		template<typename T2>
		bool compare(const Grid<T2>& _other, std::function<bool(const T&, const T2&)> _comparator) const
		{
		}
		
		/// Set/overwrite any grid cell.
		void set(const ei::IVec2& _coord, const T& _value)
		{

		}
		
		/// Find out if a grid cell contains some element and returns an access pointer to it.
		T* find(const ei::IVec2& _coord)
		{
			// Check y-range if element is on map at all.
			if(_coord.y < m_yPosition) return nullptr;
			if(_coord.y >= m_yPosition + m_rows.size()) return nullptr;
			
			auto& row = m_rows[_coord.y - m_yPosition];
			// Range check for x-coordinate
			int l = m_rows.m_xPosition
			int r = m_rows.m_xPosition + row.cells.size();
			if(_coord.x < l) return nullptr;
			if(_coord.x >= r) return nullptr;
			int t = _coord.x - row.m_xPosition;
			// Binary search of the x-coordinate
			while(l < r)
			{
				int m = (l + r) / 2;
				if(row.num[m].endOffset < t) l = m;
				else if(m == 0) return &row.cells[row.num[m].accessOffset + t];
				else if(row.num[m-1].endOffset < t)
				{
					// Empty ranges have odd indices...
					if(m & 1) return nullptr;
					return &row.cells[row.num[m].accessOffset + (t - row.num[m-1].endOffset)];
				} else r = m;
			}
			// Due to range check the element must be found.
			caAssert(false);
			return nullptr;
		}
		
		/// Get a cell. If it was not occupied before it gets filled with a default constructed
		/// T element.
		T& get(const ei::IVec2& _coord); // TODO: implement if needed

	private:
		struct Row {
			// 'num' encodes a lot of run-length vectors. The first number is the number
			// of leading occupied tiles. Then, vectors of empty tiles and occupied tiles
			// interleave where 'num' contains the end offsets (exclusive).
			// Example: 5, 7, 8, 11, 12 means there are 5 occupied tiles, then 2 empty,
			// 1 occupied, 3 empty and 1 occupied again.
			// accessOffset is the number of occupied tiles up to the start of the entry.
			// This offset addresses are used to access the tile-vector. Empty ranges
			// have odd intices and always contain -1 offsets.
			// Neither the first nor the last range are empty.
			// In the example above this gives 0, -1, 5, -1, 6
			std::vector<struct {uint endOffset; uint accessOffset;}> num;
			std::vector<T> cells;
			int m_xPosition;
		};
		std::vector<Row> m_rows;
		
		// The position is a global reference position of the first tile of the first row.
		int m_yPosition;
	};
}}