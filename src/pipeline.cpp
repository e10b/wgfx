#include "pipeline.h"

#include <iostream>

namespace wgfx
{
	Pipeline::Pipeline()
	{
		std::cout << "Creating render pipeline..." << std::endl;
		pipelineDesc = RenderPipelineDescriptor();
	}
	bool first = false;

	/*
	
	what we need to do is not send data. we need to create a vbo handle and send that,
	in that way, we update it instead of recreating it.

	it is generated then sent

	*/

	void Pipeline::setVertexBuffer(const std::vector<float>& value)
	{
		for (auto* vbo : vbos.vertexBuffers)
		{
			if (vbo->data == value)
			{
				// set current
				vbos.current = vbo;
				return;
			}
		}
			//VertexBuffer* vbo = createVertexBuffer(value);
			//vbos.add(vbo);
			//vbos.current = vbo;
		//if (vbos.current)
		//{
		//	vbos.current->write(value); // Update the existing buffer's data
		//}
		//else
		//{
			// No buffer exists, so create a new one
			VertexBuffer* newVBO = createVertexBuffer(value);
			vbos.add(newVBO);
			vbos.current = newVBO;
		//}

	}

	void Pipeline::setIndexBuffer(const std::vector<uint16_t>& value)
	{
		for (auto* ibo : ibos.indexBuffers)
		{
			if (!ibo->is32Bit && ibo->data == value)
			{
				// set current
				ibos.current = ibo;
				return;
			}
		}

		IndexBuffer* ibo = createIndexBuffer(value);
		ibos.add(ibo);
		ibos.current = ibo;

		//IndexBuffer* ibo = ibos.get(value);
		//if (ibo)
		//{
		//	// Set current
		//	ibos.current = ibo;
		//	return;
		//}

		//// None exists, make a new
		//ibo = createIndexBuffer(value);
		//ibos.add(ibo);
		//ibos.current = ibo;

	}

	void Pipeline::setIndexBuffer(const std::vector<uint32_t>& value)
	{
		IndexBuffer* ibo = createIndexBufferU32(value);
		ibos.add(ibo);
		ibos.current = ibo;
	}

	void Pipeline::init(VertexBuffer* vertexBuffer)
	{
		samples = (multiSample) ? 4 : 1;

		//initDepth();
		vertexBufferLayout.attributeCount = (uint32_t)vertexBuffer->vertexAttribs.size();
		vertexBufferLayout.attributes = vertexBuffer->vertexAttribs.data();
		vertexBufferLayout.arrayStride = (vertexBuffer->fields) * sizeof(float);
		vertexBufferLayout.stepMode = VertexStepMode::Vertex;

		wgpu::MultisampleState multisampleState = {};
		multisampleState.count = samples;                  // Number of samples (e.g., 4x MSAA)
		multisampleState.mask = ~0;                 // Sample mask (default enables all samples)
		multisampleState.alphaToCoverageEnabled = false; // Optional: Enable alpha-to-coverage


		//pipelineDesc.multisample = multisampleState;

		pipelineDesc.vertex.bufferCount = 1;
		pipelineDesc.vertex.buffers = &vertexBufferLayout;

		pipelineDesc.vertex.module = shaderModule;
		pipelineDesc.vertex.entryPoint = "vs_main";
		pipelineDesc.vertex.constantCount = 0;
		pipelineDesc.vertex.constants = nullptr;

		pipelineDesc.primitive.topology = vertexBuffer->topology;
		pipelineDesc.primitive.stripIndexFormat = IndexFormat::Undefined;
		pipelineDesc.primitive.frontFace = FrontFace::CCW;
		pipelineDesc.primitive.cullMode = CullMode::Back; // backface culling option, currently thinking about ways to expose this to the mid level wgfx::setState(wgfx::CullBack);

		pipelineDesc.fragment = &fragmentState;
		fragmentState.module = shaderModule;
		fragmentState.entryPoint = "fs_main";
		fragmentState.constantCount = 0;
		fragmentState.constants = nullptr;

		fragmentBlend.color.srcFactor = BlendFactor::SrcAlpha;
		fragmentBlend.color.dstFactor = BlendFactor::OneMinusSrcAlpha;
		fragmentBlend.color.operation = BlendOperation::Add;
		fragmentBlend.alpha.srcFactor = BlendFactor::Zero;
		fragmentBlend.alpha.dstFactor = BlendFactor::One;
		fragmentBlend.alpha.operation = BlendOperation::Add;

		for (int ti = 0; ti < 10; ++ti) {
			colorTargets[ti].format = surfaceFormat;
			colorTargets[ti].blend = &fragmentBlend;
			colorTargets[ti].writeMask = ColorWriteMask::All;
		}

		// ideally we product these based on the preset targets.... << ye
		// ideally we product these based on the preset targets.... << ye
		// ideally we product these based on the preset targets.... << ye
		// ideally we product these based on the preset targets.... << ye
		// ideally we product these based on the preset targets.... << ye


		if (targets != 0)
		{
			fragmentState.targetCount = targets; // two targets
			fragmentState.targets = colorTargets;
		}
		else
		{
			fragmentState.targetCount = 0; // two targets
			fragmentState.targets = nullptr; // two targets
		}
		// We setup a depth buffer state for the render pipeline
		depthStencilState = Default;
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

		if (useDepth)
		{
			pipelineDesc.depthStencil = &depthStencilState; // keep in mind that the depthStencil is only when you want to create a depth for the pass.
			// don't need it for shadow pass or post processing pass in general. Unless you want a depth texture of a quad. etc.
		}
		else
		{
			pipelineDesc.depthStencil = nullptr;
		}

		pipelineDesc.multisample.count = samples;
		pipelineDesc.multisample.mask = ~0u;
		pipelineDesc.multisample.alphaToCoverageEnabled = false;

		// Create a bind group layout
		//bindGroupLayoutDesc;
			//bindGroupLayoutDesc.entryCount = entries.size(); // uh
			//bindGroupLayoutDesc.entries = entries.data();
			//bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDesc);
		uniforms.touch();
		// uniform groups
		/*
						// A bind group contains one or multiple bindings
						BindGroupDescriptor bindGroupDesc;
						bindGroupDesc.layout = bindGroupLayout;
						bindGroupDesc.entryCount = (uint32_t)bindings.size();
						bindGroupDesc.entries = bindings.data(); // Pass the array of entries
						bindGroup = device.createBindGroup(bindGroupDesc);
		*/


		// Create the pipeline layout
		PipelineLayoutDescriptor layoutDesc;
		layoutDesc.bindGroupLayoutCount = 1;
		//layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&bindGroupLayout;
		layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&uniforms.bindGroupLayout;
		PipelineLayout layout = device.createPipelineLayout(layoutDesc);
		pipelineDesc.layout = layout;

		pipeline = device.createRenderPipeline(pipelineDesc);
		std::cout << "Render pipeline: " << pipeline << std::endl;
		shaderModule.release();

		/*
			test moving the prepare operation after the pipeline creation;
		*/




	}
	

	Pipeline* loadPipeline(std::string source)
	{
		if (source.empty()) {
			std::cerr << "wgfx::loadPipeline: empty shader source (WGSL file missing or unreadable)\n";
		}
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

		Pipeline* pipeline = new Pipeline();
		pipeline->shaderModule = shaderModule;
		//programs.push_back(&program);
		pipelines.push_back(pipeline);  // Move to avoid copying

		return pipeline;
	}
}