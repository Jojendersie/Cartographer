#include <charcoal.hpp>
#include <thread>
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include "stdwindow.hpp"

using namespace cac;
using namespace ei;

static FontRenderer* s_fontRenderer;
static Program s_shader;

void prepareFont()
{
	s_fontRenderer = new FontRenderer;
	s_fontRenderer->createFont("calibri.ttf", u8"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz,.:!?'(){}[]<>???????????");
}

void prepareShader(GLFWwindow* _window)
{
	// Load font shader from file
	s_shader.attach( ShaderManager::get("shader/font.vert", ShaderType::VERTEX) );
	s_shader.attach( ShaderManager::get("shader/font.geom", ShaderType::GEOMETRY) );
	s_shader.attach( ShaderManager::get("shader/font.frag", ShaderType::FRAGMENT) );
	s_shader.link();

	s_shader.use();
	int w, h;
	glfwGetFramebufferSize(_window, &w, &h);
	Mat4x4 viewProj = ei::orthographicGL(0.0f, (float)w, 0.0f, (float)h, 0.0f, 1.0f);
	glCall(glUniformMatrix4fv, 0, 1, GL_FALSE, (GLfloat*)&viewProj);
}

void runMainLoop(GLFWwindow* _window)
{
	Device::setCullMode(CullMode::BACK);
	Device::setZFunc(ComparisonFunc::LEQUAL);
	HRClock clock;
	while(!glfwWindowShouldClose(_window))
	{
		float deltaTime = (float)clock.deltaTime();
		glfwPollEvents();
		glCall(glClearColor, 0.01f, 0.01f, 0.01f, 1.0f);
		glCall(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		s_shader.use();
		s_fontRenderer->draw(Vec3(), "");
		s_fontRenderer->present();

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		glfwSwapBuffers(_window);
	}
}

int main()
{
	GLFWwindow* window = setupStdWindow("Carthographer Charcoal text demo.", false);
	if(!window) return 1;

	prepareFont();
	prepareShader(window);
	runMainLoop(window);

	delete s_fontRenderer;
	glfwDestroyWindow(window);
	return 0;
}