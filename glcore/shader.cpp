#include "core/error.hpp"
#include "opengl.hpp"
#include "shader.hpp"

namespace MiR {
	
	Shader::Shader(ShaderType _type, const char* _fileName)
	{
		m_shaderID = glCall(glCreateShader, uint(_type));
		int len = (int)strlen(_source);
		glCall(glShaderSource, m_shaderID, 1, &_source, &len);
		glCall(glCompileShader, m_shaderID);
		GLint isCompiled = 0;
		glCall(glGetShaderiv, m_shaderID, GL_COMPILE_STATUS, &isCompiled);
		if(isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glCall(glGetShaderiv, m_shaderID, GL_INFO_LOG_LENGTH, &maxLength);
	 
			// The maxLength includes the NULL character
			std::string errorLog;
			errorLog.reserve(maxLength);
			glCall(glGetShaderInfoLog, m_shaderID, maxLength, &maxLength, &errorLog[0]);
			error(errorLog.c_str());
			glCall(glDeleteShader, m_shaderID);
			return;
		}
	}
	
	Shader::~Shader()
	{
		glCall(glDeleteShader, m_shaderID);
	}
	
	Program::Program() :
		m_numShaders(0)
	{
		m_programID = glCall(glCreateProgram);
	}
	
	Program::~Program()
	{
		glCall(glDeleteProgram, m_programID);
	}
	
	void Program::attach(const Shader* _shader)
	{
		if(m_numShaders < 5)
		{
			m_shader[m_numShaders++] = _shader;
			glCall(glAttachShader, m_program, _shader->m_shaderID);
		} else error("Already 5 shaders bound. This is the maximum: Vertex + Hull + Domain + Geometry + Fragment.");
	}
		
	void Program::use()
	{
		glCall(glUseProgram, m_program);
	}
	
	void Program::link()
	{
		glCall(glLinkProgram(m_program));

		GLint isLinked = 0;
		glCall(glGetProgramiv, m_program, GL_LINK_STATUS, &isLinked);
		if(isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glCall(glGetProgramiv, m_program, GL_INFO_LOG_LENGTH, &maxLength);
	 
			// The maxLength includes the NULL character
			std::string errorLog;
			errorLog.reserve(maxLength);
			glCall(glGetProgramInfoLog, m_program, maxLength, &maxLength, &errorLog[0]);
			error(errorLog.c_str());
		}
	}
	
} // namespace Ca