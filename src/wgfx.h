#pragma once

#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#define SDL_MAIN_HANDLED
#include <sdl2webgpu.h>
#include <SDL.h>

#include <iostream>
#include <cassert>
#include <vector>
#include <filesystem>
#include <fstream>

using namespace wgpu;

namespace wgfx
{
	Device device = nullptr;
	Queue queue = nullptr;

	BufferDescriptor bufferDesc;
	struct Uniform
	{
		Buffer buffer;
		BindGroupEntry binding;

		Uniform(float a) // need a wgfx::createUniform
		{
			// Create uniform buffer
	// The buffer will only contain 1 float with the value of uTime
			bufferDesc.size = sizeof(float);
			// Make sure to flag the buffer as BufferUsage::Uniform
			bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Uniform;
			bufferDesc.mappedAtCreation = false;
			buffer = device.createBuffer(bufferDesc);

			float currentTime = 1.0f;
			queue.writeBuffer(buffer, 0, &currentTime, sizeof(float));

			// Create a binding
			//BindGroupEntry binding;
			// The index of the binding (the entries in bindGroupDesc can be in any order)
			binding.binding = 0;
			// The buffer it is actually bound to
			binding.buffer = buffer;
			// We can specify an offset within the buffer, so that a single buffer can hold
			// multiple uniform blocks.
			binding.offset = 0;
			// And we specify again the size of the buffer.
			binding.size = sizeof(float);

			

		}
	};

	struct VertexBuffer
	{
		std::vector<VertexAttribute> vertexAttribs;
		Buffer buffer;
		uint32_t vertexCount = 0;

		VertexBuffer() {};

		VertexBuffer(std::vector<float> vertices) // need a wgfx::createVertexBuffer()<<<
		{
			// We now divide the vector size by 5 fields.
			vertexCount = static_cast<uint32_t>(vertices.size() / 5);
			//indexCount = static_cast<uint32_t>(indexData.size());


			// Create vertex buffer
			bufferDesc.size = vertices.size() * sizeof(float);
			bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Vertex; // Vertex usage here!
			bufferDesc.mappedAtCreation = false;
			buffer = device.createBuffer(bufferDesc);

			// Upload geometry data to the buffer
			queue.writeBuffer(buffer, 0, vertices.data(), bufferDesc.size);

		}

		void setAttribute(int location, VertexFormat type, int offset)
		{
			VertexAttribute attrib;
			attrib.shaderLocation = location;
			attrib.format = type;
			attrib.offset = offset * sizeof(float);

			vertexAttribs.emplace_back(attrib);

			//all the calculations needed for attributes << 
		}
	};

	struct IndexBuffer
	{
		Buffer buffer;
		uint32_t indexCount;
		
		IndexBuffer() : buffer(nullptr), indexCount(0) {} // allow for no index buff

		IndexBuffer(std::vector<uint16_t> indices)
		{
			indexCount = static_cast<uint32_t>(indices.size());

			bufferDesc.size = indices.size() * sizeof(uint16_t);
			bufferDesc.size = (bufferDesc.size + 3) & ~3; // round up to the next multiple of 4
			bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Index;
			buffer = device.createBuffer(bufferDesc);

			queue.writeBuffer(buffer, 0, indices.data(), bufferDesc.size);
		}

	};


	TextureFormat surfaceFormat = TextureFormat::Undefined;

	struct Program
	{
		RenderPipeline pipeline;

		RenderPipelineDescriptor pipelineDesc;
		ShaderModule shaderModule;

		VertexBuffer vertexBuffer;
		IndexBuffer indexBuffer;

		Program()
		{
			std::cout << "Creating render pipeline..." << std::endl;
			pipelineDesc = RenderPipelineDescriptor();


		}

		void setIndexBuffer(IndexBuffer buffer)
		{
			indexBuffer = buffer;
		}
		BindGroupLayout bindGroupLayout;
		BindGroupLayoutDescriptor bindGroupLayoutDesc;
		void setVertexBuffer(VertexBuffer buffer) // take in a vbo? yuh, yuh? well what exactly is a vbo? vertexbufferhandle, it is an object which allows attribs
		{
			vertexBuffer = buffer;

			VertexBufferLayout vertexBufferLayout;
			vertexBufferLayout.attributeCount = (uint32_t)buffer.vertexAttribs.size();
			vertexBufferLayout.attributes = buffer.vertexAttribs.data();
			vertexBufferLayout.arrayStride = 5 * sizeof(float);
			vertexBufferLayout.stepMode = VertexStepMode::Vertex;

			pipelineDesc.vertex.bufferCount = 1;
			pipelineDesc.vertex.buffers = &vertexBufferLayout;

			pipelineDesc.vertex.module = shaderModule;
			pipelineDesc.vertex.entryPoint = "vs_main";
			pipelineDesc.vertex.constantCount = 0;
			pipelineDesc.vertex.constants = nullptr;

			pipelineDesc.primitive.topology = PrimitiveTopology::TriangleList;
			pipelineDesc.primitive.stripIndexFormat = IndexFormat::Undefined;
			pipelineDesc.primitive.frontFace = FrontFace::CCW;
			pipelineDesc.primitive.cullMode = CullMode::None;

			FragmentState fragmentState;
			pipelineDesc.fragment = &fragmentState;
			fragmentState.module = shaderModule;
			fragmentState.entryPoint = "fs_main";
			fragmentState.constantCount = 0;
			fragmentState.constants = nullptr;

			BlendState blendState{};
			blendState.color.srcFactor = BlendFactor::SrcAlpha;
			blendState.color.dstFactor = BlendFactor::OneMinusSrcAlpha;
			blendState.color.operation = BlendOperation::Add;
			blendState.alpha.srcFactor = BlendFactor::Zero;
			blendState.alpha.dstFactor = BlendFactor::One;
			blendState.alpha.operation = BlendOperation::Add;

			ColorTargetState colorTarget;
			colorTarget.format = surfaceFormat;
			colorTarget.blend = &blendState;
			colorTarget.writeMask = ColorWriteMask::All;

			fragmentState.targetCount = 1;
			fragmentState.targets = &colorTarget;

			pipelineDesc.depthStencil = nullptr;

			pipelineDesc.multisample.count = 1;
			pipelineDesc.multisample.mask = ~0u;
			pipelineDesc.multisample.alphaToCoverageEnabled = false;

			// Create binding layout (don't forget to = Default)
			BindGroupLayoutEntry bindingLayout = Default;
			// The binding index as used in the @binding attribute in the shader
			bindingLayout.binding = 0;
			// The stage that needs to access this resource
			bindingLayout.visibility = ShaderStage::Vertex;
			bindingLayout.buffer.type = BufferBindingType::Uniform;
			bindingLayout.buffer.minBindingSize = sizeof(float);

			// Create a bind group layout
			bindGroupLayoutDesc;
			bindGroupLayoutDesc.entryCount = 1;
			bindGroupLayoutDesc.entries = &bindingLayout;
			bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDesc);

			// Create the pipeline layout
			PipelineLayoutDescriptor layoutDesc;
			layoutDesc.bindGroupLayoutCount = 1;
			layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&bindGroupLayout;
			PipelineLayout layout = device.createPipelineLayout(layoutDesc);
			pipelineDesc.layout = layout;

			pipeline = device.createRenderPipeline(pipelineDesc);
			std::cout << "Render pipeline: " << pipeline << std::endl;
			shaderModule.release();
		}
		BindGroup bindGroup;
		void linkUniform(Uniform uniform)
		{
			// A bind group contains one or multiple bindings
			BindGroupDescriptor bindGroupDesc;
			bindGroupDesc.layout = bindGroupLayout;
			// There must be as many bindings as declared in the layout!
			bindGroupDesc.entryCount = bindGroupLayoutDesc.entryCount;
			bindGroupDesc.entries = &uniform.binding;
			bindGroup = device.createBindGroup(bindGroupDesc);
		}

		void updateUniform(Uniform uniform, float data)
		{
			queue.writeBuffer(uniform.buffer, 0, &data, sizeof(float));
		}

	};

	Program loadProgram(std::string source)
	{
		// Load the shader module
		ShaderModuleDescriptor shaderDesc;
#ifdef WEBGPU_BACKEND_WGPU
		shaderDesc.hintCount = 0;
		shaderDesc.hints = nullptr;
#endif

		// We use the extension mechanism to specify the WGSL part of the shader module descriptor
		ShaderModuleWGSLDescriptor shaderCodeDesc;
		// Set the chained struct's header
		shaderCodeDesc.chain.next = nullptr;
		shaderCodeDesc.chain.sType = SType::ShaderModuleWGSLDescriptor;
		// Connect the chain
		shaderDesc.nextInChain = &shaderCodeDesc.chain;
		shaderCodeDesc.code = source.c_str();
		ShaderModule shaderModule = device.createShaderModule(shaderDesc);

		Program program;
		program.shaderModule = shaderModule;
		return program;
	}

	RenderPassEncoder renderPass = nullptr;

	CommandEncoder encoder = nullptr;
	TextureView targetView = nullptr;

	//SDL_Window* window = nullptr;
	Surface surface = nullptr;
	std::unique_ptr<ErrorCallback> uncapturedErrorCallbackHandle;

	Instance instance = nullptr;

	uint32_t vertexCount;

	//Program program;
	//VertexBuffer buffer;
	/*Buffer pointBuffer;
	Buffer indexBuffer;
	uint32_t indexCount;*/

	TextureView getNextSurfaceTextureView()
	{
		// Get the surface texture
		SurfaceTexture surfaceTexture;
		surface.getCurrentTexture(&surfaceTexture);
		if (surfaceTexture.status != SurfaceGetCurrentTextureStatus::Success) {
			return nullptr;
		}
		Texture texture = surfaceTexture.texture;

		// Create a view for this surface texture
		TextureViewDescriptor viewDescriptor;
		viewDescriptor.label = "Surface texture view";
		viewDescriptor.format = texture.getFormat();
		viewDescriptor.dimension = TextureViewDimension::_2D;
		viewDescriptor.baseMipLevel = 0;
		viewDescriptor.mipLevelCount = 1;
		viewDescriptor.baseArrayLayer = 0;
		viewDescriptor.arrayLayerCount = 1;
		viewDescriptor.aspect = TextureAspect::All;
		TextureView targetView = texture.createView(viewDescriptor);

		return targetView;
	}

	Surface getSurface(SDL_Window* window)
	{
		instance = wgpuCreateInstance(nullptr);
		std::cout << "Requesting adapter..." << std::endl;
		return surface = SDL_GetWGPUSurface(instance, window);
	}

	void initbuffers()
	{
		// Vertex buffer data
		/*
		std::vector<float> vertexData = {
			// x0,  y0,  r0,  g0,  b0
			-0.5, -0.5, 1.0, 0.0, 0.0,

			// x1,  y1,  r1,  g1,  b1
			+0.5, -0.5, 0.0, 1.0, 0.0,

			// ...
			+0.0,   +0.5, 0.0, 0.0, 1.0,
			-0.55f, -0.5, 1.0, 1.0, 0.0,
			-0.05f, +0.5, 1.0, 0.0, 1.0,
			-0.55f, +0.5, 0.0, 1.0, 1.0
		};
		*/
		/*
		std::vector<float> pointData = {
			// x,   y,     r,   g,   b
			-0.5, -0.5,   1.0, 0.0, 0.0, // Point #0
			+0.5, -0.5,   0.0, 1.0, 0.0, // Point #1
			+0.5, +0.5,   0.0, 0.0, 1.0, // Point #2
			-0.5, +0.5,   1.0, 1.0, 0.0  // Point #3
		};

		// Define index data
		// This is a list of indices referencing positions in the pointData
		std::vector<uint16_t> indexData = {
			0, 1, 2, // Triangle #0 connects points #0, #1 and #2
			0, 2, 3  // Triangle #1 connects points #0, #2 and #3
		};

		// We now divide the vector size by 5 fields.
		//vertexCount = static_cast<uint32_t>(vertexData.size() / 5);
		indexCount = static_cast<uint32_t>(indexData.size());

	
		// Create vertex buffer
		BufferDescriptor bufferDesc;
		bufferDesc.size = pointData.size() * sizeof(float);
		bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Vertex; // Vertex usage here!
		bufferDesc.mappedAtCreation = false;
		pointBuffer = device.createBuffer(bufferDesc);

		// Upload geometry data to the buffer
		queue.writeBuffer(pointBuffer, 0, pointData.data(), bufferDesc.size);
	
		bufferDesc.size = indexData.size() * sizeof(uint16_t);
		bufferDesc.size = (bufferDesc.size + 3) & ~3; // round up to the next multiple of 4
		bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Index;
		indexBuffer = device.createBuffer(bufferDesc);

		queue.writeBuffer(indexBuffer, 0, indexData.data(), bufferDesc.size);
		*/
	}

	void init(Surface surface, int width, int height)
	{
		//Instance instance = wgpuCreateInstance(nullptr);

		//std::cout << "Requesting adapter..." << std::endl;
		//surface = SDL_GetWGPUSurface(instance, window);
		RequestAdapterOptions adapterOpts = {};
		adapterOpts.compatibleSurface = surface;
		Adapter adapter = instance.requestAdapter(adapterOpts);
		std::cout << "Got adapter: " << adapter << std::endl;

		instance.release();

		std::cout << "Requesting device..." << std::endl;
		DeviceDescriptor deviceDesc = {};
		deviceDesc.label = "My Device";
		deviceDesc.requiredFeatureCount = 0;
		deviceDesc.requiredLimits = nullptr;
		deviceDesc.defaultQueue.nextInChain = nullptr;
		deviceDesc.defaultQueue.label = "The default queue";
		deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason, char const* message, void* /* pUserData */) {
			std::cout << "Device lost: reason " << reason;
			if (message) std::cout << " (" << message << ")";
			std::cout << std::endl;
			};
		device = adapter.requestDevice(deviceDesc);
		std::cout << "Got device: " << device << std::endl;

		uncapturedErrorCallbackHandle = device.setUncapturedErrorCallback([](ErrorType type, char const* message) {
			std::cout << "Uncaptured device error: type " << type;
			if (message) std::cout << " (" << message << ")";
			std::cout << std::endl;
			});

		queue = device.getQueue();

		std::cout << "Creating swapchain...\n";
		// Configure the surface
		SurfaceConfiguration config = {};

		// Configuration of the textures created for the underlying swap chain
		config.width = width;
		config.height = height;
		config.usage = TextureUsage::RenderAttachment;
		surfaceFormat = surface.getPreferredFormat(adapter);
		config.format = surfaceFormat;//TextureFormat::BGRA8Unorm; //surfaceFormat

		// And we do not need any particular view format:
		config.viewFormatCount = 0;
		config.viewFormats = nullptr;
		config.device = device;
		config.presentMode = PresentMode::Fifo;
		config.alphaMode = CompositeAlphaMode::Auto;

		surface.configure(config);

		// Release the adapter only after it has been fully utilized
		adapter.release();



		/*
		program = wgfx::loadProgram(shaderSource);
		buffer.setAttribute(0, 0);
		buffer.setAttribute(1, 2);
		program.setVertexBuffer(buffer);
		*/

		initbuffers();
	}

	RenderPassDescriptor renderPassDesc = {}; // ought be in a view < struct
	void touch()
	{
	}

	void submit(Program program)
	{
		// Get the next target texture view
		targetView = getNextSurfaceTextureView();
		if (!targetView) return;

		// Create a command encoder for the draw call
		CommandEncoderDescriptor encoderDesc = {};
		encoderDesc.label = "My command encoder";
		encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc);

		// Create the render pass that clears the screen with our color
		renderPassDesc = {};

		// The attachment part of the render pass descriptor describes the target texture of the pass
		RenderPassColorAttachment renderPassColorAttachment = {};
		renderPassColorAttachment.view = targetView;
		renderPassColorAttachment.resolveTarget = nullptr;
		renderPassColorAttachment.loadOp = LoadOp::Clear;
		renderPassColorAttachment.storeOp = StoreOp::Store;
		renderPassColorAttachment.clearValue = WGPUColor{ 0.05, 0.05, 0.05, 1.0 };
#ifndef WEBGPU_BACKEND_WGPU
		renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif // NOT WEBGPU_BACKEND_WGPU

		renderPassDesc.colorAttachmentCount = 1;
		renderPassDesc.colorAttachments = &renderPassColorAttachment;
		renderPassDesc.depthStencilAttachment = nullptr;
		renderPassDesc.timestampWrites = nullptr;
		
		renderPass = encoder.beginRenderPass(renderPassDesc);


		// things pipeline wise\
		
		renderPass.setPipeline(program.pipeline);
		renderPass.setVertexBuffer(0, program.vertexBuffer.buffer, 0, program.vertexBuffer.buffer.getSize());

		renderPass.setBindGroup(0, program.bindGroup, 0, nullptr);
		//renderPass.draw(vertexCount, 1, 0, 0);
		if (program.indexBuffer.buffer) {
		renderPass.setIndexBuffer(program.indexBuffer.buffer, IndexFormat::Uint16, 0, program.indexBuffer.buffer.getSize());
		renderPass.drawIndexed(program.indexBuffer.indexCount, 1, 0, 0, 0);
		} else {
			renderPass.draw(program.vertexBuffer.vertexCount, 1, 0, 0);
		}

		renderPass.end();
		renderPass.release();
		// Finally encode and submit the render pass
		CommandBufferDescriptor cmdBufferDescriptor = {};
		cmdBufferDescriptor.label = "Command buffer";
		CommandBuffer command = encoder.finish(cmdBufferDescriptor);
		encoder.release();

		std::cout << "Submitting command..." << std::endl;
		queue.submit(1, &command);
		command.release();
		std::cout << "Command submitted." << std::endl;

		// At the end of the frame
		targetView.release();
#ifndef __EMSCRIPTEN__
		surface.present();
#endif

#if defined(WEBGPU_BACKEND_DAWN)
#elif defined(WEBGPU_BACKEND_WGPU)
		//device.tick();
		//device.poll(false);
		//device.ti
#endif
	}
	
	void frame()
	{
	}

	inline std::string loadFromFile(const std::filesystem::path& path)
	{
		std::ifstream file(path);
		if (!file.is_open()) {
			return nullptr;
		}
		file.seekg(0, std::ios::end);
		size_t size = file.tellg();
		std::string shaderSource(size, ' ');
		file.seekg(0);
		file.read(shaderSource.data(), size);

		return shaderSource;
	}

	// types
	wgpu::VertexFormat vec2f = VertexFormat::Float32x2;
	wgpu::VertexFormat vec3f = VertexFormat::Float32x3;
}