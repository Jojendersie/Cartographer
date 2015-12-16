#include <charcoal.hpp>
#include <glcore/opengl.hpp>
#include <thread>
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <iostream>
#include "stdwindow.hpp"

using namespace ca::cc;
using namespace ei;

static FontRenderer* s_fontRenderer;
static Program s_shader;

void prepareFont()
{
	double times[3];
	HRClock clock;
	s_fontRenderer = new FontRenderer;
	s_fontRenderer->createFont("calibri.ttf", u8" 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz,.:!?'~+-*/(){}[]<>\U000003B5\U000003A9\U0000262F\U00002713");
	times[0] = clock.deltaTime();
	s_fontRenderer->storeCaf("calibri.caf");
	times[1] = clock.deltaTime();
	s_fontRenderer->loadCaf("calibri.caf");
	times[2] = clock.deltaTime();
	std::cout << "Time to create font: " << times[0] << " ms\n";
	std::cout << "Time to store font: " << times[1] << " ms\n";
	std::cout << "Time to load font: " << times[2] << " ms\n";
}

void prepareShader(GLFWwindow* _window)
{
	// Load font shader from file
	s_shader.attach( ShaderManager::get("shader/font.vert", ShaderType::VERTEX) );
	s_shader.attach( ShaderManager::get("shader/font.geom", ShaderType::GEOMETRY) );
	s_shader.attach( ShaderManager::get("shader/font.frag", ShaderType::FRAGMENT) );
	s_shader.link();

	int w, h;
	glfwGetFramebufferSize(_window, &w, &h);
	Mat4x4 viewProj = ei::orthographicGL(0.0f, (float)w, 0.0f, (float)h, 0.0f, 1.0f);
	s_shader.setUniform(0, viewProj);
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
		s_fontRenderer->clearText();
		Vec3 pos(1.0f + (float)sin(clock.now()/1000.0), 1.0f + (float)cos(clock.now()/1000.0), 0.0f);
		//Vec3 pos(1.0f, 1.0f, 0.0f);
		float size = 68.5f;
		for(int i=0; i<25; ++i)
		{
			s_fontRenderer->draw(pos, u8"The quick onyx goblin jumps over the lazy dwarf. [Hq](AV){}<>;?!\U000003B5\U000003A9\U0000262F", size, Vec4(1.0), 0.0f, 0.0f, 0.0f, false);
			pos.y += 4.0f + size;
			size -= 2.5f;
		}
		size = 68.5f;
		pos = Vec3(1880.0f + (float)sin(clock.now()/1000.0), 1120.0f + (float)cos(clock.now()/1000.0), 0.0f);
		for(int i=0; i<23; ++i)
		{
			s_fontRenderer->draw(pos, u8"0123456789", size, Vec4(0.5f, 0.5f, 0.4f, 1.0f), 0.0f, 1.0f, 1.0f);
			pos.y -= 4.0f + size;
			size -= 3.0f;
		}
		s_fontRenderer->draw(Vec3(1400.0f, 550.0f, 0.0f), u8"\U00002713", 200.0f, Vec4(0.0f, 0.5f, 0.0f, 1.0f));
		s_fontRenderer->draw(Vec3(1430.0f, 520.0f, 0.0f), u8"\U00002713", 200.0f, Vec4(0.0f, 0.5f, 0.0f, 0.5f + 0.5f*(float)sin(clock.now()/1000.0)));
		s_fontRenderer->draw(Vec3(1270.0f, 900.0f, 0.0f), u8"(-O-)", 200.0f, Vec4(0.0f, 0.0f, 0.5f, 1.0f), float(clock.now()/1000.0), 0.46f, 0.45f);
		s_fontRenderer->draw(Vec3(1270.0f, 900.0f, 0.0f), u8"(-O-)", 8.0f, Vec4(0.0f, 0.0f, 0.5f, 1.0f), float(clock.now()/1000.0), 0.46f, 0.45f);

		s_fontRenderer->draw(Vec3(870.0f, 1000.0f, 0.0f), u8"Multiline\nexample 1", 16.0f, Vec4(0.5f, 0.5f, 0.8f, 1.0f), 0.0f, 0.0f, 0.0f, true);
		s_fontRenderer->draw(Vec3(870.0f, 1000.0f, 0.0f), u8"Multiline\nexample 2", 16.0f, Vec4(0.5f, 0.5f, 0.8f, 1.0f), 0.0f, 1.0f, 0.0f, true);
		s_fontRenderer->draw(Vec3(870.0f, 1000.0f, 0.0f), u8"Multiline\nexample 3\nextra\nlong", 16.0f, Vec4(0.5f, 0.5f, 0.8f, 1.0f), 0.0f, 0.0f, 1.0f, true);
		s_fontRenderer->draw(Vec3(870.0f, 1000.0f, 0.0f), u8"Multiline\nexample 4", 16.0f, Vec4(0.5f, 0.5f, 0.8f, 1.0f), PI*0.5f, 1.0f, 0.0f, true);
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