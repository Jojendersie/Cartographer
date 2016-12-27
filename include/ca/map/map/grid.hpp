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
		/// \details To implement an OR or some else return value you may use exceptions.
		///
		///		Empty rows or cells of any grid are skipped. If the two maps do not
		///		overlap the return value is true.
		template<typename T2>
		bool compare(const Grid<T2>& _other, std::function<bool(const T&, const T2&)> _comparator) const
		{
			bool res = true;
			for(int i = max(_other.m_yPosition, m_yPosition); i < min(_other.m_yPosition + _other.m_rows.size(), m_yPosition + m_rows.size()); ++i)
			{
			}
			return res;
		}
		
		/// Set/overwrite any grid cell.
		void set(const ei::IVec2& _coord, const T& _value)
		{
			// Add rows on top (front of the vector)
			if(_coord.y < m_yPosition)
			{
				Row emptyRow;
				m_rows.insert(m_rows.begin(), m_yPosition - _coord.y, emptyRow);
				m_yPosition = _coord.y;
				// We now have an empty row -> fill in the element
				m_rows.front().cells.push_back(_value);
				m_rows.front().xpos.push_back(_coord.x);
			}
			// Add rows at bottom (back of the vector)
			else if(_coord.y >= m_yPosition + m_rows.size()) {
				Row emptyRow;
				m_rows.insert(m_rows.back(), _coord.y - (m_yPosition + m_rows.size()), emptyRow);
				// We now have an empty row -> fill in the element
				m_rows.back().cells.push_back(_value);
				m_rows.front().xpos.push_back(_coord.x);
			}
			// There is already a row, insert the element to it
			else {
				auto& row = m_rows[_coord.y - m_yPosition];
				// Empty row?
				if(row.cells.size() == 0) {
					row.cells.push_back(_value);
					row.xpos.push_back(_coord.x);
				}
				// Insert in front of the other elements
				else if(_coord.x < row.xpos.first())
				{
					row.cells.insert(row.cells.front(), _value);
					row.xpos.insert(row.xpos.front(), _coord.x);
				}
				// Insert at the end
				else if(_coord.x >= row.xpos.last())
				{
					row.cells.insert(row.cells.back(), _value);
					row.xpos.insert(row.xpos.back(), _coord.x);
				}
				// Find the position with binary search
				else {
					int l = 0;
					int r = row.xpos.size();
					// Binary search of the x-coordinate
					while(l < r)
					{
						int m = (l + r) / 2;
						if(row.xpos[m] < _coord.x) l = m;
						else if(row.xpos[m] == _coord.x) {row.cells[m] = _value; return;}
						else r = m;
					}
					// Not found -> insert at m or m-1 ???? is only one case possible
					if(_coord.x > row.xpos[m]) --m;
					row.cells.insert(row.cells.begin() + m, _value);
					row.xpos.insert(row.xpos.begin() + m, _coord.x);
				}
			}
		}
		
		/// Find out if a grid cell contains some element and returns an access pointer to it.
		T* find(const ei::IVec2& _coord)
		{
			// Check y-range if element is on map at all.
			if(_coord.y < m_yPosition) return nullptr;
			if(_coord.y >= m_yPosition + m_rows.size()) return nullptr;
			
			auto& row = m_rows[_coord.y - m_yPosition];
			// Range check for x-coordinate
			int l = 0;
			int r = row.xpos.size();
			if(_coord.x < row.xpos[l]) return nullptr;
			if(_coord.x >= row.xpos[r-1]) return nullptr;
			// Binary search of the x-coordinate
			while(l < r)
			{
				int m = (l + r) / 2;
				if(row.xpos[m] < _coord.x) l = m;
				else if(row.xpos[m] == _coord.x) return row.cells[m];
				else else r = m;
			}
			// Not found = in some empty range
			return nullptr;
		}
		
		/// Get a cell. If it was not occupied before it gets filled with a default constructed
		/// T element.
		T& get(const ei::IVec2& _coord); // TODO: implement if needed

	private:
		struct Row {
			// CRS (compressed row storage) like format.
			// Stores the x positions of each occupied entry and its content
			std::vector<int> xpos;
			std::vector<T> cells;
		};
		std::vector<Row> m_rows;
		
		// The position is a global reference position of the first tile of the first row.
		int m_yPosition;
	};
}}