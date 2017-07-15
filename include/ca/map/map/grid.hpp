#pragma once

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
	class Grid
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
		template<typename CellT2>
		bool compare(const Grid<GridT,CellT2>& _other, std::function<bool(const CellT&, const CellT2&)> _comparator) const
		{
			bool res = true;
			// Iterate over a minimal range in y
			int minY = max(_other.m_yPosition, m_yPosition);
			int maxY = min(_other.m_yPosition + _other.m_rows.size(), m_yPosition + m_rows.size());
			for(int y = minY; y < maxY; ++y)
			{
				auto& rowA = m_rows[y - m_yPosition];
				auto& rowB = _other.m_rows[y - _other.m_yPosition];
				int minX = max(rowA.xmin, rowB.xmin);
				int maxX = min(rowA.xmax(), rowB.xmax());
				for(int x = minX; x < maxX; ++x)
					res &= _comparator(rowA.cells[x-rowA.xmin], rowB.cells[x-rowB.xmin]);
			}
			return res;
		}

		/// Set/overwrite any grid cell.
		template<typename T2> // Recapture type for move sematic. T2 should be T in this case.
		void set(const GridCoord& _coord, T2&& _value)
		{
			get(_coord) = std::move(_value);
			// First element at all?
			/*if(m_rows.empty())
			{
				m_yPosition = _coord.y;
				m_rows.push_back(Row());
				m_rows.xmin = _coord.x;
				m_rows.front().cells.push_back(std::move(_value));
			} else {
				// Insert empty rows at end or beginning if necessray
				if(_coord.y < m_yPosition || _coord.y >= m_yPosition + (int)m_rows.size())
				{
					int newY = min(_coord.y, m_yPosition);
					// Add a range of row by simple reservation
					std::vector<Row> tmpRows(max(_coord.y, m_yPosition + (int)m_rows.size()) - newY + 1);
					// Move the old rows
					int yOff = max(0, m_yPosition - newY);
					for(size_t i = 0; i < m_rows.size(); ++i)
						tmpRows[i+yOff] = std::move(m_rows[i]);
					// Use the new rows
					std::swap(m_rows, tmpRows);
					m_yPosition = newY;
				}

				auto& row = m_rows[_coord.y - m_yPosition];
				if(row.empty)
				{
					row.xmin = _coord.x;
					row.cells.push_back(std::move(_value));
				} else {
					// Need to allocate?
					if(_coord.x < row.xmin || _coord.x >= row.xmin + int(row.cells.size()))
					{
						int newX = min(_coord.x, row.xmin);
						Row tmpRow;
						int xOff = max(0, row.xmin - newX);
						for(size_t i = 0; i < row.cells.size(); ++i)
							tmpRow.cells[i+xOff] = std::move(row.cells[i]);
						tmpRow.xmin = newX;
						std::swap(row, tmpRow);
					}
					// Overwrite
					row[_coord.x - row.xmin] = std::move(_value);
				}
			}*/
		}

		/// Find out if a grid cell contains some element and returns an access pointer to it.
		CellT* find(const GridCoord& _coord)
		{
			// Check y-range if element is on map at all.
			if(_coord.y < minY()) return nullptr;
			if(_coord.y >= maxY()) return nullptr;
			
			auto& row = m_rows[_coord.y - m_yPosition];
			if(_coord.x < row.minX()) return nullptr;
			if(_coord.x >= row.maxX()) return nullptr;
			return &row.cells[_coord.x - row.xmin];
		}

		const CellT* find(const ei::IVec2& _coord) const
		{
			return const_cast<Grid*>(this)->find(_coord);
		}

		/// Get a cell. If it was not occupied before it gets filled with a default constructed
		/// T element.
		CellT& get(const GridCoord& _coord)
		{
			// First element at all?
			if(m_rows.empty())
			{
				m_yPosition = _coord.y;
				m_rows.push_back(Row(1));
				m_rows.front().xmin = _coord.x;
				return m_rows.front().cells[0];
			} else {
				// Insert empty rows at end or beginning if necessray
				if(_coord.y < m_yPosition || _coord.y >= m_yPosition + (int)m_rows.size())
				{
					int newY = ei::min(_coord.y, m_yPosition);
					// Add a range of row by simple reservation
					std::vector<Row> tmpRows(ei::max(_coord.y, m_yPosition + (int)m_rows.size()) - newY + 1, Row(0));
					// Move the old rows
					int yOff = ei::max(0, m_yPosition - newY);
					for(size_t i = 0; i < m_rows.size(); ++i)
						tmpRows[i+yOff] = std::move(m_rows[i]);
					// Use the new rows
					std::swap(m_rows, tmpRows);
					m_yPosition = newY;
				}

				Row& row = m_rows[_coord.y - m_yPosition];
				if(row.cells.empty())
				{
					row.xmin = _coord.x;
					row.cells.resize(1);
					return row.cells[0];
				} else {
					// Need to allocate?
					if(_coord.x < row.minX() || _coord.x >= row.maxX())
					{
						int newX = ei::min(_coord.x, row.minX());
						Row tmpRow(ei::max(_coord.x, row.maxX()) - newX + 1);
						int xOff = ei::max(0, row.xmin - newX);
						for(size_t i = 0; i < row.cells.size(); ++i)
							tmpRow.cells[i+xOff] = std::move(row.cells[i]);
						tmpRow.xmin = newX;
						std::swap(row, tmpRow);
					}
					return row.cells[_coord.x - row.xmin];
				}
			}
		}

		/// Remove empty rows and empty cells at the begin/end of a row.
		/// \details This cannot remove cells incide a row!
		/// \param [in] _isEmpty Predicate function to check if a cell is not needed/empty.
		void clean(std::function<bool(CellT&)> _isEmpty)
		{
			// Clean up begin and endings of rows.
			for(Row& row : m_rows)
			{
				// Count until first non-empty.
				size_t numEmptyFront = 0;
				for(; numEmptyFront < row.cells.size(); ++numEmptyFront)
					if(!_isEmpty(row.cells[numEmptyFront]))
						break;

				// Count again form the end, if there is still some element (juding at i).
				size_t numEmptyBack = 0;
				for(int i = int(row.cells.size())-1; i > int(numEmptyFront); --i)
				{
					if(_isEmpty(row.cells[i]))
						++numEmptyBack;
					else break;
				}

				// Move the non-empty range to a new thickly fitted row.
				size_t numNonEmpty = row.cells.size() - numEmptyFront - numEmptyBack;
				Row tmpRow(numNonEmpty);
				for(size_t i = numEmptyFront; i < row.cells.size() - numEmptyBack; ++i)
					tmpRow.cells[i - numEmptyFront] = std::move(row.cells[i]);
				tmpRow.xmin = row.xmin + numEmptyFront;
				std::swap(row, tmpRow);
			}

			// Remove empty rows the same way.
			size_t numEmptyFront = 0;
			for(; numEmptyFront < m_rows.size(); ++numEmptyFront)
				if(!m_rows[numEmptyFront].cells.empty())
					break;
			size_t numEmptyBack = 0;
			for(int i = int(m_rows.size())-1; i > int(numEmptyFront); --i)
			{
				if(m_rows[i].cells.empty())
					++numEmptyBack;
				else break;
			}

			size_t numNonEmpty = m_rows.size() - numEmptyFront - numEmptyBack;
			std::vector<Row> tmpRows(numNonEmpty, Row(0));
			for(size_t i = numEmptyFront; i < m_rows.size() - numEmptyBack; ++i)
				tmpRows[i - numEmptyFront] = std::move(m_rows[i]);
			std::swap(m_rows, tmpRows);
			m_yPosition += numEmptyFront;
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
			GridCoord coord() const { return GridCoord(m_gridRef.m_rows[m_row].xmin + m_col, m_gridRef.m_yPosition + m_row); }
		private:
			SeqIterator(Grid<GridT, CellT>& _gridRef) : m_gridRef(_gridRef), m_row(0), m_col(0) {}
			Grid<GridT, CellT>& m_gridRef;
			uint m_row, m_col;
			friend class Grid<GridT, CellT>;
		};

		SeqIterator begin() const { return SeqIterator(*this); }

		int minY() const { return m_yPosition; }
		int maxY() const { return m_yPosition + m_rows.size(); }
	protected:
		struct Row {
			int maxX() const { return xmin + cells.size(); }
			int minX() const { return xmin; }
			std::vector<CellT> cells;

			Row(size_t _reserveCells) : xmin(0), cells(_reserveCells)
			{}
		private:
			int xmin;
			friend Grid<GridT, CellT>;
		};
		std::vector<Row> m_rows;

		// The position is a global reference position of the first tile of the first row.
		int m_yPosition;

		friend class SeqIterator;
	};
}}
