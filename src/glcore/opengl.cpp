#include "core/error.hpp"
#include "glcore/opengl.hpp"

#include <string>

namespace ca { namespace cc {
		
	bool GLError(const char*  _openGLFunctionName)
	{
		GLenum Error = glGetError();
		if (Error != GL_NO_ERROR)
		{
			const char* errorString;
			const char* description;
			switch (Error)
			{
			case GL_INVALID_ENUM:
				errorString = "GL_INVALID_ENUM";
				description = "An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag.";
				break;

			case GL_INVALID_VALUE:
				errorString = "GL_INVALID_VALUE";
				description = "A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.";
				break;

			case GL_INVALID_OPERATION:
				errorString = "GL_INVALID_OPERATION";
				description = "The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.";
				break;

			case GL_INVALID_FRAMEBUFFER_OPERATION:
				errorString = "GL_INVALID_FRAMEBUFFER_OPERATION";
				description = "The command is trying to render to or read from the framebuffer while the currently bound framebuffer is not framebuffer complete. "
					"The offending command is ignored and has no other side effect than to set the error flag.";
				break;

			case GL_OUT_OF_MEMORY:
				errorString = "GL_OUT_OF_MEMORY";
				description = "There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
				break;

			default:
				errorString = "UNKNOWN";
				description = "Unknown error code.";
				break;
			}
			error((std::string("OpenGL Error during ") + _openGLFunctionName + ": " + errorString + "(" + description + ")").c_str());
			return true;
		}

		// No error
		return false;
	}
	
}} // namespace ca::cc
