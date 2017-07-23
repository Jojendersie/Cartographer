﻿#pragma once

#include <functional>
#include <vector>
#include <ca/paper.hpp>
#include "gridbase.hpp"

namespace ca { namespace map {

	/// A grid is a row wise sparse array with either quad- or hex- neigborhood.
	/// The origin is bottom-left and coordinates increase towards east and north.
	/// \details
	///		A good source for hexagonal coordinates: http://www.redblobgames.com/grids/hexagons/
	template<unsigned GridT, typename CellT>
	class SparseGrid
	{
	public:

		/// Rotates a grid by 90° or 60° times the number of ticks dependent on the type.
		/// \details A rotation reorganizes the memory and is therefore expensive for large
		///		grids.
		void rotate(int _ticks)
		{
			ei::IMat2x2 rot;
			if(GridT == unsigned(GridType::HEX))
			{
				// The rotation of integral hex-coordinates requires +1/-1 in more dimensions.
				// E.g. a rotation by 60° converts (0,0), (1,0), (2,0), (3,0) into (0,0), (1,1), (2,2), (3,3).
				// I.e. the rotation matrix is not orthonormal/orthogonal. Ceil hopefully does this. TODO: test
				rot = ei::ceil(ei::rotation(PI / 3.0f * _ticks));
			} else {
				// Create the rotation matrix for the coordiantes.
				rot = ei::round(ei::rotation(PI / 2.0f * _ticks));
			}
			// Create a copy of the map and move the elements
			Grid<GridT, CellT> tmp;
			for(int j = 0; j < m_rows.size(); ++j)
			{
				Row& row = m_rows[j];
				for(int i = 0; i < row.size(); ++i)
				{
					tmp.set(rot * ei::IVec2(row.xpos[i], y + m_yPosition), std::move(row.cells[i]));
				}
			}
			std::swap(tmp, *this);
		}

		/// Compare occupied cells of _other and AND all the results.
		/// \details To implement an OR or some else return value you may use exceptions.
		///
		///		Empty rows or cells of any grid are skipped. If the two maps do not
		///		overlap the return value is true.
		template<typename T2>
		bool compare(const Grid<GridT,T2>& _other, std::function<bool(const CellT&, const T2&)> _comparator) const
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
		template<typename T2> // Recapture type for move sematic. T2 should be T in this case.
		void set(const ei::IVec2& _coord, T2&& _value)
		{
			// First element at all?
			if(m_rows.empty())
			{
				m_yPosition = _coord.y;
				m_rows.push_back(Row());
				m_rows.front().xpos.push_back(_coord.x);
				m_rows.front().cells.push_back(std::move(_value));
			}
			// Add rows on top (front of the vector)
			else if(_coord.y < m_yPosition)
			{
				for(int i = _coord.y; i < m_yPosition; ++i)
					m_rows.insert(m_rows.begin(), Row());
				m_yPosition = _coord.y;
				// We now have an empty row -> fill in the element
				m_rows.front().cells.push_back(std::move(_value));
				m_rows.front().xpos.push_back(_coord.x);
			}
			// Add rows at bottom (back of the vector)
			else if(_coord.y >= m_yPosition + (int)m_rows.size()) {
				for(int i = int(m_yPosition + m_rows.size()); i <= _coord.y; ++i)
					m_rows.push_back(Row());
				// We now have an empty row -> fill in the element
				m_rows.back().cells.push_back(std::move(_value));
				m_rows.back().xpos.push_back(_coord.x);
			}
			// There is already a row, insert the element to it
			else {
				auto& row = m_rows[_coord.y - m_yPosition];
				// Empty row?
				if(row.cells.size() == 0) {
					row.cells.push_back(std::move(_value));
					row.xpos.push_back(_coord.x);
				}
				// Insert in front of the other elements
				else if(_coord.x < row.xpos.front())
				{
					row.cells.insert(row.cells.begin(), std::move(_value));
					row.xpos.insert(row.xpos.begin(), _coord.x);
				}
				// Insert at the end
				else if(_coord.x > row.xpos.back())
				{
					row.cells.push_back(std::move(_value));
					row.xpos.push_back(_coord.x);
				}
				// Find the position with binary search
				else {
					int m;
					if(binSearch(row, _coord.x, m)) {
						row.cells[m] = std::move(_value);
					} else {
						// Not found -> insert at m or m+1
						if(_coord.x > row.xpos[m])
							++m;
						row.cells.insert(row.cells.begin() + m, std::move(_value));
						row.xpos.insert(row.xpos.begin() + m, _coord.x);
					}
				}
			}
		}

		/// Find out if a grid cell contains some element and returns an access pointer to it.
		CellT* find(const ei::IVec2& _coord)
		{
			// Check y-range if element is on map at all.
			if(_coord.y < m_yPosition) return nullptr;
			if(_coord.y >= m_yPosition + (int)m_rows.size()) return nullptr;
			
			auto& row = m_rows[_coord.y - m_yPosition];
			int m;
			if(binSearch(row, _coord.x, m))
				return &row.cells[m];
			// Not found = in some empty range
			return nullptr;
		}

		const CellT* find(const ei::IVec2& _coord) const
		{
			return const_cast<Grid*>(this)->find(_coord);
		}

		/// Get a cell. If it was not occupied before it gets filled with a default constructed
		/// T element.
		CellT& get(const ei::IVec2& _coord)
		{
			// First element at all?
			if(m_rows.empty())
			{
				m_yPosition = _coord.y;
				m_rows.push_back(Row());
				m_rows.front().xpos.push_back(_coord.x);
				m_rows.front().cells.resize(1);
				return m_rows.front().cells.front();
			}
			// Add rows on top (front of the vector)
			else if(_coord.y < m_yPosition)
			{
				for(int i = _coord.y; i < m_yPosition; ++i)
					m_rows.insert(m_rows.begin(), Row());
				m_yPosition = _coord.y;
				// We now have an empty row -> fill in the element
				m_rows.front().xpos.push_back(_coord.x);
				m_rows.front().cells.resize(1);
				return m_rows.front().cells.front();
			}
			// Add rows at bottom (back of the vector)
			else if(_coord.y >= m_yPosition + (int)m_rows.size()) {
				for(int i = int(m_yPosition + m_rows.size()); i <= _coord.y; ++i)
					m_rows.push_back(Row());
				// We now have an empty row -> fill in the element
				m_rows.back().xpos.push_back(_coord.x);
				m_rows.back().cells.resize(1);
				return m_rows.back().cells.front();
			}
			// There is already a row, insert the element to it
			else {
				auto& row = m_rows[_coord.y - m_yPosition];
				// Empty row?
				if(row.cells.size() == 0) {
					row.xpos.push_back(_coord.x);
					row.cells.resize(1);
					return row.cells.front();
				}
				// Insert in front of the other elements
				else if(_coord.x < row.xpos.front())
				{
					row.cells.insert(row.cells.begin(), std::move(CellT()));
					row.xpos.insert(row.xpos.begin(), _coord.x);
					return row.cells.front();
				}
				// Insert at the end
				else if(_coord.x > row.xpos.back())
				{
					row.cells.push_back(std::move(CellT()));
					row.xpos.push_back(_coord.x);
					return row.cells.back();
				}
				// Find the position with binary search
				else {
					int m;
					if(binSearch(row, _coord.x, m)) {
						return row.cells[m];
					} else {
						// Not found -> insert at m or m+1
						if(_coord.x > row.xpos[m])
							++m;
						row.cells.insert(row.cells.begin() + m, std::move(CellT()));
						row.xpos.insert(row.xpos.begin() + m, _coord.x);
						return row.cells[m];
					}					
				}
			}
		}

		/// Determines the minimal number of grid cells between two locations.
		/// This is the Manhatten-distance for quad-4 grids and something similar
		/// for hex grids.
		int gridDistance(const ei::IVec2& _a, const ei::IVec2& _b) const
		{
			if(GridT == unsigned(GridType::QUAD_4))
				return abs(_b.x - _a.x) + abs(_b.y - _a.y);
			else if(GridT == unsigned(GridType::QUAD_8))
				return max(abs(_b - _a));
			else
				return (abs(_a.x - _b.x) + abs(_a.x + _a.y - _b.x - _b.y) + abs(_a.y - _b.y)) / 2;
		}

		/// Iterator class which allows sequential access to all grid cells.
		class SeqIterator
		{
		public:
			// Pre increment
			SeqIterator& operator ++ ()
			{
				++m_col;
				if(m_gridRef.m_rows[m_row].cells.size() == m_col)
				{
					m_col = 0;
					++m_row;
				}
				return *this;
			}

			SeqIterator& operator -- ()
			{
				--m_col;
				if(uint(-1) == m_col)
				{
					--m_row;
					m_col = m_gridRef.m_rows[m_row].cells.size() - 1;
				}
				return *this;
			}

			// Post increment
			SeqIterator operator ++ (int) {
				SeqIterator copy = *this;
				++*this;
				return copy;
			}
			SeqIterator operator -- (int) {
				SeqIterator copy = *this;
				--*this;
				return copy;
			}

			/// Is this a valid iterator?
			operator bool () const
			{
				return (m_row < m_gridRef.m_rows.size()) &&
						(m_col < m_gridRef.m_rows[m_row].cells.size());
			}

			/// Access to the data (fails hard for invalid iterators)
			CellT& dat() { return m_gridRef.m_rows[m_row].cells[m_col]; }
			const CellT& dat() const { return m_gridRef.m_rows[m_row].cells[m_col]; }
			/// Get the coordinate of the current grid cell.
			ei::IVec2 pos() const { return ei::IVec2(m_gridRef.m_rows[m_row].xpos[m_col], m_gridRef.m_yPosition + m_row); }
		private:
			SeqIterator(Grid<GridT, CellT>& _gridRef) : m_gridRef(_gridRef), m_row(0), m_col(0) {}
			Grid<GridT, CellT>& m_gridRef;
			uint m_row, m_col;
			friend class Grid<GridT, CellT>;
		};

		SeqIterator begin() const { return SeqIterator(*this); }

		/// Iterator class for neighborhood searches.
		/// \details This iterator can be valid/invalid. In valid cases it always points
		///		to a non-empty cell.
		class NeighborIterator
		{
		public:
			/// Preincrement-like step to one of the neighbors.
			/// \details The number of valid neighbors in a QUAD grid is 8
			///		(0 = east, 1 = north east, 2 = north, ...) and 6 for HEX
			///		grids (0 = east, 1 = north east, 2 = north west, ...).
			NeighborIterator getNeighbor(uint n)
			{
				if(GridT == unsigned(GridType::QUAD_4))
				{
					int8 xoff[4] = {1, 0, -1, 0};
					int8 yoff[4] = {0, -1, 0, 1};
					return NeighborIterator(m_gridRef, m_row + yoff[n], m_col, m_x + xoff[n]);
				} else if(GridT == unsigned(GridType::QUAD_8))
				{
					int8 xoff[8] = {1, 1, 0, -1, -1, -1, 0, 1};
					int8 yoff[8] = {0, -1, -1, -1, 0, 1, 1, 1};
					return NeighborIterator(m_gridRef, m_row + yoff[n], m_col, m_x + xoff[n]);
				} else {
					switch(n) {
						case 0: return NeighborIterator(m_gridRef, m_row, m_col, m_x + 1); break;
						case 1: return NeighborIterator(m_gridRef, m_row - 1, m_col, m_x + 1); break;
						case 2: return NeighborIterator(m_gridRef, m_row - 1, m_col, m_x); break;
						case 3: return NeighborIterator(m_gridRef, m_row, m_col, m_x - 1); break;
						case 4: return NeighborIterator(m_gridRef, m_row + 1, m_col, m_x - 1); break;
						case 5: return NeighborIterator(m_gridRef, m_row + 1, m_col, m_x); break;
					}
				}
				return NeighborIterator(m_gridRef, 0x7fffffff, 0x7fffffff, 0x7fffffff);
			}

			/// Is this a valid iterator?
			operator bool () const
			{
				return (m_row < m_gridRef.m_rows.size())					// In row range?
						&& (m_col < m_gridRef.m_rows[m_row].cells.size())	// Inside the row?
						&& (m_gridRef.m_rows[m_row].xpos[m_col] == m_x);	// Points to a filled cell?
			}

			/// Access to the data (fails hard for invalid iterators)
		//	T& dat() { return m_gridRef.m_rows[m_row].cells[m_col]; }
			const CellT& dat() const { return m_gridRef.m_rows[m_row].cells[m_col]; }
			/// Get the coordinate of the current grid cell.
			ei::IVec2 pos() const { return ei::IVec2(m_x, m_gridRef.m_yPosition + m_row); }
		private:
			NeighborIterator(const Grid<GridT, CellT>& _gridRef, uint _row, uint _col, int _x) :
				m_gridRef(_gridRef),
				m_row(_row),
				m_col(_col),
				m_x(_x)
			{
				// Make sure _col and _x match if possible.
				// This assumes that we are always very close.
				if(m_row < m_gridRef.m_rows.size() && !m_gridRef.m_rows.empty())					// In row range?
				{
					auto& row = m_gridRef.m_rows[m_row];
					m_col = min(m_col, row.cells.size()-1);
					while(m_col > 0                && row.xpos[m_col] > _x) --m_col;
					while(m_col < row.cells.size() && row.xpos[m_col] < _x) ++m_col;
				}
			}

			const Grid<GridT, CellT>& m_gridRef;
			uint m_row, m_col;
			int m_x;
			friend class Grid<GridT, CellT>;
		};

		NeighborIterator beginNeighborhood(const ei::IVec2& _coord) const
		{
			int y = _coord.y - m_yPosition;
			if(y >= 0 && y < (int)m_rows.size())
			{
				int m;
				binSearch(m_rows[y], _coord.x, m);
				return NeighborIterator(*this, _coord.y - m_yPosition, m, _coord.x);
			} else
				return NeighborIterator(*this, _coord.y - m_yPosition, _coord.y, _coord.x);
		}

	protected:
		struct Row {
			// CRS (compressed row storage) like format.
			// Stores the x positions of each occupied entry and its content
			std::vector<int> xpos;
			std::vector<CellT> cells;
		};
		std::vector<Row> m_rows;

		// The position is a global reference position of the first tile of the first row.
		int m_yPosition;

		friend class SeqIterator;
		friend class NeighborIterator;

		// Search for a specific x-coordinate.
		// \param [out] _m The coordiate where the binary search stopped.
		// \returns true if the element was found.
		bool binSearch(const Row& _row, int _x, int& _m) const
		{
			int l = 0;
			int r = _row.xpos.size();
			if(_x < _row.xpos[l]) { _m = l; return false; }
			if(_x > _row.xpos[r-1]) { _m = r; return false; }
			// Binary search of the x-coordinate
			while(l+1 < r)
			{
				_m = (l + r) / 2;
				if(_row.xpos[_m] < _x) l = _m;
				else if(_row.xpos[_m] == _x) return true;
				else r = _m;
			}
			if(_row.xpos[l] == _x) { _m = 0; return true; }
			return false;
		}
	};
}} // namespace ca::map