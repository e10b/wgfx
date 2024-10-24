#include "pipeline.h"

namespace wgfx
{
	Pipeline::Pipeline()
	{
		std::cout << "Creating render pipeline..." << std::endl;
		pipelineDesc = RenderPipelineDescriptor();
	}

	void Pipeline::setIndexBuffer(IndexBuffer buffer)
	{
		indexBuffer = buffer;
	}
	VertexBufferLayout vertexBufferLayout;
	void Pipeline::setVertexBuffer(VertexBuffer buffer) // take in a vbo? yuh, yuh? well what exactly is a vbo? vertexbufferhandle, it is an object which allows attribs
	{
		initDepth();
		vertexBuffer = buffer;
	}
	void Pipeline::init()
	{
		vertexBufferLayout.attributeCount = (uint32_t)vertexBuffer.vertexAttribs.size();
		vertexBufferLayout.attributes = vertexBuffer.vertexAttribs.data();
		vertexBufferLayout.arrayStride = (vertexBuffer.fields) * sizeof(float);
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
		bindGroupDesc.entryCount = (uint32_t)bindings.size();
		bindGroupDesc.entries = bindings.data(); // Pass the array of entries
		bindGroup = device.createBindGroup(bindGroupDesc);

	}
	void Pipeline::setUniform(Uniform* uniform, bool dynamic)
	{
		BindGroupLayoutEntry bindingLayout = Default;							/// layout needs to be created in joint with the actual entry
		// The binding index as used in the @binding attribute in the shader
		bindingLayout.binding = uniform->index;
		// The stage that needs to access this resource
		bindingLayout.visibility = ShaderStage::Vertex | ShaderStage::Fragment;
		bindingLayout.buffer.type = BufferBindingType::Uniform;
		bindingLayout.buffer.minBindingSize = uniform->scale;
		if (dynamic)
		{
			bindingLayout.buffer.hasDynamicOffset = true; // DYNAMIC
			dynamicUniformCount++;
			dynamicOffsets.push_back(0); // the 
		}
		uniforms.push_back(uniform);
		entries.push_back(bindingLayout);
		bindings.push_back(uniform->binding);
	}
	void Pipeline::setTexture(Uniform* uniform)
	{
		BindGroupLayoutEntry bindingLayout = Default;							/// layout needs to be created in joint with the actual entry
		bindingLayout.binding = uniform->index;
		bindingLayout.visibility = ShaderStage::Fragment;
		bindingLayout.texture.sampleType = TextureSampleType::Float;
		bindingLayout.texture.viewDimension = TextureViewDimension::_2D;

		uniforms.push_back(uniform);
		entries.push_back(bindingLayout);
		bindings.push_back(uniform->binding);
	}

	void Pipeline::setSampler(Uniform* uniform)
	{
		// The texture sampler binding
		BindGroupLayoutEntry samplerBindingLayout = Default;
		samplerBindingLayout.binding = uniform->index;
		samplerBindingLayout.visibility = ShaderStage::Fragment;
		samplerBindingLayout.sampler.type = SamplerBindingType::Filtering;


		uniforms.push_back(uniform);
		entries.push_back(samplerBindingLayout);
		bindings.push_back(uniform->binding);
	}
	void Pipeline::touch()
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



	void Pipeline::updateUniform(Uniform* uniform, const float* array)
	{
		uint32_t dynamicOffset = uniforms.at(uniform->index)->quantity * uniforms.at(uniform->index)->stride;

		if (dynamicOffsets.size() <= uniform->index) { dynamicOffsets.resize(uniform->index + 1); } // resize
		dynamicOffsets.at(uniform->index) = (dynamicOffset); // propogate current offsets

		queue.writeBuffer(uniforms.at(uniform->index)->buffer, dynamicOffset, array, uniform->scale);
		uniforms.at(uniform->index)->quantity++;

	} // problem area has to be << 

	Pipeline loadPipeline(std::string source)
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

		Pipeline pipeline;
		pipeline.shaderModule = shaderModule;
		//programs.push_back(&program);
		pipelines.push_back(&pipeline);  // Move to avoid copying

		return pipeline;
	}
}