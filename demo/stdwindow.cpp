#include <ca/charcoal.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include "stdwindow.hpp"

using namespace ca::cc;
using namespace ca;

static float s_aspectRatio;
static int s_windowWidth, s_windowHeight;

static void errorCallbackGLFW(int error, const char* description)
{
	std::cerr << "GLFW error, code " << std::to_string(error) <<" desc: \"" << description << "\"" << std::endl;
}

static void stdKeyCallback(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods)
{
	if(_key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(_window, GL_TRUE);
}

static void resizeCallback(GLFWwindow* _window, int _width, int _height)
{
	s_aspectRatio = _width / (float)_height;
	s_windowWidth = _width;
	s_windowHeight = _height;
	glCall(glViewport, 0, 0, _width, _height);
}

GLFWwindow* setupStdWindow(const char* _titel, bool _vsync)
{
	glfwSetErrorCallback(errorCallbackGLFW);
	// Setup glfw
	if(!glfwInit()) {
		pa::logError("GLFW init failed");
		return nullptr;
	}

	// Create a context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 16);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);
#ifdef _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

	// Make full screen in debug
	int count;
	const GLFWvidmode* modes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);
	int width = modes[count-1].width;
	int height = modes[count-1].height;
//#ifdef _DEBUG
	width -= 30;
	height -= 70;
	GLFWwindow* window = glfwCreateWindow(width, height, _titel, nullptr, nullptr);
	glfwSetWindowPos(window, 10, 20);
//#else
//	GLFWwindow* window = glfwCreateWindow(width, height, _titel, glfwGetPrimaryMonitor(), nullptr);
//#endif
	s_windowWidth = width;
	s_windowHeight = height;
	s_aspectRatio = width / (float)height;
	glfwSetKeyCallback(window, stdKeyCallback);
	glfwSetWindowSizeCallback(window, resizeCallback);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(_vsync ? 1 : 0); // VSync

	/*glewExperimental = GL_TRUE;
	glewInit();
	glGetError();*/
	if (!gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress) ) {
		ca::pa::logError("gladLoadGLLoader failed to initialized GL-methods!");
		return window;
	} else ca::pa::logInfo("Initialized OpenGL functions with GLAD.");

	glEnable(GL_FRAMEBUFFER_SRGB);

	return window;
}

float getCurrentAspectRatio()
{
	return s_aspectRatio;
}

int getWindowWidth()
{
	return s_windowWidth;
}

int getWindowHeight()
{
	return s_windowHeight;
}