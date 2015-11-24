#include <cagui.hpp>
#include <charcoal.hpp>
#include <glcore/opengl.hpp>
#include <GLFW/glfw3.h>
#include "stdwindow.hpp"

void createGUI()
{
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

	createGUI();
	runMainLoop(window);

	glfwDestroyWindow(window);
	return 0;
}