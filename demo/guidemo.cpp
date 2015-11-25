#include <cagui.hpp>
#include <charcoal.hpp>
#include <glcore/opengl.hpp>
#include <GLFW/glfw3.h>
#include "stdwindow.hpp"
#include <memory>

using namespace ei;
using namespace ca::gui;

std::shared_ptr<ca::gui::FlatTheme> g_flatTheme;

void createGUI(GLFWwindow* _window)
{
	// Create an instance for all themes
	FlatProperties themeProps;
	themeProps.backgroundColor = Vec4(0.05f, 0.05f, 0.05f, 1.0f);
	themeProps.foregroundColor = Vec4(0.25f, 0.25f, 0.25f, 1.0f);
	themeProps.textColor = Vec4(0.9f, 0.9f, 0.75f, 1.0f);
	themeProps.textBackColor = Vec4(0.01f, 0.01f, 0.01f, 1.0f);
	themeProps.hoverBackgroundColor = Vec4(0.15f, 0.15f, 0.15f, 1.0f);
	themeProps.hoverTextColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	themeProps.textSize = 12.0f;
	g_flatTheme = std::make_shared<FlatTheme>(themeProps);

	// Initialize GUI system itself
	int w, h;
	glfwGetFramebufferSize(_window, &w, &h);
	GUIManager::init(
		std::make_shared<CharcoalBackend>("calibri.caf"),
		g_flatTheme,
		w, h );

	// Create GUI-elements
	// A resizeable frame with different anchored buttons
	FramePtr f0 = std::make_shared<Frame>(true, false, true, true);
	f0->setExtent(Vec2(5.0f), Vec2(100.0f));
	GUIManager::add(f0);

	ButtonPtr b0 = std::make_shared<Button>();
	b0->setExtent(f0->getPosition() + Vec2(10.0f), Vec2(80.0f, 20.0f));
	b0->setText("Test0");
	f0->add(b0);

	ButtonPtr b1 = std::make_shared<Button>();
	b1->setExtent(f0->getPosition() + Vec2(10.0f, 35.0f), Vec2(80.0f, 20.0f));
	b1->setText("Test1");
	f0->add(b1);
}

void runMainLoop(GLFWwindow* _window)
{
	HRClock clock;
	while(!glfwWindowShouldClose(_window))
	{
		float deltaTime = (float)clock.deltaTime();
		glfwPollEvents();
		ca::cc::glCall(glClearColor, 0.01f, 0.01f, 0.01f, 1.0f);
		ca::cc::glCall(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ca::gui::GUIManager::draw();

		//std::this_thread::sleep_for(std::chrono::milliseconds(1));
		glfwSwapBuffers(_window);
	}
}

int main()
{
	GLFWwindow* window = setupStdWindow("Carthographer GUI demo.", false);
	if(!window) return 1;

	createGUI(window);
	runMainLoop(window);

	glfwDestroyWindow(window);
	return 0;
}