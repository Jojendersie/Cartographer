#include <mir.hpp>
#include <iostream>
#include <string>
#include <GLFW/glfw3.h>
#include <GL/glew.h>

using namespace MiR;

static InstanceRenderer* s_renderer;

static void errorCallbackGLFW(int error, const char* description)
{
	std::cerr << "GLFW error, code " << std::to_string(error) <<" desc: \"" << description << "\"" << std::endl;
}

void prepareBoxes()
{
	// Create quad renderer with positions and normals
	VertexAttribute attr[2] = {{PrimitiveFormat::FLOAT, 3, false},
							   {PrimitiveFormat::FLOAT, 3, false}};
	s_renderer = new InstanceRenderer(InstanceRenderer::PrimitiveType::QUADS, attr, 2);
	//s_renderer.beginDef();
}

void renderBoxes()
{
}

int main()
{
	glfwSetErrorCallback(errorCallbackGLFW);
	// Setup glfw
	if(!glfwInit()) {
		error("GLFW init failed");
		return 1;
	}

	// Create a context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 16);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

	// Make full screen in debug
	int count;
	const GLFWvidmode* modes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);
	int width = modes[count-1].width;
	int height = modes[count-1].height;
#ifdef _DEBUG
	width -= 30;
	height -= 70;
	GLFWwindow* window = glfwCreateWindow(width, height, "MiR cube demo.", nullptr, nullptr);
	glfwSetWindowPos(window, 10, 10);
#else
	GLFWwindow* window = glfwCreateWindow(width, height, "MiR cube demo.", glfwGetPrimaryMonitor(), nullptr);
#endif

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // VSync

	glewExperimental = GL_TRUE;
	glewInit();
	glGetError();

	glfwDestroyWindow(window);
	return 0;
}