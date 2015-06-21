#pragma once

#include <gl/glew.h>
#include <ei/elementarytypes.hpp>
#include "core/manager.hpp"

namespace MiR {
	
	enum class ShaderType
	{
		VERTEX			= GL_VERTEX_SHADER,
		TESS_CONTROLL   = GL_TESS_CONTROL_SHADER,
		TESS_EVALUATION = GL_TESS_EVALUATION_SHADER,
		GEOMETRY		= GL_GEOMETRY_SHADER,
		FRAGMENT		= GL_FRAGMENT_SHADER,
		COMPUTE 		= GL_COMPUTE_SHADER
	};

	/// Base class to load single shader files.
	/// \details This class also searches for include directives recursively.
	class Shader
	{
	public:
		Shader(ShaderType _type, const char* _source);
		
		~Shader();
		
	private:
		uint m_shaderID;	///< OpenGL shader ID.
		friend class Program;
	};
	
	typedef Manager<Shader> ShaderManager;
	
	/// Linked usable program with different shaders.
	class Program
	{
	public:
		/// Create empty program.
		Program();
		
		~Program();
		
		/// Attach one more shader. Not all combinations are valid.
		/// \details Validity is tested on linking.
		void attach(const Shader* _shader);
		
		/// Use this program now, links if necessary.
		void use();
		
		/// Build program from the attached files.
		void link();
		
		/// TODO: reflection and uniform stuff.
	private:
		const Shader* m_shaders[5]; 	///< List of attached shaders
		int m_numShaders;
		uint m_programID;		///< OpenGL program ID
	};

} // namespace MiR