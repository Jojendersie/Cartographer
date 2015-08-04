#include <mir.hpp>
#include <thread>
#include <ctime>
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <ei/vector.hpp>
#include "stdwindow.hpp"

using namespace MiR;
using namespace ei;

static SpriteRenderer* s_renderer;
static Program s_shader;
static Sampler* s_pointSampler;
static Sampler* s_smoothSampler;
static Texture2D::Handle s_texture;
static Vec2 s_cursor;

void prepareSprites()
{
	s_pointSampler = new Sampler(Sampler::Filter::POINT, Sampler::Filter::POINT, Sampler::Filter::POINT, Sampler::Border::CLAMP);
	s_smoothSampler = new Sampler(Sampler::Filter::LINEAR, Sampler::Filter::LINEAR, Sampler::Filter::LINEAR, Sampler::Border::CLAMP);
	s_texture = Texture2DManager::get("textures/cursor.png", *s_pointSampler);
	s_renderer = new SpriteRenderer;

	s_renderer->defSprite(0.0, 1.0f, s_texture);
}

void prepareShader(GLFWwindow* _window)
{
	// Load shader from file
	s_shader.attach( MiR::ShaderManager::get("shader/sprite.vert", ShaderType::VERTEX) );
	s_shader.attach( MiR::ShaderManager::get("shader/sprite.geom", ShaderType::GEOMETRY) );
	s_shader.attach( MiR::ShaderManager::get("shader/sprite.frag", ShaderType::FRAGMENT) );
	s_shader.link();

	s_shader.use();
	int w, h;
	glfwGetFramebufferSize(_window, &w, &h);
	Mat4x4 viewProj = ei::orthographicGL(0.0f, (float)w, 0.0f, (float)h, 0.0f, 1.0f);//ei::perspectiveGL(PI/4, getCurrentAspectRatio(), 1.1f, 1000.0f)
	//	* ei::camera(rotationY(time / 10.0f) * Vec3(0.0f, 100.0f, -220.0f), Vec3(0.0f, -50.0f, 0.0f));
	glCall(glUniformMatrix4fv, 0, 1, GL_FALSE, (GLfloat*)&viewProj);
}

void update(GLFWwindow* _window)
{
	s_renderer->clearInstances();

	int w, h;
	glfwGetFramebufferSize(_window, &w, &h);
	s_renderer->newInstance(0, Vec3(s_cursor.x, h-s_cursor.y, 0.0f), 0.0f, Vec2(1.0f));
}

void runMainLoop(GLFWwindow* _window)
{
	//uint64 time0 = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	Device::setCullMode(CullMode::NONE);
	Device::setZFunc(ComparisonFunc::ALWAYS);
	while(!glfwWindowShouldClose(_window))
	{
		glfwPollEvents();
		update(_window);
		//float time = float( std::chrono::high_resolution_clock::now().time_since_epoch().count() - time0 ) / 5000000.0f;
		float time = clock() / 1000.0f;
		glCall(glClearColor, 1.0f, 1.0f, 1.0f, 1.0f);
		glCall(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		s_shader.use();
		s_renderer->draw();
		
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		glfwSwapBuffers(_window);
	}
}

static void cursorCallback(GLFWwindow* _window, double _x, double _y)
{
	s_cursor.x = (float)_x;
	s_cursor.y = (float)_y;
}


int main()
{
	GLFWwindow* window = setupStdWindow("MiR sprite demo.", false);
	if(!window) return 1;
	glfwSetCursorPosCallback(window, cursorCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	prepareSprites();
	prepareShader(window);
	runMainLoop(window);

	delete s_pointSampler;
	delete s_smoothSampler;
	delete s_renderer;
	glfwDestroyWindow(window);
	return 0;
}