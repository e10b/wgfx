#define WGPU_IMPLEMENTATION
#include <wgfx.h>

#include <cmath>
#include <filesystem>
#include <vector>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "examples/common/stanfordDragonMesh.h"
#include "helper/clock.h"
#include "helper/constants.h"
#include "helper/context.h"

namespace
{
constexpr uint32_t shadowDepthTextureSize = 1024;

struct SceneUniforms {
	glm::mat4 lightViewProjMatrix;
	glm::mat4 cameraViewProjMatrix;
	glm::mat4 modelMatrix;
	glm::vec4 lightPos;
};
}

int main()
{
	Context& context = Context::Instance();
	auto* surface = new wgfx::ColorTexture();
	auto* depth = new wgfx::DepthTexture(wgpu::TextureFormat::Depth24Plus);
	auto* shadowDepth = new wgfx::DepthTexture(shadowDepthTextureSize, shadowDepthTextureSize, wgpu::TextureFormat::Depth32Float);

	auto* shadowPass = new wgfx::RenderPass();
	shadowPass->addTarget(shadowDepth);
	auto* renderPass = new wgfx::RenderPass();
	renderPass->addTarget(surface);
	renderPass->addTarget(depth);
	renderPass->setClear({0.5, 0.5, 0.5, 1.0});

	auto vertices = sample_mesh::stanfordDragonShadowVertices();
	auto indices = sample_mesh::stanfordDragonIndices();
	auto* vbo = wgfx::createVertexBuffer(vertices);
	vbo->setAttribute(0, wgfx::vec3f, 0);
	vbo->setAttribute(1, wgfx::vec3f, 3);
	auto* ibo = wgfx::createIndexBuffer(indices);

	auto* shadowPipeline = wgfx::loadPipeline(wgfx::loadFromFile(std::filesystem::path(RESOURCE_DIR) / "shadowMap.wgsl"));
	shadowPipeline->targets = 0;
	shadowPipeline->depthFormat = wgpu::TextureFormat::Depth32Float;
	shadowPipeline->cullMode = wgpu::CullMode::Back;
	shadowPipeline->addUniform(0, sizeof(SceneUniforms));
	shadowPipeline->useDepth = true;
	shadowPipeline->init(vbo);

	auto* pipeline = wgfx::loadPipeline(wgfx::loadFromFile(std::filesystem::path(RESOURCE_DIR) / "forward.wgsl"));
	pipeline->depthFormat = wgpu::TextureFormat::Depth24Plus;
	pipeline->cullMode = wgpu::CullMode::Back;
	pipeline->addUniform(0, sizeof(SceneUniforms));
	pipeline->addDepthTexture(1, shadowDepth->depthView);
	pipeline->addComparisonSampler(2);
	pipeline->useDepth = true;
	pipeline->init(vbo);

	const glm::vec3 upVector(0.0f, 1.0f, 0.0f);
	const glm::vec3 origin(0.0f);
	const glm::vec3 lightPosition(50.0f, 100.0f, -100.0f);
	const glm::mat4 projectionMatrix = glm::perspective((2.0f * glm::pi<float>()) / 5.0f, float(raytraceWidth) / float(raytraceHeight), 1.0f, 2000.0f);
	const glm::mat4 lightViewMatrix = glm::lookAt(lightPosition, origin, upVector);
	const glm::mat4 lightProjectionMatrix = glm::ortho(-80.0f, 80.0f, -80.0f, 80.0f, -200.0f, 300.0f);
	const glm::mat4 lightViewProjMatrix = lightProjectionMatrix * lightViewMatrix;
	const glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -45.0f, 0.0f));

	Clock clock;
	float time = 0.0f;
	while (!context.close) {
		time += clock.restart();
		context.update();
		wgfx::touch(surface);
		wgfx::start();

		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::pi<float>() * time / 2.0f, upVector);
		glm::vec3 eyePosition = glm::vec3(rotation * glm::vec4(0.0f, 50.0f, -100.0f, 1.0f));
		glm::mat4 viewMatrix = glm::lookAt(eyePosition, origin, upVector);
		SceneUniforms scene{lightViewProjMatrix, projectionMatrix * viewMatrix, modelMatrix, glm::vec4(lightPosition, 1.0f)};

		shadowPipeline->updateUniform(0, glm::value_ptr(scene.lightViewProjMatrix));
		shadowPass->prepare();
		shadowPipeline->setVertexBuffer(vbo);
		shadowPipeline->setIndexBuffer(ibo);
		shadowPass->draw(shadowPipeline);
		shadowPass->end();

		pipeline->updateUniform(0, glm::value_ptr(scene.lightViewProjMatrix));
		renderPass->prepare();
		pipeline->setVertexBuffer(vbo);
		pipeline->setIndexBuffer(ibo);
		renderPass->draw(pipeline);
		renderPass->end();
		context.draw();
	}
}
