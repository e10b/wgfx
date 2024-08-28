#pragma once

#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

#define SDL_MAIN_HANDLED
#include <sdl2webgpu.h>
#include <SDL.h>

#include <iostream>
#include <cassert>
#include <vector>

using namespace wgpu;

const char* shaderSource = R"(
/**
 * A structure with fields labeled with vertex attribute locations can be used
 * as input to the entry point of a shader.
 */
struct VertexInput {
	@location(0) position: vec2f,
	@location(1) color: vec3f,
};

/**
 * A structure with fields labeled with builtins and locations can also be used
 * as *output* of the vertex shader, which is also the input of the fragment
 * shader.
 */
struct VertexOutput {
	@builtin(position) position: vec4f,
	// The location here does not refer to a vertex attribute, it just means
	// that this field must be handled by the rasterizer.
	// (It can also refer to another field of another struct that would be used
	// as input to the fragment shader.)
	@location(0) color: vec3f,
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	//                         ^^^^^^^^^^^^ We return a custom struct
	var out: VertexOutput; // create the output struct
	out.position = vec4f(in.position, 0.0, 1.0); // same as what we used to directly return
	out.color = in.color; // forward the color attribute to the fragment shader
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
	//     ^^^^^^^^^^^^^^^^ Use for instance the same struct as what the vertex outputs
	return vec4f(in.color, 1.0); // use the interpolated color coming from the vertex shader
}
)";

namespace wgfx
{

	struct VertexBuffer
	{
		std::vector<VertexAttribute> vertexAttribs;

		void setAttribute(int location, int type)
		{
			/*
			vertexAttribs[location].shaderLocation = location;
			if (type == 0) {
				vertexAttribs[location].format = VertexFormat::Float32x2;

			}
			else
			{
				vertexAttribs[location].format = VertexFormat::Float32x3;
			}
			vertexAttribs[location].offset = type * sizeof(float);
			*/
			
			
			
			VertexAttribute attrib;
			attrib.shaderLocation = location;
			if (type == 0) { attrib.format = VertexFormat::Float32x2; }
			else { attrib.format = VertexFormat::Float32x3; }
			attrib.offset = type * sizeof(float);

			vertexAttribs.emplace_back(attrib);

			//all the calculations needed for attributes << 
		}
	};
	Device device = nullptr;
	TextureFormat surfaceFormat = TextureFormat::Undefined;

	struct Program
	{
		RenderPipeline pipeline;

		RenderPipelineDescriptor pipelineDesc;
		ShaderModule shaderModule;

		Program()
		{
			std::cout << "Creating render pipeline..." << std::endl;
			pipelineDesc = RenderPipelineDescriptor();


		}


		void setVertexBuffer(VertexBuffer buffer) // take in a vbo? yuh, yuh? well what exactly is a vbo? vertexbufferhandle, it is an object which allows attribs
		{
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

			fragmentState.targetCount = 1;
			fragmentState.targets = &colorTarget;
			pipelineDesc.fragment = &fragmentState;

			// We do not use stencil/depth testing for now
			pipelineDesc.depthStencil = nullptr;

			// Samples per pixel
			pipelineDesc.multisample.count = 1;

			// Default value for the mask, meaning "all bits on"
			pipelineDesc.multisample.mask = ~0u;

			// Default value as well (irrelevant for count = 1 anyways)
			pipelineDesc.multisample.alphaToCoverageEnabled = false;
			pipelineDesc.layout = nullptr;

			pipeline = device.createRenderPipeline(pipelineDesc);
			shaderModule.release();
		}
	};

	Program loadProgram(const char* source)
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
		shaderCodeDesc.code = source;
		ShaderModule shaderModule = device.createShaderModule(shaderDesc);

		Program program;
		program.shaderModule = shaderModule;
		return program;
	}

	RenderPassEncoder renderPass = nullptr;

	CommandEncoder encoder = nullptr;
	TextureView targetView = nullptr;

	//SDL_Window* window = nullptr;
	Queue queue = nullptr;
	Surface surface = nullptr;
	std::unique_ptr<ErrorCallback> uncapturedErrorCallbackHandle;

	Instance instance = nullptr;

	uint32_t vertexCount;

	Program program;
	VertexBuffer buffer;
	Buffer vertexBuffer;

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

		// We now divide the vector size by 5 fields.
		vertexCount = static_cast<uint32_t>(vertexData.size() / 5);

		// Create vertex buffer
		BufferDescriptor bufferDesc;
		bufferDesc.size = vertexData.size() * sizeof(float);
		bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Vertex; // Vertex usage here!
		bufferDesc.mappedAtCreation = false;
		vertexBuffer = device.createBuffer(bufferDesc);

		// Upload geometry data to the buffer
		queue.writeBuffer(vertexBuffer, 0, vertexData.data(), bufferDesc.size);
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



		//
		program = wgfx::loadProgram(shaderSource);
		buffer.setAttribute(0, 0);
		buffer.setAttribute(1, 2);
		program.setVertexBuffer(buffer);

		initbuffers();
	}


	void loop()
	{
		// Get the next target texture view
		targetView = getNextSurfaceTextureView();
		if (!targetView) return;

		// Create a command encoder for the draw call
		CommandEncoderDescriptor encoderDesc = {};
		encoderDesc.label = "My command encoder";
		encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc);

		// Create the render pass that clears the screen with our color
		RenderPassDescriptor renderPassDesc = {};

		// The attachment part of the render pass descriptor describes the target texture of the pass
		RenderPassColorAttachment renderPassColorAttachment = {};
		renderPassColorAttachment.view = targetView;
		renderPassColorAttachment.resolveTarget = nullptr;
		renderPassColorAttachment.loadOp = LoadOp::Clear;
		renderPassColorAttachment.storeOp = StoreOp::Store;
		renderPassColorAttachment.clearValue = WGPUColor{ 0.9, 0.9, 0.2, 1.0 };
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
		renderPass.setVertexBuffer(0, vertexBuffer, 0, vertexBuffer.getSize());

		renderPass.draw(vertexCount, 1, 0, 0);


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
}