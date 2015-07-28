#include <mir.hpp>
#include <thread>
#include <ctime>
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <ei/vector.hpp>
#include "stdwindow.hpp"

using namespace MiR;
using namespace ei;

static InstanceRenderer* s_renderer;
static Program s_shader;
static float s_aspectRatio;
static Sampler* s_sampler;
static Texture2D::Handle s_texture;
static int s_meshID;

void prepareBoxes()
{
	// Create quad renderer with positions and normals
	VertexAttribute attr[3] = {{PrimitiveFormat::FLOAT, 3, false},
							   {PrimitiveFormat::FLOAT, 3, false},
							   {PrimitiveFormat::FLOAT, 2, false}};
	s_renderer = new InstanceRenderer(InstanceRenderer::PrimitiveType::TRIANGLE_STRIPE, attr, 3);
	s_meshID = s_renderer->beginDef();
		s_renderer->put(1, Vec3(-1.0f,  0.0f,  0.0f));
		s_renderer->put(0, Vec3(-1.0f, -1.0f, -1.0f)); s_renderer->put(2, Vec2(0.0f, 0.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3(-1.0f, -1.0f,  1.0f)); s_renderer->put(2, Vec2(0.0f, 1.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3(-1.0f,  1.0f, -1.0f)); s_renderer->put(2, Vec2(1.0f, 0.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3(-1.0f,  1.0f,  1.0f)); s_renderer->put(2, Vec2(1.0f, 1.0f)); s_renderer->emit();
		s_renderer->endPrimitive();

		s_renderer->put(1, Vec3( 1.0f,  0.0f,  0.0f));
		s_renderer->put(0, Vec3( 1.0f, -1.0f, -1.0f)); s_renderer->put(2, Vec2(0.0f, 0.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3( 1.0f, -1.0f,  1.0f)); s_renderer->put(2, Vec2(0.0f, 1.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3( 1.0f,  1.0f, -1.0f)); s_renderer->put(2, Vec2(1.0f, 0.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3( 1.0f,  1.0f,  1.0f)); s_renderer->put(2, Vec2(1.0f, 1.0f)); s_renderer->emit();
		s_renderer->endPrimitive();

		s_renderer->put(1, Vec3( 0.0f, -1.0f,  0.0f));
		s_renderer->put(0, Vec3(-1.0f, -1.0f, -1.0f)); s_renderer->put(2, Vec2(0.0f, 0.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3(-1.0f, -1.0f,  1.0f)); s_renderer->put(2, Vec2(0.0f, 1.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3( 1.0f, -1.0f, -1.0f)); s_renderer->put(2, Vec2(1.0f, 0.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3( 1.0f, -1.0f,  1.0f)); s_renderer->put(2, Vec2(1.0f, 1.0f)); s_renderer->emit();
		s_renderer->endPrimitive();

		s_renderer->put(1, Vec3( 0.0f,  1.0f,  0.0f));
		s_renderer->put(0, Vec3(-1.0f,  1.0f, -1.0f)); s_renderer->put(2, Vec2(0.0f, 0.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3(-1.0f,  1.0f,  1.0f)); s_renderer->put(2, Vec2(0.0f, 1.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3( 1.0f,  1.0f, -1.0f)); s_renderer->put(2, Vec2(1.0f, 0.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3( 1.0f,  1.0f,  1.0f)); s_renderer->put(2, Vec2(1.0f, 1.0f)); s_renderer->emit();
		s_renderer->endPrimitive();

		s_renderer->put(1, Vec3( 0.0f,  0.0f, -1.0f));
		s_renderer->put(0, Vec3( 1.0f, -1.0f, -1.0f)); s_renderer->put(2, Vec2(0.0f, 0.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3( 1.0f,  1.0f, -1.0f)); s_renderer->put(2, Vec2(0.0f, 1.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3(-1.0f, -1.0f, -1.0f)); s_renderer->put(2, Vec2(1.0f, 0.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3(-1.0f,  1.0f, -1.0f)); s_renderer->put(2, Vec2(1.0f, 1.0f)); s_renderer->emit();
		s_renderer->endPrimitive();

		s_renderer->put(1, Vec3( 0.0f,  0.0f,  1.0f));
		s_renderer->put(0, Vec3( 1.0f, -1.0f,  1.0f)); s_renderer->put(2, Vec2(0.0f, 0.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3( 1.0f,  1.0f,  1.0f)); s_renderer->put(2, Vec2(0.0f, 1.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3(-1.0f, -1.0f,  1.0f)); s_renderer->put(2, Vec2(1.0f, 0.0f)); s_renderer->emit();
		s_renderer->put(0, Vec3(-1.0f,  1.0f,  1.0f)); s_renderer->put(2, Vec2(1.0f, 1.0f)); s_renderer->emit();
		s_renderer->endPrimitive();
	s_renderer->endDef();

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
		glCall(glClearColor, 0.0f, 0.0f, 0.0f, 0.0f);
		glCall(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		s_shader.use();
		s_texture->bind(0);
		Mat4x4 viewProj = ei::perspectiveGL(PI/4, s_aspectRatio, 1.1f, 1000.0f)
			* ei::camera(rotationY(time / 10.0f) * Vec3(0.0f, 100.0f, -220.0f), Vec3(0.0f, -50.0f, 0.0f));
		glCall(glUniformMatrix4fv, 0, 1, GL_FALSE, (GLfloat*)&viewProj);

		s_renderer->draw();
		
		glfwPollEvents();
		// Prepare next frame
		s_renderer->clearInstances();
		for(int x = -30; x < 30; ++x)
		{
			for(int y = -30; y < 30; ++y)
			{
				Vec3 pos(x + 0.5f, 0.0f, y + 0.5f);
				pos.y = len(pos);
				if(pos.y <= 30)
				{
					pos.x *= 5.0f; pos.z *= 5.0f;
					pos.y = sin(pos.y * 0.75f - time) * 50.0f / (pos.y + 6.0f);
					Vec3 dir = normalize(pos);
					Vec3 axis = normalize(cross(Vec3(0.0f, 1.0f, 0.0f), dir));
					s_renderer->newInstance(s_meshID, pos, Quaternion(Vec3(0.0f, 1.0f, 0.0f), acos(dir.x)) * Quaternion(axis, pos.y * 0.125f));
				}
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		glfwSwapBuffers(_window);
	}
}

int main()
{
	GLFWwindow* window = setupStdWindow("MiR cube demo.");
	if(!window) return 1;
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	s_aspectRatio = width / (float)height;

	prepareBoxes();
	prepareShader();
	runMainLoop(window);

	delete s_sampler;
	delete s_renderer;
	glfwDestroyWindow(window);
	return 0;
}