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
			// Iterate over a minimal range in y
			auto itThis = m_rows.begin();
			auto itOther = _other.m_rows.begin();
			if(_other.m_yPosition > m_yPosition)
				itThis += _other.m_yPosition - m_yPosition;
			else itOther += m_yPosition - _other.m_yPosition;
			for(int y = max(_other.m_yPosition, m_yPosition); y < min(_other.m_yPosition + _other.m_rows.size(), m_yPosition + m_rows.size()); ++y)
			{
				// First search the entry point.
				int j;
				binSearch(*itThis, _coord.x, j);
				// Compare each cell in the gird-row if there is an occupied cell in the current map.
				for(int i = 0; i < itOther->xpos.size(); ++i)
				{
					while(itThis->xpos[j] < itOther->xpos[i]) {
						++j;
						if(j == itThis->xpos.size()) goto NextRow;
					}
					if(itThis->xpos[j] == itOther->xpos[i])
						res &= _comparator(itThis->cells[j], itOther->cells[i]);
				}
			NextRow:
				++itThis;
				++itOther;
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
					int m;
					if(binSearch(row, _coord.x, m)) {
						row.cells[m] = _value;
					} else {
						// Not found -> insert at m or m-1 ???? is only one case possible
						if(_coord.x > row.xpos[m]) --m;
						row.cells.insert(row.cells.begin() + m, _value);
						row.xpos.insert(row.xpos.begin() + m, _coord.x);
					}					
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
			int m;
			if(binSearch(row, _coord.x, m))
				return row.cells[m];
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
		
		// Search for a specific x-coordinate.
		// \param [out] _m The coordiate where the binary search stopped.
		// \returns true if the element was found.
		bool binSearch(const Row& _row, int _x, int& _m)
		{
			int l = 0;
			int r = _row.xpos.size();
			if(_x < _row.xpos[l]) { _m = l; return false; }
			if(_x >= _row.xpos[r-1]) { _m = r; return false; }
			// Binary search of the x-coordinate
			while(l < r)
			{
				_m = (l + r) / 2;
				if(row.xpos[_m] < _x) l = m;
				else if(row.xpos[_m] == _x) return true;
				else r = m;
			}
			return false;
		}
	};
}}