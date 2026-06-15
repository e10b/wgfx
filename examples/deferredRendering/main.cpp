#define WGPU_IMPLEMENTATION
#include <wgfx.h>

#include <array>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <random>
#include <vector>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_wgpu.h>

#include "examples/common/stanfordDragonMesh.h"
#include "helper/clock.h"
#include "helper/constants.h"
#include "helper/context.h"

namespace
{
constexpr uint32_t kMaxNumLights = 1024;
constexpr uint32_t kNumLights = 128;

struct GBufferUniforms {
	glm::mat4 modelMatrix;
	glm::mat4 normalModelMatrix;
	glm::mat4 viewProjectionMatrix;
	glm::mat4 invViewProjectionMatrix;
};

struct CameraUniforms {
	glm::mat4 viewProjectionMatrix;
	glm::mat4 invViewProjectionMatrix;
};

struct LightData {
	glm::vec4 position;
	glm::vec4 colorRadius;
};

struct LightExtent {
	glm::vec4 min;
	glm::vec4 max;
};

struct Settings {
	int mode = 0;
	int numLights = kNumLights;
};
}

int main()
{
	Context& context = Context::Instance();
	auto* surface = new wgfx::ColorTexture();
	auto* depth = new wgfx::DepthTexture(wgpu::TextureFormat::Depth24Plus);
	auto* gNormal = new wgfx::ColorTexture(false, wgpu::TextureFormat::RGBA16Float);
	auto* gAlbedo = new wgfx::ColorTexture(false, wgpu::TextureFormat::BGRA8Unorm);

	auto* gPass = new wgfx::RenderPass();
	gPass->addTarget(gNormal);
	gPass->addTarget(gAlbedo);
	gPass->addTarget(depth);
	gPass->setClear({0.0, 0.0, 1.0, 1.0});

	auto* lightPass = new wgfx::RenderPass();
	lightPass->addTarget(surface);
	lightPass->setClear({0.0, 0.0, 0.0, 1.0});
	auto* uiPass = new wgfx::RenderPass();
	uiPass->addTarget(surface);
	uiPass->shouldClear = false;

	auto vertices = sample_mesh::stanfordDragonDeferredVertices();
	auto indices = sample_mesh::stanfordDragonIndices();
	auto* vbo = wgfx::createVertexBuffer(vertices);
	vbo->setAttribute(0, wgfx::vec3f, 0);
	vbo->setAttribute(1, wgfx::vec3f, 3);
	vbo->setAttribute(2, wgfx::vec2f, 6);
	auto* ibo = wgfx::createIndexBuffer(indices);

	auto* gPipeline = wgfx::loadPipeline(wgfx::loadFromFile(std::filesystem::path(RESOURCE_DIR) / "gbuffer.wgsl"));
	gPipeline->targets = 2;
	gPipeline->targetFormats[0] = wgpu::TextureFormat::RGBA16Float;
	gPipeline->targetFormats[1] = wgpu::TextureFormat::BGRA8Unorm;
	gPipeline->depthFormat = wgpu::TextureFormat::Depth24Plus;
	gPipeline->cullMode = wgpu::CullMode::Back;
	gPipeline->addUniform(0, sizeof(GBufferUniforms));
	gPipeline->useDepth = true;
	gPipeline->init(vbo);

	std::mt19937 rng(11);
	std::uniform_real_distribution<float> unit(0.0f, 1.0f);
	const glm::vec3 lightExtentMin(-50.0f, -30.0f, -50.0f);
	const glm::vec3 lightExtentMax(50.0f, 50.0f, 50.0f);
	const glm::vec3 extent = lightExtentMax - lightExtentMin;
	std::vector<LightData> lights(kMaxNumLights);
	for (uint32_t i = 0; i < kMaxNumLights; ++i) {
		glm::vec3 position = lightExtentMin + glm::vec3(unit(rng), unit(rng), unit(rng)) * extent;
		glm::vec3 color(unit(rng) * 2.0f, unit(rng) * 2.0f, unit(rng) * 2.0f);
		lights[i] = {glm::vec4(position, 1.0f), glm::vec4(color, 20.0f)};
	}

	auto* lightUpdate = wgfx::loadCompute(wgfx::loadFromFile(std::filesystem::path(RESOURCE_DIR) / "lightUpdate.wgsl"));
	lightUpdate->entryPoint = "cs_main";
	auto* lightsStorage = lightUpdate->addStorage(0, sizeof(LightData) * lights.size(), lights.data());
	lightUpdate->addUniform(1);
	lightUpdate->addUniform(2);
	lightUpdate->init();

	wgfx::Texture normalTexture = wgfx::loadTexture(gNormal->colorView);
	wgfx::Texture albedoTexture = wgfx::loadTexture(gAlbedo->colorView);
	auto* lighting = wgfx::loadPipeline(wgfx::loadFromFile(std::filesystem::path(RESOURCE_DIR) / "lighting.wgsl"));
	lighting->useDepth = false;
	lighting->vertices = 6;
	lighting->addTexture(0, normalTexture);
	lighting->addTexture(1, albedoTexture);
	lighting->addDepthTexture(2, depth->depthView);
	auto* renderLightsStorage = lighting->addStorage(3, sizeof(LightData) * lights.size(), nullptr, true);
	renderLightsStorage->buffer = lightsStorage->buffer;
	renderLightsStorage->entry.buffer = lightsStorage->buffer;
	lighting->uniforms.entries.back().buffer = lightsStorage->buffer;
	lighting->addUniform(4);
	lighting->addUniform(5, sizeof(CameraUniforms));
	lighting->init(nullptr);

	auto* debugView = wgfx::loadPipeline(wgfx::loadFromFile(std::filesystem::path(RESOURCE_DIR) / "debugView.wgsl"));
	debugView->useDepth = false;
	debugView->vertices = 6;
	debugView->addTexture(0, normalTexture);
	debugView->addTexture(1, albedoTexture);
	debugView->addDepthTexture(2, depth->depthView);
	debugView->init(nullptr);

	const glm::vec3 upVector(0.0f, 1.0f, 0.0f);
	const glm::vec3 origin(0.0f);
	const glm::mat4 projectionMatrix = glm::perspective((2.0f * glm::pi<float>()) / 5.0f, float(raytraceWidth) / float(raytraceHeight), 1.0f, 2000.0f);
	const glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -45.0f, 0.0f));
	const glm::mat4 normalModelMatrix = glm::transpose(glm::inverse(modelMatrix));
	const LightExtent lightExtent{glm::vec4(lightExtentMin, 0.0f), glm::vec4(lightExtentMax, 0.0f)};
	std::array<uint32_t, 16> configData{};
	configData[0] = kNumLights;
	std::array<float, 16> extentData{};
	std::memcpy(extentData.data(), &lightExtent, sizeof(lightExtent));

	wgfx::ComputePass computePass;
	Settings settings;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplSDL3_InitForOther(context.window);
	ImGui_ImplWGPU_InitInfo imguiInit{};
	imguiInit.Device = wgfx::device;
	imguiInit.NumFramesInFlight = 3;
	imguiInit.RenderTargetFormat = wgfx::surfaceFormat;
	imguiInit.DepthStencilFormat = WGPUTextureFormat_Undefined;
	imguiInit.PipelineMultisampleState.count = 1;
	imguiInit.PipelineMultisampleState.mask = UINT32_MAX;
	imguiInit.PipelineMultisampleState.alphaToCoverageEnabled = false;
	ImGui_ImplWGPU_Init(&imguiInit);
	context.eventCallback = [](const SDL_Event& event) {
		ImGui_ImplSDL3_ProcessEvent(&event);
	};

	Clock clock;
	float time = 0.0f;
	while (!context.close) {
		time += clock.restart();
		context.update();
		ImGui_ImplWGPU_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowPos(ImVec2(12.0f, 12.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(260.0f, 0.0f), ImGuiCond_FirstUseEver);
		ImGui::Begin("Deferred Rendering");
		const char* modes[] = {"rendering", "gBuffers view"};
		ImGui::Combo("mode", &settings.mode, modes, IM_ARRAYSIZE(modes));
		ImGui::SliderInt("numLights", &settings.numLights, 1, int(kMaxNumLights));
		ImGui::End();

		wgfx::touch(surface);
		wgfx::start();

		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::pi<float>() * time / 5.0f, upVector);
		glm::vec3 eyePosition = glm::vec3(rotation * glm::vec4(0.0f, 50.0f, -100.0f, 1.0f));
		glm::mat4 viewMatrix = glm::lookAt(eyePosition, origin, upVector);
		CameraUniforms camera{projectionMatrix * viewMatrix, glm::inverse(projectionMatrix * viewMatrix)};
		GBufferUniforms gbuffer{modelMatrix, normalModelMatrix, camera.viewProjectionMatrix, camera.invViewProjectionMatrix};

		gPipeline->updateUniform(0, glm::value_ptr(gbuffer.modelMatrix));
		gPass->prepare();
		gPipeline->setVertexBuffer(vbo);
		gPipeline->setIndexBuffer(ibo);
		gPass->draw(gPipeline);
		gPass->end();

		configData[0] = static_cast<uint32_t>(settings.numLights);
		lightUpdate->updateUniform(1, reinterpret_cast<const float*>(configData.data()));
		lightUpdate->updateUniform(2, extentData.data());
		computePass.prepare();
		computePass.draw(lightUpdate, (kMaxNumLights + 63) / 64);
		computePass.end();

		lightPass->prepare();
		if (settings.mode == 0) {
			lighting->updateUniform(4, reinterpret_cast<const float*>(configData.data()));
			lighting->updateUniform(5, glm::value_ptr(camera.viewProjectionMatrix));
			lightPass->draw(lighting);
		} else {
			lightPass->draw(debugView);
		}
		lightPass->end();

		ImGui::Render();
		uiPass->prepare();
		ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), uiPass->renderPass);
		uiPass->end();
		context.draw();
	}

	context.eventCallback = nullptr;
	ImGui_ImplWGPU_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
}
