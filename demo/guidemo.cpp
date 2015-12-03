#include <cagui.hpp>
#include <charcoal.hpp>
#include <glcore/opengl.hpp>
#include "../dependencies/stb_image.h" // Implementation included in Charcoal
#include <GLFW/glfw3.h>
#include "stdwindow.hpp"
#include <memory>
#include <iostream>

using namespace ei;
using namespace ca::gui;

static std::shared_ptr<ca::gui::FlatTheme> g_flatTheme;
static MouseState g_mouseState;
static GLFWcursor* g_cursors[10];

static void cursorPosFunc(GLFWwindow*, double _x, double _y)
{
	Coord2 newPos;
	newPos.x = (float)_x;
	newPos.y = GUIManager::getHeight() - (float)_y;
	g_mouseState.deltaPos = newPos - g_mouseState.position;
	g_mouseState.position = newPos;
}

void mouseButtonFunc(GLFWwindow* _window, int _button, int _action, int _mods)
{
	if(_button < 5)
	{
		if(_action == GLFW_PRESS)
			g_mouseState.buttons[_button] = MouseState::DOWN;
		else if(_action == GLFW_RELEASE)
			g_mouseState.buttons[_button] = MouseState::UP;
	}
}


GLFWcursor* loadAsCursor(const char* _imgFile, int _hotX, int _hotY)
{
	GLFWimage img;
	int numComponents = 4;
	img.pixels = stbi_load(_imgFile, &img.width, &img.height, &numComponents, 0);

	GLFWcursor* newCursor = glfwCreateCursor(&img, _hotX, _hotY);

	stbi_image_free(img.pixels);

	return newCursor;
}

void setupInput(GLFWwindow* _window)
{
	glfwSetCursorPosCallback(_window, cursorPosFunc);
	glfwSetMouseButtonCallback(_window, mouseButtonFunc);

	g_cursors[(int)CursorType::ARROW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	g_cursors[(int)CursorType::CROSSHAIR] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
	g_cursors[(int)CursorType::MOVE] = loadAsCursor("textures/cursor_cross.png", 7, 7);
	g_cursors[(int)CursorType::RESIZE_DDOWN] = loadAsCursor("textures/cursor_resize_ddown.png", 7, 7);
	g_cursors[(int)CursorType::RESIZE_DUP] = loadAsCursor("textures/cursor_resize_dup.png", 7, 7);
	g_cursors[(int)CursorType::RESIZE_H] = loadAsCursor("textures/cursor_resize_h.png", 7, 7);
	g_cursors[(int)CursorType::RESIZE_V] = loadAsCursor("textures/cursor_resize_v.png", 7, 7);
}


void createGUI(GLFWwindow* _window)
{
	// Create an instance for all themes
	FlatProperties themeProps;
	themeProps.backgroundColor = Vec4(0.05f, 0.05f, 0.05f, 1.0f);
	themeProps.foregroundColor = Vec4(0.25f, 0.25f, 0.25f, 1.0f);
	themeProps.textColor = Vec4(0.8f, 0.8f, 0.7f, 1.0f);
	themeProps.textBackColor = Vec4(0.01f, 0.01f, 0.01f, 1.0f);
	themeProps.hoverBackgroundColor = Vec4(0.15f, 0.15f, 0.15f, 1.0f);
	themeProps.hoverTextColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	themeProps.textSize = 14.0f;
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
	f0->setExtent(Vec2(5.0f), Vec2(100.0f, 105.0f));
	f0->setBackgroundOpacity(0.5f);
	GUIManager::add(f0);

	for(int i = 0; i < 4; ++i)
	{
		ButtonPtr b0 = std::make_shared<Button>();
		b0->setExtent(f0->getPosition() + Vec2(5.0f, 5.0f + i*25.0f), Vec2(90.0f, 20.0f));
		std::string name = "Test " + std::to_string(i);
		b0->setText(name.c_str());
		b0->addOnButtonChangeFunc([i,name](const Coord2&, int, MouseState::ButtonState){ std::cout << "Button " << name << " clicked.\n"; }, MouseState::CLICKED);
		b0->addOnButtonChangeFunc([i,name](const Coord2&, int, MouseState::ButtonState){ std::cout << "Button " << name << " double clicked.\n"; }, MouseState::DBL_CLICKED);
		b0->setAnchor(SIDE::LEFT, f0->getAnchor(SIDE::LEFT));
		b0->setAnchor(SIDE::BOTTOM, f0->getAnchor(SIDE::BOTTOM));
		f0->add(b0);
	}

	// Second frame with image buttons
	FramePtr f1 = std::make_shared<Frame>(true, false, true, true);
	f1->setExtent(Vec2(5.0f, 120.0f), Vec2(100.0f, 230.0f));
	f1->setBackgroundOpacity(0.5f);
	GUIManager::add(f1);

	ButtonPtr b1 = std::make_shared<Button>();
	b1->setExtent(f1->getPosition() + Vec2(5.0f, 5.0f), Vec2(90.0f, 40.0f));
	b1->setIcon("textures/ca_icon32.png", SIDE::LEFT, coord::pixel(32,32), false);
	b1->setAnchor(SIDE::LEFT, f1->getAnchor(SIDE::LEFT));
	b1->setAnchor(SIDE::BOTTOM, f1->getAnchor(SIDE::BOTTOM));
	f1->add(b1);

	b1 = std::make_shared<Button>();
	b1->setExtent(f1->getPosition() + Vec2(5.0f, 50.0f), Vec2(90.0f, 40.0f));
	b1->setIcon("textures/ca_icon32.png", SIDE::LEFT, coord::pixel(20,20), false, 5.0f);
	b1->setText("Icon Left");
	b1->setAnchor(SIDE::LEFT, f1->getAnchor(SIDE::LEFT));
	b1->setAnchor(SIDE::BOTTOM, f1->getAnchor(SIDE::BOTTOM));
	f1->add(b1);

	b1 = std::make_shared<Button>();
	b1->setExtent(f1->getPosition() + Vec2(5.0f, 95.0f), Vec2(90.0f, 40.0f));
	b1->setIcon("textures/ca_icon32.png", SIDE::RIGHT, coord::pixel(20,20), false, 2.0f);
	b1->setText("Icon Right");
	b1->setAnchor(SIDE::LEFT, f1->getAnchor(SIDE::LEFT));
	b1->setAnchor(SIDE::BOTTOM, f1->getAnchor(SIDE::BOTTOM));
	f1->add(b1);

	b1 = std::make_shared<Button>();
	b1->setExtent(f1->getPosition() + Vec2(5.0f, 140.0f), Vec2(90.0f, 40.0f));
	b1->setIcon("textures/ca_icon32.png", SIDE::BOTTOM, coord::pixel(20,20), false, 2.0f);
	b1->setText("Icon Bottom");
	b1->setAnchor(SIDE::LEFT, f1->getAnchor(SIDE::LEFT));
	b1->setAnchor(SIDE::BOTTOM, f1->getAnchor(SIDE::BOTTOM));
	f1->add(b1);

	b1 = std::make_shared<Button>();
	b1->setExtent(f1->getPosition() + Vec2(5.0f, 185.0f), Vec2(90.0f, 40.0f));
	b1->setIcon("textures/ca_icon32.png", SIDE::TOP, coord::pixel(20,20), false, 2.0f);
	b1->setText("Icon Top");
	b1->setAnchor(SIDE::LEFT, f1->getAnchor(SIDE::LEFT));
	b1->setAnchor(SIDE::BOTTOM, f1->getAnchor(SIDE::BOTTOM));
	f1->add(b1);
}

void runMainLoop(GLFWwindow* _window)
{
	HRClock clock;
	while(!glfwWindowShouldClose(_window))
	{
		float deltaTime = (float)clock.deltaTime();

		// Change the mouse button input states from last frame
		for(int i = 0; i < 5; ++i)
		{
			if(g_mouseState.buttons[i] & MouseState::DOWN)
				g_mouseState.buttons[i] = MouseState::PRESSED;
			if(g_mouseState.buttons[i] & MouseState::UP)
				g_mouseState.buttons[i] = MouseState::RELEASED;
		}
		g_mouseState.deltaPos = Coord2(0.0f);
		glfwPollEvents();
		ca::gui::GUIManager::processInput(g_mouseState);
		glfwSetCursor(_window, g_cursors[(int)GUIManager::getCursorType()]);

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

	setupInput(window);

	createGUI(window);
	runMainLoop(window);

	GUIManager::exit();
	ca::cc::ShaderManager::clear();
	ca::cc::Texture2DManager::clear();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}