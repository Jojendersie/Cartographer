#pragma once

#include <gl/glew.h>
#include <ei/elementarytypes.hpp>
#include "core/manager.hpp"

namespace ca { namespace cc {
	
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
		typedef const Shader* Handle;

		~Shader();

		static Handle load(const char* _source, ShaderType _type, bool _isFileName = true);

		static void unload(Handle _shader);
		
	private:
		/// Construction to create a shader from source code.
		///
		/// This is private to allow only load() to be used
		Shader(const char* _source, ShaderType _type);

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

		/// Find the location of a uniform variable
		int getUniform(const char* _uniformName);
	private:
		const Shader* m_shaders[5]; 	///< List of attached shaders
		int m_numShaders;
		uint m_programID;		///< OpenGL program ID
	};

}} // namespace ca::cc
