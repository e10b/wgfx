#include "pipeline.h"

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

		// little check
			//std::cout << "vbos count: " << vbos.vertexBuffers.size() << "\n";

	}
	VertexBufferLayout vertexBufferLayout;
	void Pipeline::setIndexBuffer(const std::vector<uint16_t>& value)
	{
		for (auto* ibo : ibos.indexBuffers)
		{
			if (!ibo->is32Bit && ibo->data16 == value)
			{
				// set current
				ibos.current = ibo;
				return;
			}
		}

		IndexBuffer* ibo = createIndexBuffer(value);
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

		ColorTargetState colorTarget[10];
		//if (multiTarget)
		//{
		colorTarget[0].format = surfaceFormat;
		colorTarget[0].blend = &blendState;
		colorTarget[0].writeMask = ColorWriteMask::All;

		colorTarget[1].format = surfaceFormat;
		colorTarget[1].blend = &blendState;
		colorTarget[1].writeMask = ColorWriteMask::All;

		colorTarget[2].format = surfaceFormat;
		colorTarget[2].blend = &blendState;
		colorTarget[2].writeMask = ColorWriteMask::All;

		colorTarget[3].format = surfaceFormat;
		colorTarget[3].blend = &blendState;
		colorTarget[3].writeMask = ColorWriteMask::All;
		// ideally we product these based on the preset targets.... << ye
		// ideally we product these based on the preset targets.... << ye
		// ideally we product these based on the preset targets.... << ye
		// ideally we product these based on the preset targets.... << ye
		// ideally we product these based on the preset targets.... << ye



		//fragmentState.targetCount = 1; // two targets
		//fragmentState.targets = colorTarget;
		//}
		//else
		//{
			//ColorTargetState colorTarget;
			//colorTarget[0].format = surfaceFormat;
			//colorTarget[0].blend = &blendState;
			//colorTarget[0].writeMask = ColorWriteMask::All;

		//}
		if (targets != 0)
		{
			fragmentState.targetCount = targets; // two targets
			fragmentState.targets = colorTarget;
		} 
		else
		{
			fragmentState.targetCount = 0; // two targets
			fragmentState.targets = nullptr; // two targets
		}

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