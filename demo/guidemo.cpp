#include <ca/gui.hpp>
#include <ca/charcoal.hpp>
#include <ca/paper.hpp>
#include "../dependencies/stb_image.h" // Implementation included in Charcoal
#include <GLFW/glfw3.h>
#include "stdwindow.hpp"
#include <memory>
#include <iostream>

using namespace ei;
using namespace ca::gui;

static ThemePtr g_flatTheme;
static ThemePtr g_3dTheme;
static MouseState g_mouseState;
static GLFWcursor* g_cursors[10];

static void cursorPosFunc(GLFWwindow*, double _x, double _y)
{
	g_mouseState.position = Coord2((float)_x, GUIManager::getHeight() - (float)_y);
}

static void mouseButtonFunc(GLFWwindow* _window, int _button, int _action, int _mods)
{
	if(_button < 8)
	{
		if(_action == GLFW_PRESS) {
			g_mouseState.buttons[_button] = MouseState::DOWN;
			g_mouseState.anyButtonDown = true;
		} else if(_action == GLFW_RELEASE) {
			g_mouseState.buttons[_button] = MouseState::UP;
			g_mouseState.anyButtonUp = true;
		}
	}
}

static void scrollFunc(GLFWwindow*, double _x, double _y)
{
	g_mouseState.deltaScroll.x = (float)_x;
	g_mouseState.deltaScroll.y = (float)_y;
}


GLFWcursor* loadAsCursor(const char* _imgFile, int _hotX, int _hotY)
{
	GLFWimage img;
	int numComponents = 4;
	img.pixels = stbi_load(_imgFile, &img.width, &img.height, &numComponents, 0);

	GLFWcursor* newCursor = glfwCreateCursor(&img, _hotX, _hotY);

	stbi_image_free(img.pixels);

	if(newCursor) ca::pa::logPedantic("Loaded cursor '", _imgFile, "'.");
	else ca::pa::logWarning("Could not load cursor '", _imgFile, "'.");
	return newCursor;
}

void setupInput(GLFWwindow* _window)
{
	glfwSetCursorPosCallback(_window, cursorPosFunc);
	glfwSetMouseButtonCallback(_window, mouseButtonFunc);
	glfwSetScrollCallback(_window, scrollFunc);

	g_cursors[(int)CursorType::ARROW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	if(g_cursors[(int)CursorType::ARROW]) ca::pa::logPedantic("Loaded cursor 'GLFW_ARROW_CURSOR'.");
	g_cursors[(int)CursorType::CROSSHAIR] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
	if(g_cursors[(int)CursorType::CROSSHAIR]) ca::pa::logPedantic("Loaded cursor 'GLFW_CROSSHAIR_CURSOR'.");
	g_cursors[(int)CursorType::MOVE] = loadAsCursor("textures/cursor_cross.png", 7, 7);
	g_cursors[(int)CursorType::RESIZE_DDOWN] = loadAsCursor("textures/cursor_resize_ddown.png", 7, 7);
	g_cursors[(int)CursorType::RESIZE_DUP] = loadAsCursor("textures/cursor_resize_dup.png", 7, 7);
	g_cursors[(int)CursorType::RESIZE_H] = loadAsCursor("textures/cursor_resize_h.png", 7, 7);
	g_cursors[(int)CursorType::RESIZE_V] = loadAsCursor("textures/cursor_resize_v.png", 7, 7);
	ca::pa::logPedantic("Loaded cursors.");
}


void createGUI(GLFWwindow* _window)
{
	// Create an instance for all themes
	FlatProperties themeProps;
	themeProps.backgroundColor = Vec4(0.025f, 0.025f, 0.025f, 1.0f);
	themeProps.buttonColor = Vec4(0.05f, 0.05f, 0.05f, 1.0f);
	themeProps.foregroundColor = Vec4(0.25f, 0.25f, 0.25f, 1.0f);
	themeProps.textColor = Vec4(0.8f, 0.8f, 0.7f, 1.0f);
	themeProps.textBackColor = Vec4(0.01f, 0.01f, 0.01f, 1.0f);
	themeProps.hoverButtonColor = Vec4(0.15f, 0.15f, 0.15f, 1.0f);
	themeProps.hoverTextColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	themeProps.textSize = 14.0f;
	g_flatTheme = std::make_shared<FlatTheme>(themeProps);

	Sharp3DProperties themeProps2;
	themeProps2.borderWidth = 1;
	themeProps2.basicColor = Vec4(0.05f, 0.05f, 0.05f, 1.0f);
	themeProps2.basicHoverColor = Vec4(0.15f, 0.15f, 0.15f, 1.0f);
	themeProps2.textColor = Vec4(0.8f, 0.8f, 0.7f, 1.0f);
	themeProps2.textBackColor = Vec4(0.01f, 0.01f, 0.01f, 1.0f);
	themeProps2.hoverTextColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	themeProps2.textSize = 14.0f;
	g_3dTheme = std::make_shared<Sharp3DTheme>(themeProps2);

	// Initialize GUI system itself
	int w, h;
	glfwGetFramebufferSize(_window, &w, &h);
	GUIManager::init(
		std::make_shared<CharcoalBackend>("calibri.caf"),
		g_3dTheme,
		w, h );

	// Create GUI-elements
	// A resizeable frame with different anchored buttons
	GroupPtr f0group(new Group); // Contains f0 and the node based handles
	GUIManager::add(f0group);
	FramePtr f0(new Frame(true, false, true, true));
	f0->setExtent(Vec2(5.0f), Vec2(100.0f, 105.0f));
	f0->setAnchorProvider( std::make_unique<GridAnchorProvider>(3, 5) );
	f0group->add(f0);

	for(int i = 0; i < 4; ++i)
	{
		ButtonPtr b0(new Button);
		b0->setExtent(f0->getPosition() + Vec2(5.0f, 5.0f + i*25.0f), Vec2(90.0f, 20.0f));
		std::string name = "Test " + std::to_string(i);
		b0->setText(name.c_str());
		b0->addOnButtonChangeFunc([i,name](const Coord2&, int, MouseState::ButtonState){ std::cout << "Button " << name << " clicked.\n"; }, MouseState::CLICKED);
		b0->addOnButtonChangeFunc([i,name](const Coord2&, int, MouseState::ButtonState){ std::cout << "Button " << name << " double clicked.\n"; }, MouseState::DBL_CLICKED);
		b0->autoAnchor(f0->getAnchorProvider());
		b0->setAnchorModes(Anchorable::PREFER_MOVE, Anchorable::PREFER_RESIZE);
		f0->add(b0);
	}
	
	// Add node handles for node-based-editing demo
	NodeHandlePtr h00(new NodeHandle);
	h00->setColor(Vec3(themeProps2.basicHoverColor));
	Coord2 handleCenter(f0->getRefFrame().horizontalCenter(), f0->getRefFrame().top() + 2.0f);
	h00->setExtent(handleCenter - 6.0f, Coord2(12.0f));
	h00->autoAnchor(f0->getAnchorProvider());
	h00->setRotation(PI / 2.0f);
	f0group->add(h00, 1);

	// Second frame with image buttons
	FramePtr f1(new Frame(true, false, true, true));
	f1->setExtent(Vec2(5.0f, 120.0f), Vec2(100.0f, 230.0f));
	f1->setAnchorProvider( std::make_unique<BorderAnchorProvider>() );
	BorderAnchorProvider* anchorsf1 = static_cast<BorderAnchorProvider*>(f1->getAnchorProvider());
	GUIManager::add(f1);

	ButtonPtr b1(new Button);
	b1->setExtent(f1->getPosition() + Vec2(5.0f, 5.0f), Vec2(90.0f, 40.0f));
	b1->setIcon("textures/ca_icon32.png", SIDE::LEFT, coord::pixel(32,32), false);
	b1->autoAnchor(anchorsf1);
	f1->add(b1);

	b1 = ButtonPtr(new Button);
	b1->setExtent(f1->getPosition() + Vec2(5.0f, 50.0f), Vec2(90.0f, 40.0f));
	b1->setIcon("textures/ca_icon32.png", SIDE::LEFT, coord::pixel(20,20), false, 5.0f);
	b1->setText("Icon Left");
	b1->autoAnchor(anchorsf1);
	f1->add(b1);

	b1 = ButtonPtr(new Button);
	b1->setExtent(f1->getPosition() + Vec2(5.0f, 95.0f), Vec2(90.0f, 40.0f));
	b1->setIcon("textures/ca_icon32.png", SIDE::RIGHT, coord::pixel(20,20), false, 2.0f);
	b1->setText("Icon Right");
	b1->autoAnchor(anchorsf1);
	f1->add(b1);

	b1 = ButtonPtr(new Button);
	b1->setExtent(f1->getPosition() + Vec2(5.0f, 140.0f), Vec2(90.0f, 40.0f));
	b1->setIcon("textures/ca_icon32.png", SIDE::BOTTOM, coord::pixel(20,20), false, 2.0f);
	b1->setText("Icon Bottom");
	b1->autoAnchor(anchorsf1);
	f1->add(b1);

	b1 = ButtonPtr(new Button);
	b1->setExtent(f1->getPosition() + Vec2(5.0f, 185.0f), Vec2(90.0f, 40.0f));
	b1->setIcon("textures/ca_icon32.png", SIDE::TOP, coord::pixel(20,20), false, 2.0f);
	b1->setText("Icon Top");
	b1->autoAnchor(anchorsf1);
	f1->add(b1);

	// Third frame with checkboxes and sliders
	FramePtr f2(new Frame(true, false, true, true));
	f2->setExtent(Vec2(5.0f, 360.0f), Vec2(100.0f, 255.0f));
	f2->setAnchorProvider( std::make_unique<BorderAnchorProvider>() );
	BorderAnchorProvider* anchorsf2 = static_cast<BorderAnchorProvider*>(f2->getAnchorProvider());
	GUIManager::add(f2);

	for(int i = 0; i < 5; ++i)
	{
		CheckBoxPtr c0(new CheckBox);
		c0->setExtent(f2->getPosition() + Coord2(5.0f, 5.0f + i * 25.0f), Coord2(90.0f, 20.0f));
		if(i < 4) c0->setText("Cookie?");
		c0->autoAnchor(anchorsf2);
		c0->setAnchorModes(Anchorable::PREFER_MOVE);
		f2->add(c0);
	}

	for(int i = 0; i < 5; ++i)
	{
		SliderPtr s0(new Slider);
		s0->setExtent(f2->getPosition() + Coord2(5.0f, 130.0f + i * 25.0f), Coord2(90.0f, 20.0f));
		s0->autoAnchor(anchorsf2);
		s0->setAnchorModes(Anchorable::PREFER_MOVE);
		s0->setValueTextPosition(0.5f);
		f2->add(s0);
	}

	// Fourth frame with labels (window fake)
	GroupPtr f3group(new Group); // Contains f3 and the node based handles
	GUIManager::add(f3group);
	FramePtr f3(new Frame(true, false, true, true));
	f3->setExtent(Vec2(115.0f, 120.0f), Vec2(200.0f, 230.0f));
	f3->setAnchorProvider( std::make_unique<BorderAnchorProvider>() );
	BorderAnchorProvider* anchorsf3 = static_cast<BorderAnchorProvider*>(f3->getAnchorProvider());
	f3group->add(f3);

	// Add node handles
	NodeHandlePtr h30(new NodeHandle);
	h30->setColor(Vec3(themeProps2.basicHoverColor));
	handleCenter = Coord2(f3->getRefFrame().horizontalCenter(), f3->getRefFrame().bottom() - 2.0f);
	h30->setExtent(handleCenter - 6.0f, Coord2(12.0f));
	h30->autoAnchor(f3->getAnchorProvider());
	h30->setRotation(-PI / 2.0f);
	f3group->add(h30, 1);
	NodeHandlePtr h31(new NodeHandle);
	h31->setColor(Vec3(0.0f, 0.5f, 0.0f));
	handleCenter = Coord2(f3->getRefFrame().left() - 2.0f, f3->getRefFrame().verticalCenter());
	h31->setExtent(handleCenter - 6.0f, Coord2(12.0f));
	h31->autoAnchor(f3->getAnchorProvider());
	h31->setRotation(-PI);
	f3group->add(h31, 1);

	LabelPtr l0(new Label);
	l0->setExtent(f3->getPosition() + Coord2(0.0f, f3->getSize().y - 20.0f) + 2.0f, Coord2(f3->getSize().x, 16.0f));
	l0->setText(" Usually useless titelbar");
	l0->setAnchoring(SIDE::LEFT, anchorsf3->getAnchor(SIDE::LEFT));
	l0->setAnchoring(SIDE::RIGHT, anchorsf3->getAnchor(SIDE::RIGHT));
	l0->setAnchoring(SIDE::TOP, anchorsf3->getAnchor(SIDE::TOP));
	l0->setHorizontalAnchorMode(Anchorable::PREFER_MOVE);
	f3->add(l0);

	ButtonPtr b0(new Button);
	b0->setExtent(f3->getPosition() + f3->getSize() - 20.0f, Coord2(16.0f));
	b0->setText(" X");//TODO: why is centering wrong here?
	b0->setAnchoring(SIDE::RIGHT, anchorsf3->getAnchor(SIDE::RIGHT));
	b0->setAnchoring(SIDE::TOP, anchorsf3->getAnchor(SIDE::TOP));
	f3->add(b0);

	l0 = LabelPtr(new Label);
	l0->setExtent(f3->getPosition() + 2.0f, f3->getSize() - Coord2(4.0f, 24.0f));
	l0->setText("LEFT");
	l0->autoAnchor(anchorsf3);
	l0->setAnchorModes(Anchorable::PREFER_RESIZE);
	f3->add(l0);

	l0 = LabelPtr(new Label);
	l0->setExtent(f3->getPosition() + 2.0f, f3->getSize() - Coord2(4.0f, 24.0f));
	l0->setText("RIGHT");
	l0->setAlignment(SIDE::RIGHT);
	l0->autoAnchor(anchorsf3);
	l0->setAnchorModes(Anchorable::PREFER_RESIZE);
	f3->add(l0);

	l0 = LabelPtr(new Label);
	l0->setExtent(f3->getPosition() + 2.0f, f3->getSize() - Coord2(4.0f, 24.0f));
	l0->setText("BOTTOM");
	l0->setAlignment(SIDE::BOTTOM);
	l0->autoAnchor(anchorsf3);
	l0->setAnchorModes(Anchorable::PREFER_RESIZE);
	f3->add(l0);

	l0 = LabelPtr(new Label);
	l0->setExtent(f3->getPosition() + 2.0f, f3->getSize() - Coord2(4.0f, 24.0f));
	l0->setText("TOP");
	l0->setAlignment(SIDE::TOP);
	l0->autoAnchor(anchorsf3);
	l0->setAnchorModes(Anchorable::PREFER_RESIZE);
	f3->add(l0);

	l0 = LabelPtr(new Label);
	l0->setExtent(f3->getPosition() + 2.0f, f3->getSize() - Coord2(4.0f, 24.0f));
	l0->setText("Reopen the window\nby clicking one of\nthe icon-buttons.");
	l0->setAlignment(SIDE::CENTER);
	l0->autoAnchor(anchorsf3);
	l0->setAnchorModes(Anchorable::PREFER_RESIZE);
	f3->add(l0);

	// Add some connectors between node handles
	NodeConnectorPtr connector0(new NodeConnector);
	connector0->setSource(h00);
	connector0->setDest(h30);
	GUIManager::add(connector0, 1);
}

void runMainLoop(GLFWwindow* _window)
{
	ca::pa::HRClock clock;
	while(!glfwWindowShouldClose(_window))
	{
		float deltaTime = (float)clock.deltaTime();

		g_mouseState.clear();
		glfwPollEvents();
		double x, y;
		glfwGetCursorPos(_window, &x, &y);
		//g_mouseState.position = Coord2((float)x, GUIManager::getHeight() - (float)y);
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
	GLFWwindow* window = setupStdWindow("Carthographer GUI demo.", true);
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