#include <mir.hpp>
#include <thread>
#include <ctime>
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <ei/vector.hpp>
#include "stdwindow.hpp"

using namespace MiR;
using namespace ei;

struct Smile
{
	Vec3 position;
	Vec2 anim;
	float velocity;
};

static SpriteRenderer* s_renderer;
static Program s_shader;
static Sampler* s_pointSampler;
static Sampler* s_smoothSampler;
static Texture2D::Handle s_textures[2];
static Vec2 s_cursor;
const int NUM_SMILES = 50;
static Smile s_smiles[NUM_SMILES];

void prepareSprites()
{
	s_pointSampler = new Sampler(Sampler::Filter::POINT, Sampler::Filter::POINT, Sampler::Filter::POINT, Sampler::Border::CLAMP);
	s_smoothSampler = new Sampler(Sampler::Filter::LINEAR, Sampler::Filter::LINEAR, Sampler::Filter::LINEAR, Sampler::Border::CLAMP);
	s_textures[0] = Texture2DManager::get("textures/cursor.png", *s_pointSampler);
	s_textures[1] = Texture2DManager::get("textures/smile.png", *s_pointSampler);
	s_renderer = new SpriteRenderer;

	s_renderer->defSprite(0.0f, 1.0f, s_textures[0]);
	s_renderer->defSprite(0.5f, 0.5f, s_textures[1], 0, 0, 22, 22, 6, 6);

	for(int i = 0; i < NUM_SMILES; ++i)
		s_smiles[i].position = Vec3(float(rand() % getWindowWidth()), float(rand() % (getWindowHeight()/2)) + getWindowHeight()/3, 0.01f);
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

void update(float _time)
{
	s_renderer->clearInstances();

	s_renderer->newInstance(0, Vec3(s_cursor.x, getWindowHeight()-s_cursor.y, 0.0f), 0.0f, Vec2(1.0f));

	for(int i = 0; i < NUM_SMILES; ++i)
	{
		s_smiles[i].anim.x = _time * 2.0f + s_smiles[i].position.x * 0.02f;
		s_smiles[i].velocity -= 0.01f;
		s_smiles[i].position.y += s_smiles[i].velocity;
		if(s_smiles[i].position.y < 20.0f) s_smiles[i].velocity = -s_smiles[i].velocity;
		s_smiles[i].anim.y = clamp(7.0f * s_smiles[i].position.y / (getWindowHeight()) - 0.5f, 0.0f, 6.0f);
		s_renderer->newInstance(1, s_smiles[i].position, 0.0f, Vec2(3.0f), s_smiles[i].anim.x, s_smiles[i].anim.y);
	}
}

void runMainLoop(GLFWwindow* _window)
{
	//uint64 time0 = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	Device::setCullMode(CullMode::BACK);
	Device::setZFunc(ComparisonFunc::LEQUAL);
	while(!glfwWindowShouldClose(_window))
	{
		//float time = float( std::chrono::high_resolution_clock::now().time_since_epoch().count() - time0 ) / 5000000.0f;
		float time = clock() / 1000.0f;
		glfwPollEvents();
		update(time);
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