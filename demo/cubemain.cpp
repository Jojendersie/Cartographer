#include <mir.hpp>
#include <iostream>
#include <string>
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <ei/vector.hpp>

using namespace MiR;
using namespace ei;

static InstanceRenderer* s_renderer;
static Program s_shader;
static float s_aspectRatio;
static int s_ViewProjLocation;

static void errorCallbackGLFW(int error, const char* description)
{
	std::cerr << "GLFW error, code " << std::to_string(error) <<" desc: \"" << description << "\"" << std::endl;
}

void prepareBoxes()
{
	// Create quad renderer with positions and normals
	VertexAttribute attr[2] = {{PrimitiveFormat::FLOAT, 3, false},
							   {PrimitiveFormat::FLOAT, 3, false}};
	s_renderer = new InstanceRenderer(InstanceRenderer::PrimitiveType::TRIANGLE_STRIPE, attr, 2);
	int meshID = s_renderer->beginDef();
		s_renderer->put(1, Vec3(-1.0f,  0.0f,  0.0f));
		s_renderer->put(0, Vec3(-1.0f, -1.0f, -1.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3(-1.0f, -1.0f,  1.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3(-1.0f,  1.0f, -1.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3(-1.0f,  1.0f,  1.0f)); s_renderer->emit();
		s_renderer->endPrimitive();

		s_renderer->put(1, Vec3( 1.0f,  0.0f,  0.0f));
		s_renderer->put(0, Vec3( 1.0f, -1.0f, -1.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3( 1.0f, -1.0f,  1.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3( 1.0f,  1.0f, -1.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3( 1.0f,  1.0f,  1.0f)); s_renderer->emit();
		s_renderer->endPrimitive();

		s_renderer->put(1, Vec3( 0.0f, -1.0f,  0.0f));
		s_renderer->put(0, Vec3(-1.0f, -1.0f, -1.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3(-1.0f, -1.0f,  1.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3( 1.0f, -1.0f, -1.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3( 1.0f, -1.0f,  1.0f)); s_renderer->emit();
		s_renderer->endPrimitive();

		s_renderer->put(1, Vec3( 0.0f,  1.0f,  0.0f));
		s_renderer->put(0, Vec3(-1.0f,  1.0f, -1.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3(-1.0f,  1.0f,  1.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3( 1.0f,  1.0f, -1.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3( 1.0f,  1.0f,  1.0f)); s_renderer->emit();
		s_renderer->endPrimitive();

		s_renderer->put(1, Vec3( 0.0f,  0.0f, -1.0f));
		s_renderer->put(0, Vec3( 1.0f, -1.0f, -1.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3( 1.0f,  1.0f, -1.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3(-1.0f, -1.0f, -1.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3(-1.0f,  1.0f, -1.0f)); s_renderer->emit();
		s_renderer->endPrimitive();

		s_renderer->put(1, Vec3( 0.0f,  0.0f,  1.0f));
		s_renderer->put(0, Vec3( 1.0f, -1.0f,  1.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3( 1.0f,  1.0f,  1.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3(-1.0f, -1.0f,  1.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3(-1.0f,  1.0f,  1.0f)); s_renderer->emit();
		s_renderer->endPrimitive();
	s_renderer->endDef();
	s_renderer->newInstance(meshID, Vec3(0.0f), qidentity());
}

void prepareShader()
{
	// Load shader from file
	s_shader.attach( MiR::ShaderManager::get("shader/instanced3d.vert", ShaderType::VERTEX) );
	s_shader.attach( MiR::ShaderManager::get("shader/simple.frag", ShaderType::FRAGMENT) );
	s_shader.link();

	// Set a fixed camera
	s_ViewProjLocation = s_shader.getUniform("c_viewProjection");
}

void runMainLoop(GLFWwindow* _window)
{
	float time = 0.0f;
	Device::setCullMode(CullMode::NONE);
	Device::setZFunc(ComparisonFunc::LESS);
	while(!glfwWindowShouldClose(_window))
	{
		time += 0.01f;
		glCall(glClearColor, 0.0f, 0.0f, 0.0f, 0.0f);
		glCall(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		s_shader.use();
		Mat4x4 viewProj = ei::perspectiveGL(PI/4, s_aspectRatio, 0.1f, 100.0f)
			* ei::camera(rotationY(time) * Vec3(0.0f, 4.0f, -6.0f), Vec3(0.0f));
		glCall(glUniformMatrix4fv, s_ViewProjLocation, 1, GL_FALSE, (GLfloat*)&viewProj);

		s_renderer->draw();

		glfwSwapBuffers(_window);
		glfwPollEvents();
	}
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
	glfwSetWindowPos(window, 10, 20);
#else
	GLFWwindow* window = glfwCreateWindow(width, height, "MiR cube demo.", glfwGetPrimaryMonitor(), nullptr);
#endif
	s_aspectRatio = width / (float)height;

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // VSync

	glewExperimental = GL_TRUE;
	glewInit();
	glGetError();

	prepareBoxes();
	prepareShader();
	runMainLoop(window);

	glfwDestroyWindow(window);
	return 0;
}