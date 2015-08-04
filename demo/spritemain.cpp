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
static Sampler* s_sampler;
static Texture2D::Handle s_texture;

void prepareSprites()
{
	s_renderer = new SpriteRenderer;

	s_sampler = new Sampler(Sampler::Filter::LINEAR, Sampler::Filter::LINEAR, Sampler::Filter::LINEAR);
	s_texture = Texture2DManager::get("textures/rope2.png", *s_sampler);
}

void prepareShader()
{
	// Load shader from file
	s_shader.attach( MiR::ShaderManager::get("shader/instanced3d.vert", ShaderType::VERTEX) );
	s_shader.attach( MiR::ShaderManager::get("shader/simple.frag", ShaderType::FRAGMENT) );
	s_shader.link();
}

void runMainLoop(GLFWwindow* _window)
{
	//uint64 time0 = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	Device::setCullMode(CullMode::NONE);
	Device::setZFunc(ComparisonFunc::LESS);
	while(!glfwWindowShouldClose(_window))
	{
		//float time = float( std::chrono::high_resolution_clock::now().time_since_epoch().count() - time0 ) / 5000000.0f;
		float time = clock() / 1000.0f;
		glCall(glClearColor, 1.0f, 1.0f, 1.0f, 1.0f);
		glCall(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		s_shader.use();
		//Mat4x4 viewProj = ei::perspectiveGL(PI/4, getCurrentAspectRatio(), 1.1f, 1000.0f)
		//	* ei::camera(rotationY(time / 10.0f) * Vec3(0.0f, 100.0f, -220.0f), Vec3(0.0f, -50.0f, 0.0f));
		//glCall(glUniformMatrix4fv, 0, 1, GL_FALSE, (GLfloat*)&viewProj);

		s_renderer->draw();
		
		glfwPollEvents();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		glfwSwapBuffers(_window);
	}
}

int main()
{
	GLFWwindow* window = setupStdWindow("MiR sprite demo.");
	if(!window) return 1;

	prepareSprites();
	prepareShader();
	runMainLoop(window);

	delete s_sampler;
	delete s_renderer;
	glfwDestroyWindow(window);
	return 0;
}