#include "core/error.hpp"
#include "core/scopedptr.hpp"
#include "glcore/opengl.hpp"
#include "glcore/shader.hpp"
#include <string>

namespace ca { namespace cc {
	
	Shader::Shader(const char* _source, ShaderType _type)
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

	Shader::Handle Shader::load(const char* _source, ShaderType _type, bool _isFileName)
	{
		if(_isFileName) {
			FILE* file = fopen(_source, "rb");
			if(!file) error(("Cannot open shader file: " + std::string(_source)).c_str());
			// Get file size and allocate memory
			fseek(file, 0, SEEK_END);
			unsigned fileLength = ftell(file);
			ScopedPtr<char> source(new char[fileLength + 1]);
			fseek(file, 0, SEEK_SET);
			// Read in whole file and make 0 terminated.
			fread(source, 1, fileLength, file);
			source[fileLength] = 0;
			return new Shader(source, _type);
		}
		return new Shader(_source, _type);
	}

	void Shader::unload(Shader::Handle _shader)
	{
		// The handle is defined as const, so nobody can do damage, but now we need
		// the real address for deletion
		delete const_cast<Shader*>(_shader);
	}


	
	Program::Program() :
		m_numShaders(0),
		m_programID(0)
	{
	}
	
	Program::~Program()
	{
		glCall(glDeleteProgram, m_programID);
	}
	
	void Program::attach(const Shader* _shader)
	{
		if(!m_programID)
			m_programID = glCall(glCreateProgram);
		if(m_numShaders < 5)
		{
			m_shaders[m_numShaders++] = _shader;
			glCall(glAttachShader, m_programID, _shader->m_shaderID);
		} else error("Already 5 shaders bound. This is the maximum: Vertex + Hull + Domain + Geometry + Fragment.");
	}
		
	void Program::use()
	{
		if(!m_programID)
			error("Trying to use an uninitialized program!");
		glCall(glUseProgram, m_programID);
	}
	
	void Program::link()
	{
		if(!m_programID)
			error("Trying to link an uninitialized program!");
		glCall(glLinkProgram, m_programID);

		GLint isLinked = 0;
		glCall(glGetProgramiv, m_programID, GL_LINK_STATUS, &isLinked);
		if(isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glCall(glGetProgramiv, m_programID, GL_INFO_LOG_LENGTH, &maxLength);
	 
			// The maxLength includes the NULL character
			std::string errorLog;
			errorLog.reserve(maxLength);
			glCall(glGetProgramInfoLog, m_programID, maxLength, &maxLength, &errorLog[0]);
			error(errorLog.c_str());

			glCall(glDeleteProgram, m_programID);
			m_programID = 0;
		}
	}

	int Program::getUniform(const char* _uniformName)
	{
		return glCall(glGetUniformLocation, m_programID, _uniformName);
	}
	
}} // namespace ca::cc