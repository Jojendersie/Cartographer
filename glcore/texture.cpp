#include "texture.hpp"

namespace MiR {

	bool TextureAtlas::allocate(int _width, int _height, Entry& _location)
	{
		if(_width > m_width || _height > m_height)
		{
			error("The texture size exceeds the atlas size.");
			return false;
		}
		int w = m_width;
		int h = m_height;
		
		// Search in all layers
		for(int l=0; l<m_depth; ++l)
		{
			if((m_width >> m_maxFreeLevel[l]) >= _width &&
			   (m_height >> m_maxFreeLevel[l]) >= _height)
		   {
				// Enough space in this layer.
				
				return true;
		   }
		}
		
		// No layer had enough space, weed need a new one.
		resize();
		return allocate(_width, _height, _location);
	}

} // namespace MiR