#pragma once

#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#define SDL_MAIN_HANDLED
#include <sdl2webgpu.h>
//#include <SDL.h>

#include <SDL3/SDL.h>

#include <iostream>
#include <cassert>
#include <vector>
#include <filesystem>
#include <fstream>

#include <array>

using namespace wgpu;
namespace wgfx
{
	TextureFormat depthTextureFormat = TextureFormat::Depth24Plus;

	Device device = nullptr;
	Queue queue = nullptr;
	Surface surface = nullptr;

	BufferDescriptor bufferDesc;
	struct Uniform
	{
		Buffer buffer;
		BindGroupEntry binding;
		int index;
		size_t scale;

		Uniform(int i, size_t size, float data) // need a wgfx::createUniform
		{
			index = i;
			scale = size;

			bufferDesc.size = size;
			bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Uniform;
			bufferDesc.mappedAtCreation = false;
			buffer = device.createBuffer(bufferDesc);

			queue.writeBuffer(buffer, 0, &data, size);

			binding.binding = i;
			binding.buffer = buffer;
			binding.offset = 0;
			binding.size = size;
		}
		template <size_t N>
		Uniform(int i, size_t size, const float(&array)[N]) // need a wgfx::createUniform
		{
			index = i;
			scale = size;

			bufferDesc.size = size;
			bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Uniform;
			bufferDesc.mappedAtCreation = false;
			buffer = device.createBuffer(bufferDesc);

			queue.writeBuffer(buffer, 0, &array, size);

			binding.binding = i;
			binding.buffer = buffer;
			binding.offset = 0;
			binding.size = size;
		}
	};

	struct VertexBuffer
	{
		std::vector<VertexAttribute> vertexAttribs;
		Buffer buffer;
		uint32_t vertexCount = 0;
		TextureView depthTextureView;
		int fields;

		VertexBuffer() {};

		VertexBuffer(std::vector<float> vertices, int fields) // need a wgfx::createVertexBuffer()<<<
		{
			// We now divide the vector size by 5 fields.
			//vertexCount = static_cast<uint32_t>(vertices.size() / 5);
			this->fields = fields;
			vertexCount = static_cast<uint32_t>(vertices.size() / fields);
			//indexCount = static_cast<uint32_t>(indexData.size());

					// Create the depth texture
					TextureDescriptor depthTextureDesc;
					depthTextureDesc.dimension = TextureDimension::_2D;
					depthTextureDesc.format = depthTextureFormat;
					depthTextureDesc.mipLevelCount = 1;
					depthTextureDesc.sampleCount = 1;
					depthTextureDesc.size = { 1280, 720, 1 };
					depthTextureDesc.usage = TextureUsage::RenderAttachment;
					depthTextureDesc.viewFormatCount = 1;
					depthTextureDesc.viewFormats = (WGPUTextureFormat*)&depthTextureFormat;
					Texture depthTexture = device.createTexture(depthTextureDesc);
					std::cout << "Depth texture: " << depthTexture << std::endl;

					// Create the view of the depth texture manipulated by the rasterizer
					TextureViewDescriptor depthTextureViewDesc;
					depthTextureViewDesc.aspect = TextureAspect::DepthOnly;
					depthTextureViewDesc.baseArrayLayer = 0;
					depthTextureViewDesc.arrayLayerCount = 1;
					depthTextureViewDesc.baseMipLevel = 0;
					depthTextureViewDesc.mipLevelCount = 1;
					depthTextureViewDesc.dimension = TextureViewDimension::_2D;
					depthTextureViewDesc.format = depthTextureFormat;
					depthTextureView = depthTexture.createView(depthTextureViewDesc);
					std::cout << "Depth texture view: " << depthTextureView << std::endl;


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
		std::vector<BindGroupLayoutEntry> entries;
		std::vector<BindGroupEntry> bindings;
		void setVertexBuffer(VertexBuffer buffer) // take in a vbo? yuh, yuh? well what exactly is a vbo? vertexbufferhandle, it is an object which allows attribs
		{
			vertexBuffer = buffer;

			VertexBufferLayout vertexBufferLayout;
			vertexBufferLayout.attributeCount = (uint32_t)buffer.vertexAttribs.size();
			vertexBufferLayout.attributes = buffer.vertexAttribs.data();
			vertexBufferLayout.arrayStride = (buffer.fields) * sizeof(float);
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

					// We setup a depth buffer state for the render pipeline
					DepthStencilState depthStencilState = Default;
					// Keep a fragment only if its depth is lower than the previously blended one
					depthStencilState.depthCompare = CompareFunction::Less;
					// Each time a fragment is blended into the target, we update the value of the Z-buffer
					depthStencilState.depthWriteEnabled = true;
					// Store the format in a variable as later parts of the code depend on it
					//TextureFormat depthTextureFormat = TextureFormat::Depth24Plus;
					depthStencilState.format = depthTextureFormat;
					// Deactivate the stencil alltogether
					depthStencilState.stencilReadMask = 0;
					depthStencilState.stencilWriteMask = 0;

			pipelineDesc.depthStencil = &depthStencilState;

			pipelineDesc.multisample.count = 1;
			pipelineDesc.multisample.mask = ~0u;
			pipelineDesc.multisample.alphaToCoverageEnabled = false;

			// Create a bind group layout
			//bindGroupLayoutDesc;
			bindGroupLayoutDesc.entryCount = entries.size(); // uh
			bindGroupLayoutDesc.entries = entries.data();
			bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDesc);

			// uniform groups


			// Create the pipeline layout
			PipelineLayoutDescriptor layoutDesc;
			layoutDesc.bindGroupLayoutCount = 1;
			layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&bindGroupLayout;
			PipelineLayout layout = device.createPipelineLayout(layoutDesc);
			pipelineDesc.layout = layout;

			pipeline = device.createRenderPipeline(pipelineDesc);
			std::cout << "Render pipeline: " << pipeline << std::endl;
			shaderModule.release();





			// A bind group contains one or multiple bindings
			BindGroupDescriptor bindGroupDesc;
			bindGroupDesc.layout = bindGroupLayout;
			// There must be as many bindings as declared in the layout!
			//bindGroupDesc.entryCount = bindGroupLayoutDesc.entryCount;
			//bindGroupDesc.entries = &uniform.binding;
			bindGroupDesc.entryCount = static_cast<uint32_t>(bindings.size());
			bindGroupDesc.entries = bindings.data(); // Pass the array of entries
			bindGroup = device.createBindGroup(bindGroupDesc);
		}
		

		void setUniform(Uniform uniform)
		{
			BindGroupLayoutEntry bindingLayout = Default;							/// layout needs to be created in joint with the actual entry
			// The binding index as used in the @binding attribute in the shader
			bindingLayout.binding = uniform.index;
			// The stage that needs to access this resource
			bindingLayout.visibility = ShaderStage::Vertex | ShaderStage::Fragment;
			bindingLayout.buffer.type = BufferBindingType::Uniform;
			bindingLayout.buffer.minBindingSize = uniform.scale;


			entries.push_back(bindingLayout);
			bindings.push_back(uniform.binding);
		}

		void touch()
		{
			// Create a bind group layout
			//bindGroupLayoutDesc;
			bindGroupLayoutDesc.entryCount = entries.size(); // uh
			bindGroupLayoutDesc.entries = entries.data();
			bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDesc);
			
			// uniform groups


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

		//std::vector<BindGroupEntry> bindings;
		//void setUniform(Uniform uniform)
		//{
		//	bindings.push_back(uniform.binding);
		//}

		BindGroup bindGroup;
		void linkUniforms()
		{
			// A bind group contains one or multiple bindings
			BindGroupDescriptor bindGroupDesc;
			bindGroupDesc.layout = bindGroupLayout;
			// There must be as many bindings as declared in the layout!
			//bindGroupDesc.entryCount = bindGroupLayoutDesc.entryCount;
			//bindGroupDesc.entries = &uniform.binding;
			bindGroupDesc.entryCount = static_cast<uint32_t>(bindings.size());
			bindGroupDesc.entries = bindings.data(); // Pass the array of entries
			bindGroup = device.createBindGroup(bindGroupDesc);
		}


		void updateUniform(Uniform uniform, float data)
		{
			queue.writeBuffer(uniform.buffer, 0, &data, uniform.scale);
		}
		template <size_t N>
		void updateUniform(Uniform uniform, const float(&array)[N])
		{
			queue.writeBuffer(uniform.buffer, 0, &array, uniform.scale);
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
	SDL_Window* swindow;
	Surface getSurface(SDL_Window* window)
	{
		swindow = window;
		instance = wgpuCreateInstance(nullptr);
		std::cout << "Requesting adapter..." << std::endl;
		return surface = SDL_GetWGPUSurface(instance, window);
	}
	Adapter adapter;
	void initSurface()
	{
		int w, h;
		SDL_GetWindowSize(swindow, &w, &h);

		std::cout << "Creating swapchain...\n";
		// Configure the surface
		SurfaceConfiguration config = {};

		// Configuration of the textures created for the underlying swap chain
		config.width = w;
		config.height = h;
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
		//adapter.release(); << maintain the adapter for reinit surface 
	}
	void destroySurface()
	{
		surface.release();
	}

	void init(Surface surface, int width, int height)
	{
		// INIT DEVICE
		RequestAdapterOptions adapterOpts = {};
		adapterOpts.compatibleSurface = surface;
		adapter = instance.requestAdapter(adapterOpts);
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



		// INIT SURFACE(SWAP)
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
		//adapter.release();



		/*
		program = wgfx::loadProgram(shaderSource);
		buffer.setAttribute(0, 0);
		buffer.setAttribute(1, 2);
		program.setVertexBuffer(buffer);
		*/

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
		//renderPassDesc = {};

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

				// We now add a depth/stencil attachment:
				RenderPassDepthStencilAttachment depthStencilAttachment;
				// The view of the depth texture
				depthStencilAttachment.view = program.vertexBuffer.depthTextureView;

				// The initial value of the depth buffer, meaning "far"
				depthStencilAttachment.depthClearValue = 1.0f;
				// Operation settings comparable to the color attachment
				depthStencilAttachment.depthLoadOp = LoadOp::Clear;
				depthStencilAttachment.depthStoreOp = StoreOp::Store;
				// we could turn off writing to the depth buffer globally here
				depthStencilAttachment.depthReadOnly = false;

				// Stencil setup, mandatory but unused
				depthStencilAttachment.stencilClearValue = 0;
		#ifdef WEBGPU_BACKEND_WGPU
				depthStencilAttachment.stencilLoadOp = LoadOp::Clear;
				depthStencilAttachment.stencilStoreOp = StoreOp::Store;
		#else
				depthStencilAttachment.stencilLoadOp = LoadOp::Undefined;
				depthStencilAttachment.stencilStoreOp = StoreOp::Undefined;
		#endif
				depthStencilAttachment.stencilReadOnly = true;

				renderPassDesc.depthStencilAttachment = &depthStencilAttachment;


		//renderPassDesc.depthStencilAttachment = nullptr;
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

		//std::cout << "Submitting command..." << std::endl;
		queue.submit(1, &command);
		command.release();
		//std::cout << "Command submitted." << std::endl;

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