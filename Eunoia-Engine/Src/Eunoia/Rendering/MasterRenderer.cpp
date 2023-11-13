#include "MasterRenderer.h"
#include "../Core/Engine.h"

namespace Eunoia {

	MasterRenderer::MasterRenderer(RenderContext* rc, Display* display) :
		m_Renderer2D(rc, display),
		m_Renderer3D(rc, display)
	{}

	MasterRenderer::~MasterRenderer()
	{
	}

	void Eunoia::MasterRenderer::Init()
	{
		m_Output2D = m_Renderer2D.Init();
		m_Output3D = m_Renderer3D.Init(LIGHTING_MODEL_PBR);

		RenderContext* rc = Engine::GetRenderContext();

		RenderPass renderPass;
		Framebuffer* framebuffer = &renderPass.framebuffer;
		framebuffer->numAttachments = 1;
		framebuffer->attachments[0].isClearAttachment = false;
		framebuffer->attachments[0].isStoreAttachment = true;
		framebuffer->attachments[0].nonClearAttachmentPreserve = false;
		framebuffer->attachments[0].isSubpassInputAttachment = false;
		framebuffer->attachments[0].memoryTransferSrc = false;

		if (Engine::IsEditorAttached())
		{
			framebuffer->useSwapchainSize = false;
			framebuffer->width = 50;
			framebuffer->height = 75;
			framebuffer->attachments[0].format = TEXTURE_FORMAT_RGBA8_UNORM;
			framebuffer->attachments[0].isSamplerAttachment = true;
			framebuffer->attachments[0].isSwapchainAttachment = false;
		}
		else
		{
			framebuffer->useSwapchainSize = true;
			framebuffer->attachments[0].format = TEXTURE_FORMAT_SWAPCHAIN_FORMAT;
			framebuffer->attachments[0].isSamplerAttachment = false;
			framebuffer->attachments[0].isSwapchainAttachment = true;
		}

		Subpass subpass;
		subpass.depthStencilAttachment = 0;
		subpass.useDepthStencilAttachment = false;
		subpass.numReadAttachments = 0;
		subpass.numWriteAttachments = 1;
		subpass.writeAttachments[0] = 0;
		
		ShaderID shader = rc->LoadShader("Final");

		GraphicsPipeline pipeline {};
		pipeline.shader = shader;
		pipeline.topology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipeline.viewportState.useFramebufferSizeForScissor = true;
		pipeline.viewportState.useFramebufferSizeForViewport = true;
		pipeline.viewportState.scissor.x =
		pipeline.viewportState.scissor.y =
		pipeline.viewportState.viewport.x =
		pipeline.viewportState.viewport.y = 0;
		pipeline.rasterizationState.cullMode = CULL_MODE_BACK;
		pipeline.rasterizationState.depthClampEnabled = false;
		pipeline.rasterizationState.discard = false;
		pipeline.rasterizationState.frontFace = FRONT_FACE_CW;
		pipeline.rasterizationState.polygonMode = POLYGON_MODE_FILL;
		pipeline.vertexInputState.numAttributes = 1;
		pipeline.vertexInputState.vertexSize = EU_VERTEX_SIZE_AUTO;
		pipeline.vertexInputState.attributes[0].name = "POSITION";
		pipeline.vertexInputState.attributes[0].type = VERTEX_ATTRIBUTE_FLOAT2;
		pipeline.vertexInputState.attributes[0].location = 0;
		pipeline.numBlendStates = 1;
		pipeline.blendStates[0].blendEnabled = true;
		pipeline.blendStates[0].alpha.dstFactor = BLEND_FACTOR_ZERO;
		pipeline.blendStates[0].alpha.srcFactor = BLEND_FACTOR_ONE;
		pipeline.blendStates[0].alpha.operation = BLEND_OPERATION_ADD;
		pipeline.blendStates[0].color.dstFactor = BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		pipeline.blendStates[0].color.operation = BLEND_OPERATION_ADD;
		pipeline.blendStates[0].color.srcFactor = BLEND_FACTOR_ONE;
		pipeline.depthStencilState.depthCompare = COMPARE_OPERATION_LESS;
		pipeline.depthStencilState.depthTestEnabled = false;
		pipeline.depthStencilState.depthWriteEnabled = false;
		pipeline.depthStencilState.stencilTestEnabled = false;

		MaxTextureGroupBinds maxBinds;
		maxBinds.set = 0;
		maxBinds.maxBinds = 2;

		pipeline.maxTextureGroupBinds.Push(maxBinds);

		subpass.pipelines.Push(pipeline);
		renderPass.subpasses.Push(subpass);

		m_RenderPass = rc->CreateRenderPass(renderPass);
		std::cout << m_RenderPass << std::endl;

	if(Engine::IsEditorAttached())
		m_FinalOutput = rc->CreateTextureHandleForFramebufferAttachment(m_RenderPass, 0);
	else
		m_FinalOutput = EU_INVALID_TEXTURE_ID;


		v2 vertices[4] = { v2(-1.0f, 1.0f), v2(-1.0f, -1.0f), v2(1.0f, -1.0f), v2(1.0f, 1.0f) };
		u16 indices[6] = { 0, 1, 2, 0, 2, 3 };
		m_DrawQuad.vertexBuffer = rc->CreateBuffer(BUFFER_TYPE_VERTEX, BUFFER_USAGE_STATIC, vertices, sizeof(v2) * 4);
		m_DrawQuad.indexBuffer = rc->CreateBuffer(BUFFER_TYPE_INDEX, BUFFER_USAGE_STATIC, indices, sizeof(u16) * 6);
		m_DrawQuad.count = 6;
		m_DrawQuad.indexOffset = 0;
		m_DrawQuad.indexType = INDEX_TYPE_U16;
		m_DrawQuad.vertexOffset = 0;
	}

	void MasterRenderer::BeginFrame()
	{
		m_Renderer2D.BeginFrame();
		m_Renderer3D.BeginFrame();
	}

	void MasterRenderer::EndFrame()
	{
		m_Renderer2D.EndFrame();
		m_Renderer3D.EndFrame();
	}

	void MasterRenderer::RenderFrame()
	{
		m_Renderer2D.RenderFrame();
		m_Renderer3D.RenderFrame();

		RenderContext* rc = Engine::GetRenderContext();
		
		RenderPassBeginInfo beginInfo;
		beginInfo.initialPipeline = 0;
		beginInfo.renderPass = m_RenderPass;
		beginInfo.numClearValues = 1;
		beginInfo.clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		beginInfo.clearValues[0].clearDepthStencil = false;

		rc->BeginRenderPass(beginInfo);

		TextureGroupBind bind;
		bind.set = 0;
		bind.numTextureBinds = 1;
		bind.binds[0].binding = 0;
		bind.binds[0].sampler = EU_SAMPLER_NEAREST_CLAMP_TO_EDGE;
		bind.binds[0].texture[0] = m_Output3D;
		bind.binds[0].textureArrayLength = 1;

		rc->BindTextureGroup(bind);
		rc->SubmitRenderCommand(m_DrawQuad);

		bind.binds[0].texture[0] = m_Output2D;
		rc->BindTextureGroup(bind);
		rc->SubmitRenderCommand(m_DrawQuad);

		rc->EndRenderPass();
	}

	Renderer2D* MasterRenderer::GetRenderer2D()
	{
		return &m_Renderer2D;
	}

	Renderer3D* MasterRenderer::GetRenderer3D()
	{
		return &m_Renderer3D;
	}

	TextureID MasterRenderer::GetFinalOutput()
	{
		return m_FinalOutput;
	}

	void MasterRenderer::GetOutputSize(u32* width, u32* height)
	{
		Engine::GetRenderContext()->GetFramebufferSize(m_RenderPass, width, height);
	}

	void MasterRenderer::ResizeOutput(u32 width, u32 height)
	{
		m_Renderer2D.ResizeOutput(width, height);
		m_Renderer3D.ResizeOutput(width, height);
		Engine::GetRenderContext()->ResizeFramebuffer(m_RenderPass, width, height);
	}

}
