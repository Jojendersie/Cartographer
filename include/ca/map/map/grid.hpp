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
			// Add rows on top (front of the vector)
			if(_coord.y < m_yPosition)
			{
				Row emptyRow;
				emptyRow.num.push_back({0, -1});
				emptyRow.m_xPosition = 0.0f;
				m_rows.insert(m_rows.begin(), m_yPosition - _coord.y, emptyRow);
				m_yPosition = _coord.y;
				// We now have an empty row -> fill in the element
				m_rows.front().num = {1, 0};
				m_rows.front().cells.push_back(_value);
				m_rows.front().m_xPosition = _coord.x;
			}
			// Add rows at bottom (back of the vector)
			else if(_coord.y >= m_yPosition + m_rows.size()) {
				Row emptyRow;
				emptyRow.num.push_back({0, -1});
				emptyRow.m_xPosition = 0.0f;
				m_rows.insert(m_rows.back(), _coord.y - (m_yPosition + m_rows.size()), emptyRow);
				// We now have an empty row -> fill in the element
				m_rows.back().num = {1, 0};
				m_rows.back().cells.push_back(_value);
				m_rows.back().m_xPosition = _coord.x;
			}
			// There is already a row, insert the element to it
			else {
				auto& row = m_rows[_coord.y - m_yPosition];
				// Empty row?
				if(row.num[0].endOffset == 0) {
					row.num[0].endOffset++;
					row.num[0].accessOffset = 0;
					row.m_xPosition = _coord.x;
				}
				// Insert in front of the other elements
				else if(_coord.x < row.m_xPosition)
				{
					row.cells.insert(row.cells.front(), _value);
					// Directly in front or with a gap?
					if(_coord.x == row.m_xPosition-1)
						row.num[0].endOffset++;
					else {
						row.num.insert(row.num.begin(), {0,-1});
						row.num.insert(row.num.begin(), {1,0});
					}
					row.m_xPosition = _coord.x;
					// Repair all the offsets after the first (new/updated) entry
					for(size_t i = 1; i < row.num.size(); ++i)
					{
						row.num[i].endOffset++;
						if(row.num[i].accessOffset != -1) row.num[i].accessOffset++;
					}
				}
				// Insert at the end
				else if(_coord.x >= m_rows.m_xPosition + row.cells.size())
				{
					row.cells.insert(row.cells.back(), _value);
					// Directly at the end or with a gap?
					if(_coord.x == m_rows.m_xPosition + row.cells.size())
						row.num.back().endOffset++;
					else {
						row.num.push_back({0,-1});
						row.num.push_back({1,row.cells.size()-1});
					}
				}
				// Find the position with binary search
				else {
					int l = row.m_xPosition
					int r = row.m_xPosition + row.cells.size();
					int t = _coord.x - row.m_xPosition;
					while(l < r)
					{
						int m = (l + r) / 2;
						if(row.num[m].endOffset <= t) l = m;
						else if(m == 0) {
							// The element overwrites one in the first range
							row.cells[row.num[m].accessOffset + t] = _value;
							return;
						}
						else if(row.num[m-1].endOffset <= t)
						{
							// Empty ranges have odd indices...
							if(m & 1) {
								row.cells.insert(row.cells.begin() + row[m+1].accessOffset, _value);
								// Fuse three ranges, if the empty range is completly
								// filled by the new element
								if(row.num[m-1].endOffset + 1 == row[m].endOffset) {
									row.num[m-1].endOffset = row[m+1].endOffset;
									row.num.erease(row.num.begin() + m, row.num.begin() + m + 2);
								}
								// Append to one of the neighbour ranges or create two new ranges
								else if(row.num[m-1].endOffset == t)
									row.num[m-1].endOffset++;
								else if(row.num[m].endOffset == t+1)
									m++; // No action necessary, except that the repair does not need to modify m+1 element.
								else {
									row.num.insert(row.num.begin() + m, {t+1, row.num[m+1].accessOffset});
									row.num.insert(row.num.begin() + m, {t, -1});
									m += 2;
								}
								// Repair access offsets
								for(size_t i = m+1; i < row.num.size(); ++i)
									if(row.num[i].accessOffset != -1) row.num[i].accessOffset++;
							} else {
								// Replace element in this range
								row.cells[row.num[m].accessOffset + (t - row.num[m-1].endOffset)] = _value;
							}
							return;
						} else r = m;
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
			// Range check for x-coordinate
			int l = row.m_xPosition
			int r = row.m_xPosition + row.cells.size();
			if(_coord.x < l) return nullptr;
			if(_coord.x >= r) return nullptr;
			int t = _coord.x - row.m_xPosition;
			// Binary search of the x-coordinate
			while(l < r)
			{
				int m = (l + r) / 2;
				if(row.num[m].endOffset <= t) l = m;
				else if(m == 0) return &row.cells[row.num[m].accessOffset + t];
				else if(row.num[m-1].endOffset <= t)
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