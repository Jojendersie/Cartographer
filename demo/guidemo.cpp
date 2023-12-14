#include <ca/gui.hpp>
#include <ca/charcoal.hpp>
#include <ca/paper.hpp>
#include "../dependencies/stb_image.h" // Implementation included in Charcoal
#include <GLFW/glfw3.h>
#include <memory>
#include <iostream>
#include <codecvt>

#include <regex>
#include <algorithm>

#include "stdwindow.hpp"

using namespace ei;
using namespace ca::gui;

static ThemePtr g_flatTheme;
static ThemePtr g_3dTheme;
static MouseState g_mouseState;
static KeyboardState g_keyboardState;
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

static void keyFunc(GLFWwindow*, int _key, int _scancode, int _action, int _mods)
{
	// TODO: make this part of the lib as convert function? Con: glfw is not a dependency.
	// The gui does not know of such things. But, some eventual useless converter functions are not that bad.
	static uint8 GLFW_TO_CAGUI_KEYMAP[349] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		44, 0, 0, 0, 0, 0, 0, 52,
		0, 0, 0, 0, 54, 45, 55, 56,
		30, 31, 32, 33, 34, 35, 36, 37,
		38, 39, 0, 51, 0, 46, 0, 0,
		0, 4, 5, 6, 7, 8, 9, 10,
		11, 12, 13, 14, 15, 16, 17, 18,
		19, 20, 21, 22, 23, 24, 25, 26,
		27, 28, 29, 47, 49, 48, 0, 0,
		53, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 50, 100, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		41, 40, 43, 42, 73, 76, 79, 80,
		81, 82, 75, 78, 74, 77, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		57, 71, 83, 70, 72, 0, 0, 0,
		0, 0, 58, 59, 60, 61, 62, 63,
		64, 65, 66, 67, 68, 69, 104, 105,
		106, 107, 108, 109, 110, 111, 112, 113,
		114, 115, 0, 0, 0, 0, 0, 0, 98,
		89, 90, 91, 92, 93, 94, 95, 96,
		97, 99, 84, 85, 86, 87, 88, 103,
		0, 0, 0, 225, 224, 226, 227, 229,
		228, 230, 231, 101
	};
	if(_action == GLFW_PRESS) {
		g_keyboardState.keys[GLFW_TO_CAGUI_KEYMAP[_key]] = ca::gui::KeyboardState::DOWN;
		g_keyboardState.anyKeyChanged = true;
	} else if(_action == GLFW_RELEASE) {
		g_keyboardState.keys[GLFW_TO_CAGUI_KEYMAP[_key]] = ca::gui::KeyboardState::UP;
		g_keyboardState.anyKeyChanged = true;
	} else if(_action == GLFW_REPEAT) {
		g_keyboardState.keys[GLFW_TO_CAGUI_KEYMAP[_key]] = ca::gui::KeyboardState::DOWN;
		g_keyboardState.anyKeyChanged = true;
	}
}

void charFunc(GLFWwindow*, unsigned int _char)
{
	std::wstring_convert<std::codecvt_utf8<int32_t>,int32_t> convert;
	g_keyboardState.characterInput += convert.to_bytes(_char);
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
	glfwSetKeyCallback(_window, keyFunc);
	glfwSetCharCallback(_window, charFunc);

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
	FramePtr f0(new Frame);
	f0->setMoveable(true);
	f0->setResizeable(SIDE_FLAGS::ALL);
	f0->setExtent(Vec2(5.0f), Vec2(100.0f, 105.0f));
	f0group->add(f0);

	for(int i = 0; i < 4; ++i)
	{
		ButtonPtr b0(new Button);
		b0->setExtent(f0->position() + Vec2(5.0f, 5.0f + i*25.0f), Vec2(90.0f, 20.0f));
		std::string name = "Test " + std::to_string(i);
		b0->setText(name.c_str());
		b0->addOnButtonChangeFunc([i,name](Widget*, const Coord2&, int, MouseState::ButtonState){ std::cout << "Button " << name << " clicked.\n"; }, MouseState::CLICKED);
		b0->addOnButtonChangeFunc([i,name](Widget*, const Coord2&, int, MouseState::ButtonState){ std::cout << "Button " << name << " double clicked.\n"; }, MouseState::DBL_CLICKED);
		b0->setAnchors(f0.get(), f0->left(), b0->bottom() - (i==0?5.0f:2.5f), f0->right(), b0->top() + (i==3?5.0f:2.5f));
		f0->add(b0);
	}
	
	// Add node handles for node-based-editing demo
	NodeHandlePtr h00(new NodeHandle);
	h00->setColor(Vec3(themeProps2.basicHoverColor));
	Coord2 handleCenter(f0->horizontalCenter(), f0->top() + 2.0f);
	h00->setExtent(handleCenter - 6.0f, Coord2(12.0f));
	h00->setAnchors(f0.get(), handleCenter.x, f0->top(), -1.0f, -1.0f);
	h00->setRotation(PI / 2.0f);
	f0group->add(h00, 1);

	// Second frame with image buttons
	FramePtr f1(new Frame);
	f1->setMoveable(true);
	f1->setResizeable(SIDE_FLAGS::ALL);
	f1->setExtent(Vec2(5.0f, 120.0f), Vec2(100.0f, 230.0f));
	GUIManager::add(f1);

	ButtonPtr b1(new Button);
	b1->setExtent(f1->position() + Vec2(5.0f, 5.0f), Vec2(90.0f, 40.0f));
	b1->setIconImg("textures/ca_icon32.png", false);
	b1->setIconExtent(SIDE::LEFT, coord::pixel(32,32));
	b1->setAnchors(f1.get(), f1->left(), f1->bottom(), f1->right(), b1->top()+2.5f);
	f1->add(b1);

	b1 = ButtonPtr(new Button);
	b1->setExtent(f1->position() + Vec2(5.0f, 50.0f), Vec2(90.0f, 40.0f));
	b1->setIconImg("textures/ca_icon32.png", false);
	b1->setIconExtent(SIDE::LEFT, coord::pixel(20,20), 5.0f);
	b1->setText("Icon Left");
	b1->setAnchors(f1.get(), f1->left(), b1->bottom()-2.5f, f1->right(), b1->top()+2.5f);
	f1->add(b1);

	b1 = ButtonPtr(new Button);
	b1->setExtent(f1->position() + Vec2(5.0f, 95.0f), Vec2(90.0f, 40.0f));
	b1->setIconImg("textures/ca_icon32.png", false);
	b1->setIconExtent(SIDE::RIGHT, coord::pixel(20,20), 2.0f);
	b1->setText("Icon Right");
	b1->setAnchors(f1.get(), f1->left(), b1->bottom()-2.5f, f1->right(), b1->top()+2.5f);
	f1->add(b1);

	b1 = ButtonPtr(new Button);
	b1->setExtent(f1->position() + Vec2(5.0f, 140.0f), Vec2(90.0f, 40.0f));
	b1->setIconImg("textures/ca_icon32.png", false);
	b1->setIconExtent(SIDE::BOTTOM, coord::pixel(20,20), 2.0f);
	b1->setText("Icon Bottom");
	b1->setAnchors(f1.get(), f1->left(), b1->bottom()-2.5f, f1->right(), b1->top()+2.5f);
	f1->add(b1);

	b1 = ButtonPtr(new Button);
	b1->setExtent(f1->position() + Vec2(5.0f, 185.0f), Vec2(90.0f, 40.0f));
	b1->setIconImg("textures/ca_icon32.png", false);
	b1->setIconExtent(SIDE::TOP, coord::pixel(20,20), 2.0f);
	b1->setText("Icon Top");
	b1->setAnchors(f1.get(), f1->left(), b1->bottom()-2.5f, f1->right(), b1->top()+2.5f);
	f1->add(b1);

	// Third frame with checkboxes and sliders
	FramePtr f2(new Frame);
	f2->setMoveable(true);
	f2->setResizeable(SIDE_FLAGS::ALL);
	f2->setExtent(Vec2(5.0f, 360.0f), Vec2(100.0f, 255.0f));
	GUIManager::add(f2);

	for(int i = 0; i < 5; ++i)
	{
		CheckBoxPtr c0(new CheckBox);
		c0->setExtent(f2->position() + Coord2(5.0f, 5.0f + i * 25.0f), Coord2(90.0f, 20.0f));
		if(i < 4) c0->setText("Cookie?");
		c0->setAnchors(f2.get(), f2->left(), c0->bottom()-2.5f, f2->right(), c0->top()+2.5f);
		f2->add(c0);
	}

	for(int i = 0; i < 5; ++i)
	{
		SliderPtr s0(new Slider);
		s0->setExtent(f2->position() + Coord2(5.0f, 130.0f + i * 25.0f), Coord2(90.0f, 20.0f));
		s0->setAnchors(f2.get(), f2->left(), s0->bottom()-2.5f, f2->right(), s0->top()+2.5f);
		s0->setValueTextPosition(0.5f);
		f2->add(s0);
	}

	// Fourth frame with labels (window fake)
	GroupPtr f3group(new Group); // Contains f3 and the node based handles
	GUIManager::add(f3group);
	FramePtr f3(new Frame);
	f3->setMoveable(true);
	f3->setResizeable(SIDE_FLAGS::ALL);
	f3->setExtent(Vec2(115.0f, 120.0f), Vec2(200.0f, 230.0f));
	f3group->add(f3);

	// Add node handles
	NodeHandlePtr h30(new NodeHandle);
	h30->setColor(Vec3(themeProps2.basicHoverColor));
	handleCenter = Coord2(f3->horizontalCenter(), f3->bottom() - 2.0f);
	h30->setExtent(handleCenter - 6.0f, Coord2(12.0f));
	h30->setAnchors(f3.get(), handleCenter.x, f3->bottom(), -1.0f, -1.0f);
	h30->setRotation(-PI / 2.0f);
	f3group->add(h30, 1);
	NodeHandlePtr h31(new NodeHandle);
	h31->setColor(Vec3(0.0f, 0.5f, 0.0f));
	handleCenter = Coord2(f3->left() - 2.0f, f3->verticalCenter());
	h31->setExtent(handleCenter - 6.0f, Coord2(12.0f));
	h31->setAnchors(f3.get(), f3->left(), handleCenter.y, -1.0f, -1.0f);
	h31->setRotation(-PI);
	f3group->add(h31, 1);

	LabelPtr l0(new Label);
	l0->setExtent(f3->position() + Coord2(0.0f, f3->height() - 20.0f) + 2.0f, Coord2(f3->width(), 16.0f));
	l0->setText(" Usually useless titelbar");
	l0->setAnchors(f3.get(), SIDE_FLAGS::HORIZONTAL | SIDE_FLAGS::TOP);
	f3->add(l0);

	ButtonPtr b0(new Button);
	b0->setExtent(f3->position() + f3->size() - 20.0f, Coord2(16.0f));
	b0->setText(" X");//TODO: why is centering wrong here?
	b0->setAnchors(f3.get(), SIDE_FLAGS::TOPRIGHT);
	f3->add(b0);

	l0 = LabelPtr(new Label);
	l0->setExtent(f3->position() + 2.0f, f3->size() - Coord2(4.0f, 24.0f));
	l0->setText("LEFT");
	l0->setAnchors(f3.get(), f3->left(), l0->verticalCenter(), -1.0f, -1.0f);
	f3->add(l0);

	l0 = LabelPtr(new Label);
	l0->setExtent(f3->position() + 2.0f, f3->size() - Coord2(4.0f, 24.0f));
	l0->setText("RIGHT");
	l0->setAlignment(SIDE::RIGHT);
	l0->setAnchors(f3.get(), -1.0f, l0->verticalCenter() , f3->right(), -1.0f);
	f3->add(l0);

	l0 = LabelPtr(new Label);
	l0->setExtent(f3->position() + 2.0f, f3->size() - Coord2(4.0f, 24.0f));
	l0->setText("BOTTOM");
	l0->setAlignment(SIDE::BOTTOM);
	l0->setAnchors(f3.get(), l0->horizontalCenter(), f3->bottom(), -1.0f, -1.0f);
	f3->add(l0);

	l0 = LabelPtr(new Label);
	l0->setExtent(f3->position() + 2.0f, f3->size() - Coord2(4.0f, 24.0f));
	l0->setText("TOP");
	l0->setAlignment(SIDE::TOP);
	l0->setAnchors(f3.get(), l0->horizontalCenter(), -1.0f, -1.0f, f3->top());
	f3->add(l0);

	l0 = LabelPtr(new Label);
	l0->setExtent(f3->position() + 2.0f, f3->size() - Coord2(4.0f, 24.0f));
	l0->setText("Reopen the window\nby clicking one of\nthe icon-buttons.");
	l0->setAlignment(SIDE::CENTER);
	l0->setAnchors(f3.get(), l0->horizontalCenter(), l0->verticalCenter(), -1.0f, -1.0f);
	f3->add(l0);

	// Add some connectors between node handles
	NodeConnectorPtr connector0(new NodeConnector);
	connector0->setSource(h00);
	connector0->setDest(h30);
	GUIManager::add(connector0, 1);

	// Create a set of images with widget-connectors
	FramePtr f4(new Frame);
	f4->setMoveable(true);
	f4->setExtent(Vec2(115.0f, 360.0f), Vec2(200.0f, 255.0f));
	GUIManager::add(f4);
	ImagePtr i0(new Image);
	i0->setExtent(f4->position() + Vec2(20.0f, 10.0f), Vec2(32.0f));
	i0->setTexture("textures/ca_icon32.png");
	i0->setAnchors(f4.get(), i0->horizontalCenter(), i0->verticalCenter(), -1.0f, -1.0f);
	i0->setRegion(std::make_unique<EllipseRegion>(i0.get()));
	f4->add(i0);
	ImagePtr i1(new Image);
	i1->setExtent(f4->position() + Vec2(10.0f, 70.0f), Vec2(32.0f));
	i1->setTexture("textures/ca_icon32.png");
	i1->setAnchors(f4.get(), i0->horizontalCenter(), i0->verticalCenter(), -1.0f, -1.0f);
	i1->setRegion(std::make_unique<EllipseRegion>(i1.get()));
	f4->add(i1);
	ImagePtr i2(new Image);
	i2->setExtent(f4->position() + Vec2(100.0f, 100.0f), Vec2(32.0f));
	i2->setTexture("textures/ca_icon32.png");
	i2->setAnchors(f4.get(), i0->horizontalCenter(), i0->verticalCenter(), -1.0f, -1.0f);
	i2->setRegion(std::make_unique<EllipseRegion>(i2.get()));
	f4->add(i2);
	ImagePtr i3(new Image);
	i3->setExtent(f4->position() + Vec2(20.0f, 140.0f), Vec2(32.0f));
	i3->setTexture("textures/ca_icon32.png");
	i3->setAnchors(f4.get(), i0->horizontalCenter(), i0->verticalCenter(), -1.0f, -1.0f);
	i3->setRegion(std::make_unique<EllipseRegion>(i3.get()));
	f4->add(i3);
	ImagePtr i4(new Image);
	i4->setExtent(f4->position() + Vec2(140.0f, 190.0f), Vec2(32.0f));
	i4->setTexture("textures/ca_icon32.png");
	i4->setAnchors(f4.get(), i0->horizontalCenter(), i0->verticalCenter(), -1.0f, -1.0f);
	i4->setRegion(std::make_unique<EllipseRegion>(i4.get()));
	f4->add(i4);
	WidgetConnectorPtr con0(new WidgetConnector);
	con0->setSource(i0, 2.0f);
	con0->setDest(i1, -2.0f);
	f4->add(con0);
	WidgetConnectorPtr con1(new WidgetConnector);
	con1->setSource(i1, 2.0f);
	con1->setDest(i3, -1.0f);
	f4->add(con1);
	WidgetConnectorPtr con2(new WidgetConnector);
	con2->setSource(i1, 0.0f);
	con2->setDest(i2, -3.0f);
	f4->add(con2);
	WidgetConnectorPtr con3(new WidgetConnector);
	con3->setSource(i0, 0.0f);
	con3->setDest(i4, -1.0f);
	f4->add(con3);

	// A frame which triggers a popup
	FramePtr f5(new Frame);
	f5->setMoveable(true);
	f5->setResizeable(SIDE_FLAGS::ALL);
	f5->setExtent(f0->position() + Coord2(f0->width() + 10.0f, 0.0f), Coord2(f4->width(), f0->height()));
	GUIManager::add(f5);

	l0 = LabelPtr(new Label);
	l0->setExtent(f5->position() + 2.0f, f5->size() - Coord2(4.0f, 4.0f));
	l0->setText("This area triggers a popup.");
	l0->setAlignment(SIDE::CENTER);
	l0->setAnchors(f5.get());
	f5->add(l0);

	LabelPtr l1(new Label);
	l1->setText("Info popup.", 2.0f);
	l1->setBackgroundOpacity(1.0f);
	f5->setInfoPopup(l1);
	l0->setInfoPopup(l1);

	// A frame with edits
	FramePtr f6(new Frame);
	f6->setMoveable(true);
	f6->setResizeable(SIDE_FLAGS::ALL);
	f6->setExtent(f5->position() + Coord2(f5->width() + 10.0f, 0.0f), f5->size());
	GUIManager::add(f6);

	EditPtr e0( new Edit );
	e0->setExtent(f6->position() + Coord2(5.0f), Coord2(f6->width() - 10.0f, 20.0f));
	e0->setText("Type text here");
	e0->setAnchors(f6.get(), f6->left(), e0->verticalCenter(), f6->right(), -1.0f);
	f6->add(e0);

	EditPtr e1( new Edit );
	e1->setExtent(f6->position() + Coord2(5.0f, 30.0f), Coord2(f6->width() - 10.0f, 20.0f));
	e1->setDescriptorText("Type text here");
	e1->setAnchors(f6.get(), f6->left(), e1->verticalCenter(), f6->right(), -1.0f);
	e1->setMargin(2.0f);
	f6->add(e1);

	EditPtr e2( new Edit );
	e2->setExtent(f6->position() + Coord2(5.0f, 55.0f), Coord2(f6->width() - 10.0f, 20.0f));
	e2->setAlignment(SIDE::CENTER);
	e2->setDescriptorText("centered text");
	e2->setAnchors(f6.get(), f6->left(), e2->verticalCenter(), f6->right(), -1.0f);
	e2->setMargin(2.0f);
	f6->add(e2);

	EditPtr e3( new Edit );
	e3->setExtent(f6->position() + Coord2(5.0f, 80.0f), Coord2(f6->width() - 10.0f, 20.0f));
	e3->setDescriptorText("decimal number");
	e3->setAnchors(f6.get(), f6->left(), e3->verticalCenter(), f6->right(), -1.0f);
	e3->setMargin(2.0f);
	e3->setOnTextChangeFunc([](Widget* _this, std::string &_text){
		bool hadPoint = false;
		_text.erase(
			std::remove_if(_text.begin(), _text.end(), [&hadPoint](const char c){
				if(c >= '0' && c <= '9') return false;
				if(hadPoint == true) return true;
				if(c == '.') { hadPoint = true; return false; }
				return true;
			}), _text.end());
		//std::regex r(R"([-+]?([0-9]*[.])?[0-9]+([eE][-+]?[0-9]+)?)");
		//std::sregex_iterator i = std::sregex_iterator(_text.begin(), _text.end(), r);
		//if(i == std::sregex_iterator())
		//	_text = "0";
		//else
		//	_text = i->str();
	});
	f6->add(e3);

	// *********************************************
	// A frame with scrollbars
	FramePtr f7(new Frame);
	f7->setMoveable(true);
	f7->setResizeable(SIDE_FLAGS::ALL);
	f7->setExtent(f6->position() + Coord2(0.0f, f6->height() + 10.0f), f6->size());
	GUIManager::add(f7);

	l0 = LabelPtr(new Label);
	l0->setText("A long text with multiple lines that is here to stay.\nCompletely boring stuff.\n\nIt talks about cookies.\nBut only seldomly.\nWhere is my personal cookie?\nChocolate!\n\nFor the spoon!", 2.0f);
	l0->setPosition(f7->position() + coord::pixel(2,14));

	ScrollBarPtr sbh(new ScrollBar);
	sbh->setExtent(f7->position(), Coord2(f7->width()-12.0f, 12.0f));
	sbh->setHorizontalMode(true);
	sbh->setViewArea(f7, 12.0f + 2.0f);
	sbh->setContent(l0);
	sbh->setAnchors(f7.get(), SIDE_FLAGS::HORIZONTAL | SIDE_FLAGS::BOTTOM);
	f7->add(sbh, 1u);

	ScrollBarPtr sbv(new ScrollBar);
	sbv->setExtent(f7->position() + Coord2(f7->width() - 12.0f, 12.0f), Coord2(12.0f, f7->height() - 12.0f));
	sbv->setViewArea(f7, 12.0f + 2.0f);
	sbv->setContent(l0);
	sbv->setAnchors(f7.get(), SIDE_FLAGS::VERTICAL | SIDE_FLAGS::RIGHT);
	f7->add(sbv, 1u);

	l0->setAnchors(sbh->getAnchor(), l0->left(), -1.0f, -1.0f, -1.0f);
	l0->setAnchors(sbv->getAnchor(), -1.0f, l0->bottom(), -1.0f, -1.0f);
	sbv->setScrollOffset(10000.0f); // Just skip to the top

	// A frame for clipping
	FramePtr f7s(new Frame);
	f7s->setExtent(f7->position() + Coord2(2.0f, 14.0f), f7->size() - 16.0f);
	f7s->setAnchors(f7.get());
	f7s->add(l0);
	f7->add(f7s, 0u);

	// *********************************************
	// Frame with drop down menus
	FramePtr f8(new Frame);
	f8->setMoveable(true);
	f8->setResizeable(SIDE_FLAGS::ALL);
	f8->setExtent(f7->position() + Coord2(0.0f, f7->height() + 10.0f), f7->size());
	GUIManager::add(f8);

	DropDownMenuPtr dm0(new DropDownMenu);
	dm0->setExtent(f8->position() + coord::pixel(2, 24), Coord2(f8->width() - 4, 20));
	dm0->setAnchors(f8.get(), dm0->horizontalCenter(), dm0->verticalCenter(), -1.0f, -1.0f);
	f8->add(dm0);
	dm0->addItem("Alpha");
	dm0->addItem("Beta");
	dm0->addItem("Gamma");
}

void runMainLoop(GLFWwindow* _window)
{
	ca::pa::HRClock clock;
	while(!glfwWindowShouldClose(_window))
	{
		float deltaTime = (float)clock.deltaTime();

		g_mouseState.clear();
		g_keyboardState.clear();
		glfwPollEvents();
		double x, y;
		glfwGetCursorPos(_window, &x, &y);
		//g_mouseState.position = Coord2((float)x, GUIManager::getHeight() - (float)y);
		ca::gui::GUIManager::processInput(g_mouseState);
		ca::gui::GUIManager::processInput(g_keyboardState);
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