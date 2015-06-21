#include "texture.hpp"
#include "core/error.hpp"
#include <memory.h>

namespace MiR {

	TextureAtlas::TextureAtlas(int _maxWidth, int _maxHeight) :
		m_width(_maxWidth),
		m_height(_maxHeight),
		m_quadTree(new uint8[341]) // Allocate fixed size quad tree
	{
		// Mark all nodes in the quad tree as empty
		memset(m_quadTree, 0, TREE_SIZE);
	}

	bool TextureAtlas::allocate(int _width, int _height, Entry& _location)
	{
		if(_width > m_width || _height > m_height)
		{
			error("The texture size exceeds the atlas size.");
			return false;
		}

		// Search in all layers
		for(int l=0; l<m_numRoots; ++l)
		{
			int w = m_width >> m_quadTree[uint(l * TREE_SIZE)];
			int h = m_height >> m_quadTree[uint(l * TREE_SIZE)];
			if(w >= _width && h >= _height)
			{
				// Enough space in this subtree.
				// Now use tree recursion to find the tile itself.
				_location.tileSize = 0;
				while((m_width >> _location.tileSize) > _width && (m_height >> _location.tileSize) > _height)
					_location.tileSize++;
				_location.texLayer = l;
				_location.texCoordX = 0;
				_location.texCoordY = 0;
				recursiveAllocate(uint(l * TREE_SIZE), 0, m_width, m_height, _location);
				
				return true;
			}
		}
		
		// No layer had enough space, weed need a new one.
		resize();
		return allocate(_width, _height, _location);
	}

	bool TextureAtlas::recursiveAllocate(uint _off, uint _idx, int _w, int _h, Entry& _location)
	{
		// Check if current level is finest resolution.
		int tileW = m_width >> _location.tileSize;
		int tileH = m_height >> _location.tileSize;
		if(_w == tileW || _h == tileH)
		{
			m_quadTree[_off+_idx] = 255;	// Mark es fully occupied
			// The parents already set full location information
			return true;
		} else {
			// Check the 4 children
			bool ret;
			uint childidx = _idx * 4 + 1; // heap order
			if((m_width >> m_quadTree[_off+childidx]) >= tileW && (m_height >> m_quadTree[_off+childidx]) >= tileH) {
				ret = recursiveAllocate(_off, childidx, _w/2, _h/2, _location);
			} else if((m_width >> m_quadTree[_off+childidx+1]) >= tileW && (m_height >> m_quadTree[_off+childidx+1]) >= tileH) {
				_location.texCoordX += _w/2;
				ret = recursiveAllocate(_off, childidx+1, _w/2, _h/2, _location);
			} else if((m_width >> m_quadTree[_off+childidx+2]) >= tileW && (m_height >> m_quadTree[_off+childidx+2]) >= tileH) {
				_location.texCoordY += _h/2;
				ret = recursiveAllocate(_off, childidx+2, _w/2, _h/2, _location);
			} else if((m_width >> m_quadTree[_off+childidx+3]) >= tileW && (m_height >> m_quadTree[_off+childidx+3]) >= tileH) {
				_location.texCoordX += _w/2;
				_location.texCoordY += _h/2;
				ret = recursiveAllocate(_off, childidx+3, _w/2, _h/2, _location);
			} else {
				// Error in allocation (current node to large and none of the children is large enough)
				return false;
			}

			// Update current max-free from the children
			m_quadTree[_off+_idx] = ei::min(m_quadTree[_off + childidx],
											m_quadTree[_off + childidx + 1],
											m_quadTree[_off + childidx + 2],
											m_quadTree[_off + childidx + 3]);
			return ret;
		}
	}

} // namespace MiR