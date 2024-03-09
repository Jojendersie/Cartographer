#pragma once

#include <ei/vector.hpp>
#include "ca/cc/core/manager.hpp"

namespace ca { namespace cc {
	
	enum class ShaderType
	{
		VERTEX			= 0x8B31,	///< GL_VERTEX_SHADER
		TESS_CONTROLL   = 0x8E88,	///< GL_TESS_CONTROL_SHADER
		TESS_EVALUATION = 0x8E87,	///< GL_TESS_EVALUATION_SHADER
		GEOMETRY		= 0x8DD9,	///< GL_GEOMETRY_SHADER
		FRAGMENT		= 0x8B30,	///< GL_FRAGMENT_SHADER
		COMPUTE 		= 0x91B9,	///< GL_COMPUTE_SHADER
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
		void use() const;
		
		/// Build program from the attached files.
		void link();

		/// Dispatch as compute shader (if it is one!). The group size
		/// is inferred from the shader itself and a sufficient thread count is scheduled.
		void dispatch(int _x, int _y, int _z);
		
		/// TODO: reflection and uniform stuff.

		/// Find the location of a uniform variable
		int getUniformLoc(const char* _uniformName);

		void setUniform(int _location, float _value);
		void setUniform(int _location, const ei::Vec2& _value);
		void setUniform(int _location, const ei::Vec3& _value);
		void setUniform(int _location, const ei::Vec4& _value);
		void setUniform(int _location, const ei::Mat4x4& _value);
		void setUniform(int _location, int _value);
		void setUniform(int _location, const ei::IVec2& _value);
		void setUniform(int _location, const ei::IVec3& _value);
		void setUniform(int _location, const ei::IVec4& _value);
		void setUniform(int _location, uint _value);
		void setUniform(int _location, const ei::UVec2& _value);
		void setUniform(int _location, const ei::UVec3& _value);
		void setUniform(int _location, const ei::UVec4& _value);

		/// Get OpenGL handle
		uint getID() const { return m_programID; }
	private:
		const Shader* m_shaders[5]; 	///< List of attached shaders
		int m_numShaders;
		uint m_programID;		///< OpenGL program ID
		ei::IVec3 m_group_size;
	};

}} // namespace ca::cc
