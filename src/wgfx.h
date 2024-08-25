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

namespace wgfx
{
	RenderPassEncoder renderPass = nullptr;

	CommandEncoder encoder = nullptr;
	TextureView targetView = nullptr;

	//SDL_Window* window = nullptr;
	Device device = nullptr;
	Queue queue = nullptr;
	Surface surface = nullptr;
	std::unique_ptr<ErrorCallback> uncapturedErrorCallbackHandle;
	TextureFormat surfaceFormat = TextureFormat::Undefined;

	Instance instance = nullptr;

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
		config.format = TextureFormat::BGRA8Unorm; //surfaceFormat

		// And we do not need any particular view format:
		config.viewFormatCount = 0;
		config.viewFormats = nullptr;
		config.device = device;
		config.presentMode = PresentMode::Fifo;
		config.alphaMode = CompositeAlphaMode::Auto;
		
		surface.configure(config);

		// Release the adapter only after it has been fully utilized
		adapter.release();
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

		// pipelines and all sorts of things << frankely. we might state different elements but within the grounds of some name statement like 
		//wgfx::Pipeline pipeline;

		//i am thinking to allow for asynchronous processes;

		//so
		// 
		// a "shader" is going to be a pipeline which can be initialized anywhere.

		// wgfx::Pipeline pipeline;

		// then you can call functions like pipeline.fragment = shader. // wanna make it simple.

		// takes in what then? data naturally. let the people use a predefined loadShader function.

		//wgfx::Vertex vertex = wgfx::loadShader("vs.wgsl");
		//wgfx::Fragment fragment = wgfx::loadShader("fs.wgsl");
		// we allow for a call of wgfx::createShader(vertex, fragment);

		//attribs like any -- note attributes like in bgfx must be predefined through a vertex layout, 
				// then we may define them by wgfx::setVertexBuffer(0, vbo, 0, size); or wgfx::setIndexBuffer(ibo, .... sorry this is wrong.. 
		// but we must predefine;

		// rightly so they all net vbos and ibos, the difference comes from the notion of vertex attributes like bgfx as i said must have a VertexLayout. we ought define that more clearly than bgfx
		// bgfx's notation for vertex layouts is obtuse. i think it can be simplified. a little differently though is that i believe we must define the uniforms before hand as well. which is fine, that seems normal.

		//so we might have an object, although they all ought to sit under a subclass

		//wgfx::Layout layout;

		//layout.setAttribute(0, wgfx::Type::vec3);

		// IN THE EXAMPLE I LOOK AT uTime is the ONLY UNIFORM, the rest are ATTRIBUTES WHICH RELATE TO THE VERTICES.

		//POSITION AND COLOR ATTRIBUTES

		// then uniforms like
		//wgfx::





		// so you really only state to begin with where you want to have the attributes so like
		//wgfx::Layout layout;
		//layout.setAttribute(0, vec3); // pos
		//layout.setAttribute(1, vec2); // color

		//THEN IN THE LOOP WE THROW  // i want to ignore uniforms for now and focus on sending vbos ibos and vertex attribs

		// i am thinking for VERTEXATTRIBUTES
		// two layouts exist, naturally the  data is defined individually in the construction of the pipeline but the sending of the data is up to us;
		// in bgfx only interleaved data structure is available. i wonder about the possibility of multiple buffers.
		// how to make that intuitive.

		// layout.setAttribute(0, vec3);
		// layout.setAttribute(1, vec2);
		
		// now we cant just allow sending multiple for creating vbos, we must define the process clearly.
		// in webgpu you do properlly say setVertexBuffer(0, posbuffer, , size .... and
		//								  setVBO(		  1, colorbuffer, size ... etc);

		//in bgfx this would not be proper practice but it makes perfect sense to set multiple vbos.
		// in this way attributes would be described by their own vectors. makes sense logically but the pipeline must prepare for this the logical option is to make a choice before the pipeline is described
		// before the attributes are defined. you see because if we want to use two buffers we must have already created them. this makes neccessary the definition of another bufferDesc.

		// so, although i want to allow for this because i think it is quite interesting, i want to make sure that the platform for describing thise buffers makes sense and is not verbose.

		// naturally this other buffer must be accessed in the main loop.

		// it is interesting because multiple buffers is less like a vertex attribute and more like -- well another buffer.

		// in bgfx you say // x, y, z, u, v; first three are position and next two are texcoords, and then of course this is interleaving. and then
		// in the loop you state setVertexBuffer(the data) but this is sometimes counterintuitive and confusing. instead you might wish to say

		// firstly a similar but different declaration of attributes
		// pipeline.setAttrib(0, vec3)
		// pipeline.setAttrib(1, vec2) .. etc;
		
		//		intermission .. .. .. .. .. i think the thing to realize is that any buffer represents just a store of data.  - the statement of any vertexattribute represents a declaration of where in the code there
		// is going to be a representation of some amount of data

		// in any case it seems to be that for either situation you need to declare your attributes to the vertexBufferLayout(s) and further that to the pipeline.

		// in the case whereby you wish to have multiple buffers instead of interleaved buffers the change** is that you need to null the byte offset for the proceeding attribs and state the byte stride. whereas
		// if you have interleaved attributes its the opposite, you state the byte offset in the vertex buffer but leave byte stride at something different. the byte offset represent the offset in the vbo. whereas the byte stride 
		// represents it's "length".
		
		// further if you havem multiple you need to declare another buffer to store the data.

		// so how can i make that intuitive??

		//i can automate the process but need some declaration.

		//in bgfx you don't really describe a buffer. it is done behind the scenes. prehaps the trick might be decreasing the abstraction and allowing for the creation of buffers. ,, maybe not

		// maybe only a tag like

		// well. let me test

		// wgfx::Buffer position;
		// wgfx::Buffer color;
		// you see you don't even describe the "length" of each vertex's data here. you just say (float) as the type.
		// this makes me think that it might make more sense to define behind the scenes.

		// what would be helpful in my engine?

		// maybe instead of a blunt bgfx::setVertexBuffer(data)

		// to properly have a kind of wgfx::Buffer position;

		// position.setVertexBuffer(data); so that multiple can occur?

		// but to still have the definition of wgfx::Layout layout;

		// the definition of a layout is important so that we can have the description of the attributes, we need the length of the data so that we can calculate the stride.(and the type)

		// so maybe we just think about general abstraction to the current formulation of the webgpu process

		// Layout layout;
		// Buffer position;
		// Buffer color;
		// layout.setAttribute(0, vec3);
		// layout.setAttribute(1, vec2);
		// position.setVertexBuffer(data);
		// color.setVertexBuffer(data);

		// or maybe
		// Layout layout;
		// Buffer vertex;
		// layout.setAttribute(0, vec3);
		// layout.setAttribute(1, vec2);
		// vertex.setVertexBuffer(data);

// maybe;

		
		




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
}