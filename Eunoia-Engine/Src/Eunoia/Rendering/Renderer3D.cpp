#include "Renderer3D.h"

#include "../Core/Engine.h"
#include "Asset/AssetManager.h"
#include "../DataStructures/Map.h"
#include "Asset/ModelLoader.h"

namespace Eunoia {

	struct GBufferPerFrameBuffer
	{
		m4 viewProjection;
		v3 ambient;
		r32 p0;
		v3 camPos;
		r32 p1;
	};

	struct GBufferMaterialModifierBuffer
	{
		v3 albedo;
		r32 p0;
		r32 texCoordScale;
		r32 ao;
		r32 dispScale;
		r32 dispOffset;
		r32 specularOrMetallic;
		r32 glossOrRoughness;
	};

	struct LightPerFrameBuffer
	{
		v3 camPos;
		r32 p0;
	};

	static void DisplayResizeCallback(const DisplayEvent& e, void* userPtr)
	{
		Renderer3D* renderer = (Renderer3D*)userPtr;
		RenderContext* rc = renderer->m_RenderContext;

		if (e.type == DISPLAY_EVENT_RESIZE)
		{
			rc->ResizeFramebuffer(renderer->m_DeferredPass, e.width, e.height);
			rc->ResizeFramebuffer(renderer->m_GaussIter1RenderPass, e.width, e.height);
			rc->ResizeFramebuffer(renderer->m_GaussIter2RenderPass, e.width, e.height);
			rc->ResizeFramebuffer(renderer->m_FinalRenderPass, e.width, e.height);
		//	rc->RecreateBuffer(albedoTexturePixels, BUFFER_TYPE_MEMORY_TRANSFORM_DST, BUFFER_USAGE_DYNAMIC, 0, e.width * e.height * 4 * sizeof(u16));
		}
	}

	void Renderer3D::InitShadowMapPass(u32 resolution)
	{
		
	}

	Renderer3D::Renderer3D(RenderContext* renderContext, Display* display) :
		m_RenderContext(renderContext),
		m_Display(display)
	{}

	Renderer3D::~Renderer3D()
	{
	}

	TextureID Eunoia::Renderer3D::Init(LightingModel lightingModel)
	{
#ifdef EU_DIST
		m_Display->AddDisplayEventCallback(DisplayResizeCallback, this);
#endif

		Sampler sampler;
		sampler.minFilter = TEXTURE_FILTER_NEAREST;
		sampler.magFilter = TEXTURE_FILTER_NEAREST;
		sampler.addressModes[0] = TEXTURE_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler.addressModes[1] = TEXTURE_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler.addressModes[2] = TEXTURE_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler.anisotropyEnabled = false;
		sampler.maxAnisotropy = 0.0f;
		sampler.borderColor = TEXTURE_BORDER_COLOR_BLACK_INT;
		sampler.normalizedCoords = true;

		m_NearestSampler = m_RenderContext->CreateSampler(sampler);
		sampler.minFilter = sampler.magFilter = TEXTURE_FILTER_LINEAR;
		m_LinearSampler = m_RenderContext->CreateSampler(sampler);
		sampler.minFilter = sampler.magFilter = TEXTURE_FILTER_NEAREST;
		sampler.addressModes[0] = sampler.addressModes[1] = sampler.addressModes[2] = TEXTURE_ADDRESS_MODE_CLAMP_TO_BORDER;
		sampler.borderColor = TEXTURE_BORDER_COLOR_WHITE_FLOAT;
		m_ShadowMapSampler = m_RenderContext->CreateSampler(sampler);

		m_LightingModel = lightingModel;
		m_Ambient = v3(0.1f, 0.1f, 0.1f);
		m_BloomThreshold = 1.0f;
		m_BloomBlurIterationCount = 4;
		m_CamPos = v3(0.0f, 0.0f, 0.0f);
		m_ViewProjection = m4::CreateIdentity();
		m_WireframeColor = v3(1.0f, 1.0f, 0.0);

		InitDeferredRenderPass(lightingModel);
		InitGuassianBlurRenderPass();
		InitFinalRenderPass();

		m_GBufferPerFrameBuffer = m_RenderContext->CreateShaderBuffer(SHADER_BUFFER_UNIFORM_BUFFER, sizeof(GBufferPerFrameBuffer), 1);
		m_GBufferPerInstanceBuffer = m_RenderContext->CreateShaderBuffer(SHADER_BUFFER_UNIFORM_BUFFER, sizeof(GBufferPerInstanceBuffer), EU_RENDERER3D_MAX_SUBMITIONS_PER_RENDERPASS);
		m_GBufferBoneBuffer = m_RenderContext->CreateShaderBuffer(SHADER_BUFFER_STORAGE_BUFFER, sizeof(m4) * EU_RENDERER3D_MAX_BONES, EU_RENDERER3D_MAX_SUBMITIONS_PER_RENDERPASS);
		m_GBufferMaterialModifierBuffer = m_RenderContext->CreateShaderBuffer(SHADER_BUFFER_UNIFORM_BUFFER, sizeof(GBufferMaterialModifierBuffer), EU_RENDERER3D_MAX_SUBMITIONS_PER_RENDERPASS);

		m_LightPerFrameBuffer = m_RenderContext->CreateShaderBuffer(SHADER_BUFFER_UNIFORM_BUFFER, sizeof(LightPerFrameBuffer), 1);
		m_DLightLightBuffer = m_RenderContext->CreateShaderBuffer(SHADER_BUFFER_UNIFORM_BUFFER, sizeof(DirectionalLight), EU_RENDERER3D_MAX_DIRECTIONAL_LIGHTS);

		m_PLightMVPBuffer = m_RenderContext->CreateShaderBuffer(SHADER_BUFFER_UNIFORM_BUFFER, sizeof(m4), EU_RENDERER3D_MAX_POINT_LIGHTS);
		m_PLightBuffer = m_RenderContext->CreateShaderBuffer(SHADER_BUFFER_UNIFORM_BUFFER, sizeof(PointLight), EU_RENDERER3D_MAX_POINT_LIGHTS);

		m_WireframePerInstanceBuffer = m_RenderContext->CreateShaderBuffer(SHADER_BUFFER_UNIFORM_BUFFER, sizeof(m4), EU_RENDERER3D_MAX_WIREFRAME_SUBMITIONS_PER_RENDERPASS);
		m_WireframeBuffer = m_RenderContext->CreateShaderBuffer(SHADER_BUFFER_UNIFORM_BUFFER, sizeof(v4), 1);

		m_BloomThresholdBuffer = m_RenderContext->CreateShaderBuffer(SHADER_BUFFER_UNIFORM_BUFFER, sizeof(r32), 1);

		m_RenderContext->AttachShaderBufferToRenderPass(m_DeferredPass, m_GBufferPerFrameBuffer, 0, 0, 0, 0);
		m_RenderContext->AttachShaderBufferToRenderPass(m_DeferredPass, m_GBufferPerInstanceBuffer, 0, 0, 1, 0);
		m_RenderContext->AttachShaderBufferToRenderPass(m_DeferredPass, m_GBufferBoneBuffer, 0, 0, 1, 1);
		m_RenderContext->AttachShaderBufferToRenderPass(m_DeferredPass, m_GBufferMaterialModifierBuffer, 0, 0, 3, 0);

		m_RenderContext->AttachShaderBufferToRenderPass(m_DeferredPass, m_LightPerFrameBuffer, 1, 0, 1, 0);
		m_RenderContext->AttachShaderBufferToRenderPass(m_DeferredPass, m_DLightLightBuffer, 1, 0, 2, 0);

		m_RenderContext->AttachShaderBufferToRenderPass(m_DeferredPass, m_LightPerFrameBuffer, 1, 1, 1, 0);
		m_RenderContext->AttachShaderBufferToRenderPass(m_DeferredPass, m_PLightMVPBuffer, 1, 1, 2, 0);
		m_RenderContext->AttachShaderBufferToRenderPass(m_DeferredPass, m_PLightBuffer, 1, 1, 3, 0);
		m_RenderContext->AttachShaderBufferToRenderPass(m_DeferredPass, m_PLightMVPBuffer, 1, 2, 1, 0);

		m_RenderContext->AttachShaderBufferToRenderPass(m_DeferredPass, m_GBufferPerFrameBuffer, 2, 0, 0, 0);
		m_RenderContext->AttachShaderBufferToRenderPass(m_DeferredPass, m_WireframePerInstanceBuffer, 2, 0, 1, 0);
		m_RenderContext->AttachShaderBufferToRenderPass(m_DeferredPass, m_WireframeBuffer, 2, 0, 2, 0);

		m_RenderContext->AttachShaderBufferToRenderPass(m_DeferredPass, m_BloomThresholdBuffer, 3, 0, 1, 0);

		v2 vertices[4] = { v2(-1.0f, 1.0f), v2(-1.0f, -1.0f), v2(1.0f, -1.0f), v2(1.0f, 1.0f) };
		u16 indices[6] = { 0, 1, 2, 0, 2, 3 };
		m_DrawQuad.vertexBuffer = m_RenderContext->CreateBuffer(BUFFER_TYPE_VERTEX, BUFFER_USAGE_STATIC, vertices, sizeof(v2) * 4);
		m_DrawQuad.indexBuffer = m_RenderContext->CreateBuffer(BUFFER_TYPE_INDEX, BUFFER_USAGE_STATIC, indices, sizeof(u16) * 6);
		m_DrawQuad.count = 6;
		m_DrawQuad.indexOffset = 0;
		m_DrawQuad.indexType = INDEX_TYPE_U16;
		m_DrawQuad.vertexOffset = 0;

		LoadedModel sphereData;
		EumdlLoadError error = ModelLoader::LoadEumdlModel("Res/Models/BoundingSphere.eumdl", &sphereData);

		List<v3> sphereVertices(sphereData.vertices.Size(), sphereData.vertices.Size());
		for (u32 i = 0; i < sphereData.vertices.Size(); i++)
			sphereVertices[i] = sphereData.vertices[i].pos;

		m_DrawBoundingSphere.count = sphereData.indices.Size();
		m_DrawBoundingSphere.vertexOffset = 0;
		m_DrawBoundingSphere.indexOffset = 0;
		m_DrawBoundingSphere.indexType = INDEX_TYPE_U32;
		m_DrawBoundingSphere.vertexBuffer = m_RenderContext->CreateBuffer(BUFFER_TYPE_VERTEX, BUFFER_USAGE_STATIC, &sphereVertices[0], sizeof(v3) * sphereVertices.Size());
		m_DrawBoundingSphere.indexBuffer = m_RenderContext->CreateBuffer(BUFFER_TYPE_INDEX, BUFFER_USAGE_STATIC, &sphereData.indices[0], sizeof(u32) * sphereData.indices.Size());

		u32 aWidth, aHeight;
		m_RenderContext->GetFramebufferSize(m_DeferredPass, &aWidth, &aHeight);
		m_AlbedoTexturePixels = m_RenderContext->CreateBuffer(BUFFER_TYPE_MEMORY_TRANSFER_DST, BUFFER_USAGE_DYNAMIC, 0, aWidth * aHeight * 4 * sizeof(u16));

		return m_Textures.outputTexture;
	}

	void Renderer3D::SetCamera(const m4& view, const m4& projection, const v3& pos, const quat& rot)
	{
		m_View = view;
		m_Projection = projection;
		m_ViewProjection = projection * view;
		m_CamPos = pos;
		m_CamRot = rot;
	}

	void Renderer3D::SetAmbient(const v3& ambient)
	{
		m_Ambient = ambient;
	}

	void Renderer3D::SetBloomThreshold(r32 threshold)
	{
		m_BloomThreshold = threshold;
	}

	void Renderer3D::SetBloomBlurIterationCount(u32 iterationCount)
	{
		m_BloomBlurIterationCount = iterationCount;
	}

	void Renderer3D::SetWireframeColor(const v3& color)
	{
		m_WireframeColor = color;
	}

	static r32 CalcPointLightSphereScale(const PointLight& light)
	{
		r32 maxChannel = EU_MAX(EU_MAX(light.color.x, light.color.y), light.color.z);

		r32 a = light.attenuation.quadratic;
		r32 b = light.attenuation.linear;
		r32 c = light.attenuation.constant - 256.0f * light.color.w * maxChannel;

		r32 s = (-b + sqrtf(b * b - 4 * a * c)) / (2 * a);

		return (-b + sqrtf(b * b - 4 * a * c)) / (2 * a);
	}

	void Renderer3D::BeginFrame()
	{
		m_Renderables.Clear();
		m_WireframeRenderables.Clear();
		m_DLights.Clear();
		m_PLights.Clear();
	}

	void Renderer3D::SubmitModel(const Model& model, const m4& transform, m4* boneTransforms, u32 numBoneTransforms, b32 animated, EntityID entity)
	{
		SubmittedRenderable renderable;
		renderable.vertexBuffer = model.vertexBuffer;
		renderable.indexBuffer = model.indexBuffer;
		renderable.totalIndexCount = model.totalIndexCount;
		renderable.meshes = model.meshes;
		renderable.materials = model.materials;
		renderable.modifiers = model.modifiers;
		renderable.transform = transform;
		renderable.boneTransforms = boneTransforms;
		renderable.numBoneTransforms = numBoneTransforms;
		renderable.animated = animated;
		renderable.entityID = entity;

		m_Renderables.Push(renderable);
	}

	void Renderer3D::SubmitWireframeModel(const Model& model, const m4& transform)
	{
		SubmittedWireframeRenderable renderable;
		renderable.vertexBuffer = model.vertexBuffer;
		renderable.indexBuffer = model.indexBuffer;
		renderable.totalIndexCount = model.totalIndexCount;
		renderable.transform = transform;

		m_WireframeRenderables.Push(renderable);
	}

	void Renderer3D::SubmitLight(const Light3D& light)
	{
		if (light.type == LIGHT3D_DIRECTIONAL)
		{
			DirectionalLightSubmission dlight;
			dlight.shadowInfo = light.shadowInfo;
			dlight.light.color = light.colorAndIntensity;
			dlight.light.direction = light.direction;
			m_DLights.Push(dlight);
		}
		else if (light.type == LIGHT3D_POINT)
		{
			PointLight plight;
			plight.color = light.colorAndIntensity;
			plight.position = light.pos;
			plight.attenuation = light.attenuation;
			m_PLights.Push(plight);
		}
	}

	void Renderer3D::EndFrame()
	{
	}

	void Renderer3D::DoShadowMapPass()
	{
		
	}

	void Renderer3D::DoDeferredPass()
	{
		RenderPassBeginInfo begin;
		begin.initialPipeline = 0;
		begin.renderPass = m_DeferredPass;
		begin.numClearValues = 6;
		begin.clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		begin.clearValues[0].clearDepthStencil = false;
		begin.clearValues[1].depth = 1.0f;
		begin.clearValues[1].stencil = 0;
		begin.clearValues[1].clearDepthStencil = true;
		begin.clearValues[2].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		begin.clearValues[2].clearDepthStencil = false;
		begin.clearValues[3].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		begin.clearValues[3].clearDepthStencil = false;
		begin.clearValues[4].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		begin.clearValues[4].clearDepthStencil = false;
		begin.clearValues[5].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		begin.clearValues[5].clearDepthStencil = false;

		m_RenderContext->BeginRenderPass(begin);

		GBufferPerFrameBuffer perFrame;
		perFrame.viewProjection = m_ViewProjection;
		perFrame.ambient = m_Ambient;
		perFrame.camPos = m_CamPos;

		m_RenderContext->UpdateShaderBuffer(m_GBufferPerFrameBuffer, &perFrame, sizeof(GBufferPerFrameBuffer));
		RenderCommand renderMesh;
		renderMesh.indexType = INDEX_TYPE_U32;
		renderMesh.vertexOffset = 0;
		for (u32 i = 0; i < m_Renderables.Size(); i++)
		{
			const SubmittedRenderable& renderable = m_Renderables[i];
			m_GBufferPerInstanceBufferData.model = renderable.transform;
			m_GBufferPerInstanceBufferData.animated = renderable.animated;
			m_GBufferPerInstanceBufferData.entityID = renderable.entityID;
			m_RenderContext->UpdateShaderBuffer(m_GBufferPerInstanceBuffer, &m_GBufferPerInstanceBufferData, sizeof(GBufferPerInstanceBuffer));
			if (renderable.numBoneTransforms > 0)
				m_RenderContext->UpdateShaderBuffer(m_GBufferBoneBuffer, renderable.boneTransforms, sizeof(m4) * renderable.numBoneTransforms);

			renderMesh.vertexBuffer = renderable.vertexBuffer;
			renderMesh.indexBuffer = renderable.indexBuffer;
			for (u32 j = 0; j < renderable.meshes.Size(); j++)
			{
				const LoadedMesh& mesh = renderable.meshes[j];
				renderMesh.indexOffset = mesh.indexOffset;
				renderMesh.vertexOffset = mesh.vertexOffset;
				renderMesh.count = mesh.indexCount;
				
				BindMaterial(renderable.materials[mesh.materialIndex]);
				BindMaterialModifier(renderable.modifiers[mesh.materialModifierIndex]);
				m_RenderContext->SubmitRenderCommand(renderMesh);
			}
		}

		m_RenderContext->NextSubpass();

		m_RenderContext->UpdateShaderBuffer(m_LightPerFrameBuffer, &m_CamPos, sizeof(v3));
		for (u32 i = 0; i < m_DLights.Size(); i++)
		{
			m_RenderContext->UpdateShaderBuffer(m_DLightLightBuffer, &m_DLights[i].light, sizeof(DirectionalLight));
			m_RenderContext->SubmitRenderCommand(m_DrawQuad);
		}

		for (u32 i = 0; i < m_PLights.Size(); i++)
		{
			r32 scale = CalcPointLightSphereScale(m_PLights[i]) * 1.5f;
			m4 mvp = m_ViewProjection * (m4::CreateTranslation(m_PLights[i].position) * m4::CreateScale(v3(scale, scale, scale)));

			m_RenderContext->UpdateShaderBuffer(m_PLightMVPBuffer, &mvp, sizeof(m4));

			m_RenderContext->SwitchPipeline(2);
			m_RenderContext->ClearStencil(0);
			m_RenderContext->SubmitRenderCommand(m_DrawBoundingSphere);

			m_RenderContext->SwitchPipeline(1);
			m_RenderContext->UpdateShaderBuffer(m_PLightBuffer, &m_PLights[i], sizeof(PointLight));
			m_RenderContext->SubmitRenderCommand(m_DrawBoundingSphere);
		}

		m_RenderContext->NextSubpass();

		RenderCommand wireframeCommand;
		wireframeCommand.vertexOffset = 0;
		wireframeCommand.indexOffset = 0;
		wireframeCommand.indexType = INDEX_TYPE_U32;

		for (u32 i = 0; i < m_WireframeRenderables.Size(); i++)
		{
			const SubmittedWireframeRenderable& renderable = m_WireframeRenderables[i];
			m_RenderContext->UpdateShaderBuffer(m_WireframePerInstanceBuffer, &renderable.transform, sizeof(m4));
			m_RenderContext->UpdateShaderBuffer(m_WireframeBuffer, &m_WireframeColor, sizeof(v3));
			wireframeCommand.vertexBuffer = renderable.vertexBuffer;
			wireframeCommand.indexBuffer = renderable.indexBuffer;
			wireframeCommand.count = renderable.totalIndexCount;
			
			m_RenderContext->SubmitRenderCommand(wireframeCommand);
		}

		m_RenderContext->NextSubpass();
		m_RenderContext->UpdateShaderBuffer(m_BloomThresholdBuffer, &m_BloomThreshold, sizeof(r32));
		m_RenderContext->SubmitRenderCommand(m_DrawQuad);

		m_RenderContext->EndRenderPass();
	}

	void Renderer3D::DoBloomPass()
	{
		RenderPassBeginInfo blurBeginInfo;
		blurBeginInfo.initialPipeline = 0;
		blurBeginInfo.renderPass = m_GaussIter1RenderPass;
		blurBeginInfo.numClearValues = 0;

		TextureGroupBind blurBind;
		blurBind.set = 0;
		blurBind.numTextureBinds = 1;
		blurBind.binds[0].sampler = m_NearestSampler;
		blurBind.binds[0].binding = 0;
		blurBind.binds[0].texture[0] = m_Textures.gbufferBloomThreshold;
		blurBind.binds[0].textureArrayLength = 1;

		for (u32 i = 0; i < m_BloomBlurIterationCount; i++)
		{
			blurBeginInfo.renderPass = m_GaussIter1RenderPass;
			if (i > 0)
				blurBind.binds[0].texture[0] = m_GaussIter2Texture;

			m_RenderContext->BeginRenderPass(blurBeginInfo);
			m_RenderContext->BindTextureGroup(blurBind);
			m_RenderContext->SubmitRenderCommand(m_DrawQuad);
			m_RenderContext->EndRenderPass();

			blurBeginInfo.renderPass = m_GaussIter2RenderPass;
			blurBind.binds[0].texture[0] = m_GaussIter1Texture;

			m_RenderContext->BeginRenderPass(blurBeginInfo);
			m_RenderContext->BindTextureGroup(blurBind);
			m_RenderContext->SubmitRenderCommand(m_DrawQuad);
			m_RenderContext->EndRenderPass();
		}
	}

	void Renderer3D::DoFinalPass()
	{
		TextureGroupBind finalBind;
		finalBind.set = 0;
		finalBind.numTextureBinds = 2;
		finalBind.binds[0].sampler = m_NearestSampler;
		finalBind.binds[0].binding = 0;
		finalBind.binds[0].texture[0] = m_Textures.gbufferOutput;
		finalBind.binds[0].textureArrayLength = 1;
		finalBind.binds[1].sampler = m_NearestSampler;
		finalBind.binds[1].binding = 1;
		finalBind.binds[1].texture[0] = m_GaussIter2Texture;
		finalBind.binds[1].textureArrayLength = 1;

		RenderPassBeginInfo finalBeginInfo;
		finalBeginInfo.initialPipeline = 0;
		finalBeginInfo.renderPass = m_FinalRenderPass;
		finalBeginInfo.numClearValues = 0;

		m_RenderContext->BeginRenderPass(finalBeginInfo);
		m_RenderContext->BindTextureGroup(finalBind);
		m_RenderContext->SubmitRenderCommand(m_DrawQuad);
		m_RenderContext->EndRenderPass();
	}

	void Renderer3D::RenderFrame()
	{
		DoDeferredPass();
		DoBloomPass();
		DoFinalPass();
	}

	LightingModel Renderer3D::GetLightingModel()
	{
		return m_LightingModel;
	}

	v3& Renderer3D::GetAmbient()
	{
		return m_Ambient;
	}

	r32& Renderer3D::GetBloomThreshold()
	{
		return m_BloomThreshold;
	}

	u32& Renderer3D::GetBloomBlurIterCount()
	{
		return m_BloomBlurIterationCount;
	}

	v3& Renderer3D::GetWireframeColor()
	{
		return m_WireframeColor;
	}

	const m4& Renderer3D::GetView()
	{
		return m_View;
	}

	const m4& Renderer3D::GetProjection()
	{
		return m_Projection;
	}

	const m4& Renderer3D::GetViewProjection()
	{
		return m_ViewProjection;
	}

	const v3& Renderer3D::GetCameraPos()
	{
		return m_CamPos;
	}

	const quat& Renderer3D::GetCameraRot()
	{
		return m_CamRot;
	}

	EntityID Renderer3D::GetEntityIDFromGBuffer(u32 px, u32 py)
	{
		u32 width, height;
		m_RenderContext->GetFramebufferSize(m_DeferredPass, &width, &height);

		m_RenderContext->ReadPixelsIntoBuffer(m_Textures.gbufferAlbedo, m_AlbedoTexturePixels);
		u16* pixels = (u16*)m_RenderContext->MapBuffer(m_AlbedoTexturePixels);

		u32 pixelIndex = py * width + px;

		if (pixelIndex >= width * height)
			return EU_ECS_INVALID_ENTITY_ID;

		u16 alphaComponent = pixels[pixelIndex * 4 + 3];
		r32 normalizedAlpha = (r32)alphaComponent / (r32)EU_U16_MAX;
		m_RenderContext->UnmapBuffer(m_AlbedoTexturePixels);
		EntityID entity = EntityID(EU_ROUND(normalizedAlpha * 1000.0f));
		return entity;
	}

	const Renderer3DOutputTextures& Renderer3D::GetOutputTextures()
	{
		return m_Textures;
	}

	void Renderer3D::ResizeOutput(u32 width, u32 height)
	{
		m_RenderContext->ResizeFramebuffer(m_DeferredPass, width, height);
		m_RenderContext->ResizeFramebuffer(m_GaussIter1RenderPass, width, height);
		m_RenderContext->ResizeFramebuffer(m_GaussIter2RenderPass, width, height);
		m_RenderContext->ResizeFramebuffer(m_FinalRenderPass, width, height);
		u32 numPixelComponents = width * height * 4;
		m_RenderContext->RecreateBuffer(m_AlbedoTexturePixels, BUFFER_TYPE_MEMORY_TRANSFER_DST, BUFFER_USAGE_DYNAMIC, 0, numPixelComponents * sizeof(u16));
	}

	void Renderer3D::InitDeferredRenderPass(LightingModel lightingModel)
	{
		RenderPass deferredPass;

		Framebuffer* framebuffer = &deferredPass.framebuffer;
		framebuffer->useSwapchainSize = true;
		framebuffer->numAttachments = 6;

		b32 createTextureHandlesForIntermediateTextures = false;
		if (Engine::IsEditorAttached())
			createTextureHandlesForIntermediateTextures = true;

		//Output Attachment
		framebuffer->attachments[0].format = TEXTURE_FORMAT_RGBA32_FLOAT;
		framebuffer->attachments[0].isClearAttachment = true;
		framebuffer->attachments[0].isSamplerAttachment = true;
		framebuffer->attachments[0].isStoreAttachment = true;
		framebuffer->attachments[0].isSubpassInputAttachment = true;
		framebuffer->attachments[0].isSwapchainAttachment = false;
		framebuffer->attachments[0].nonClearAttachmentPreserve = false;
		framebuffer->attachments[0].memoryTransferSrc = false;
		//Depth Attachment
		framebuffer->attachments[1].format = TEXTURE_FORMAT_DEPTH32_FLOAT_STENCIL8_UINT;
		framebuffer->attachments[1].isClearAttachment = true;
		framebuffer->attachments[1].isSamplerAttachment = createTextureHandlesForIntermediateTextures;
		framebuffer->attachments[1].isStoreAttachment = false;
		framebuffer->attachments[1].isSubpassInputAttachment = false;
		framebuffer->attachments[1].isSwapchainAttachment = false;
		framebuffer->attachments[1].nonClearAttachmentPreserve = false;
		framebuffer->attachments[1].memoryTransferSrc = false;
		//Albedo(RGB) + EntityID(A) Attachment 
		framebuffer->attachments[2].format = TEXTURE_FORMAT_RGBA16_UNORM;
		framebuffer->attachments[2].isClearAttachment = true;
		framebuffer->attachments[2].isSamplerAttachment = createTextureHandlesForIntermediateTextures;
		framebuffer->attachments[2].isStoreAttachment = false;
		framebuffer->attachments[2].isSubpassInputAttachment = true;
		framebuffer->attachments[2].isSwapchainAttachment = false;
		framebuffer->attachments[2].nonClearAttachmentPreserve = false;
		framebuffer->attachments[2].memoryTransferSrc = createTextureHandlesForIntermediateTextures;
		//Position(RGB) + Specular/Metallic(A) Attachment
		framebuffer->attachments[3].format = TEXTURE_FORMAT_RGBA32_FLOAT;
		framebuffer->attachments[3].isClearAttachment = true;
		framebuffer->attachments[3].isSamplerAttachment = createTextureHandlesForIntermediateTextures;
		framebuffer->attachments[3].isStoreAttachment = false;
		framebuffer->attachments[3].isSubpassInputAttachment = true;
		framebuffer->attachments[3].isSwapchainAttachment = false;
		framebuffer->attachments[3].nonClearAttachmentPreserve = false;
		framebuffer->attachments[3].memoryTransferSrc = false;
		//Normal(RGB) + Gloss/Roughness(A) Attachment
		framebuffer->attachments[4].format = TEXTURE_FORMAT_RGBA32_FLOAT;
		framebuffer->attachments[4].isClearAttachment = true;
		framebuffer->attachments[4].isSamplerAttachment = createTextureHandlesForIntermediateTextures;
		framebuffer->attachments[4].isStoreAttachment = false;
		framebuffer->attachments[4].isSubpassInputAttachment = true;
		framebuffer->attachments[4].isSwapchainAttachment = false;
		framebuffer->attachments[4].nonClearAttachmentPreserve = false;
		framebuffer->attachments[4].memoryTransferSrc = false;
		//Bloom Threshold Attachment
		framebuffer->attachments[5].format = TEXTURE_FORMAT_RGBA32_FLOAT;
		framebuffer->attachments[5].isClearAttachment = true;
		framebuffer->attachments[5].isSamplerAttachment = true;
		framebuffer->attachments[5].isSamplerAttachment = true;
		framebuffer->attachments[5].isSubpassInputAttachment = false;
		framebuffer->attachments[5].isSwapchainAttachment = false;
		framebuffer->attachments[5].nonClearAttachmentPreserve = false;
		framebuffer->attachments[5].memoryTransferSrc = false;

		Subpass gbufferPass;
		gbufferPass.useDepthStencilAttachment = true;
		gbufferPass.depthStencilAttachment = 1;
		gbufferPass.numReadAttachments = 0;
		gbufferPass.numWriteAttachments = 4;
		gbufferPass.writeAttachments[0] = 0;
		gbufferPass.writeAttachments[1] = 2;
		gbufferPass.writeAttachments[2] = 3;
		gbufferPass.writeAttachments[3] = 4;

		ShaderID gbufferShader = EU_INVALID_SHADER_ID;
		ShaderID directionalShader = EU_INVALID_SHADER_ID;
		ShaderID pointShader = EU_INVALID_SHADER_ID;
		ShaderID stencilShader = EU_INVALID_SHADER_ID;
		switch (lightingModel)
		{
			case LIGHTING_MODEL_BLINNPHONG: {
				gbufferShader = m_RenderContext->LoadShader("3D/Deferred/Blinn-Phong/GBuffer");
				directionalShader = m_RenderContext->LoadShader("3D/Deferred/Blinn-Phong/DirectionalLight");
				pointShader = m_RenderContext->LoadShader("3D/Deferred/Blinn-Phong/PointLight");
				stencilShader = m_RenderContext->LoadShader("3D/Deferred/Blinn-Phong/StencilWrite");
				break;
			} case LIGHTING_MODEL_PBR: {
				gbufferShader = m_RenderContext->LoadShader("3D/Deferred/PBR/GBuffer");
				directionalShader = m_RenderContext->LoadShader("3D/Deferred/PBR/DirectionalLight");
				pointShader = m_RenderContext->LoadShader("3D/Deferred/PBR/PointLight");
				stencilShader = m_RenderContext->LoadShader("3D/Deferred/PBR/StencilWrite");
				break;
			}
		}

		GraphicsPipeline gbufferPipeline{};
		gbufferPipeline.shader = gbufferShader;
		gbufferPipeline.topology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		gbufferPipeline.viewportState.useFramebufferSizeForViewport = true;
		gbufferPipeline.viewportState.useFramebufferSizeForScissor = true;
		gbufferPipeline.viewportState.scissor.x = gbufferPipeline.viewportState.scissor.y = 0;
		gbufferPipeline.viewportState.viewport.x = gbufferPipeline.viewportState.viewport.y = 0;
		gbufferPipeline.vertexInputState.vertexSize = EU_VERTEX_SIZE_AUTO;
		gbufferPipeline.vertexInputState.numAttributes = 7;
		gbufferPipeline.vertexInputState.attributes[0].location = 0;
		gbufferPipeline.vertexInputState.attributes[0].name = "POSITION";
		gbufferPipeline.vertexInputState.attributes[0].type = VERTEX_ATTRIBUTE_FLOAT3;
		gbufferPipeline.vertexInputState.attributes[1].location = 1;
		gbufferPipeline.vertexInputState.attributes[1].name = "COLOR";
		gbufferPipeline.vertexInputState.attributes[1].type = VERTEX_ATTRIBUTE_FLOAT3;
		gbufferPipeline.vertexInputState.attributes[2].location = 2;
		gbufferPipeline.vertexInputState.attributes[2].name = "NORMAL";
		gbufferPipeline.vertexInputState.attributes[2].type = VERTEX_ATTRIBUTE_FLOAT3;
		gbufferPipeline.vertexInputState.attributes[3].location = 3;
		gbufferPipeline.vertexInputState.attributes[3].name = "TANGENT";
		gbufferPipeline.vertexInputState.attributes[3].type = VERTEX_ATTRIBUTE_FLOAT3;
		gbufferPipeline.vertexInputState.attributes[4].location = 4;
		gbufferPipeline.vertexInputState.attributes[4].name = "TEXCOORD";
		gbufferPipeline.vertexInputState.attributes[4].type = VERTEX_ATTRIBUTE_FLOAT2;
		gbufferPipeline.vertexInputState.attributes[5].location = 5;
		gbufferPipeline.vertexInputState.attributes[5].name = "BONEIDS";
		gbufferPipeline.vertexInputState.attributes[5].type = VERTEX_ATTRIBUTE_U32_4;
		gbufferPipeline.vertexInputState.attributes[6].location = 6;
		gbufferPipeline.vertexInputState.attributes[6].name = "BONEWEIGHTS";
		gbufferPipeline.vertexInputState.attributes[6].type = VERTEX_ATTRIBUTE_FLOAT4;
		gbufferPipeline.rasterizationState.cullMode = CULL_MODE_BACK;
		gbufferPipeline.rasterizationState.depthClampEnabled = false;
		gbufferPipeline.rasterizationState.discard = false;
		gbufferPipeline.rasterizationState.frontFace = FRONT_FACE_CW;
		gbufferPipeline.rasterizationState.polygonMode = POLYGON_MODE_FILL;
		gbufferPipeline.numBlendStates = 4;
		gbufferPipeline.blendStates[0].blendEnabled = false;
		gbufferPipeline.blendStates[0].color.dstFactor = BLEND_FACTOR_ZERO;
		gbufferPipeline.blendStates[0].color.srcFactor = BLEND_FACTOR_ONE;
		gbufferPipeline.blendStates[0].color.operation = BLEND_OPERATION_ADD;
		gbufferPipeline.blendStates[0].alpha = gbufferPipeline.blendStates[0].color;
		gbufferPipeline.blendStates[1] = gbufferPipeline.blendStates[0];
		gbufferPipeline.blendStates[2] = gbufferPipeline.blendStates[0];
		gbufferPipeline.blendStates[3] = gbufferPipeline.blendStates[0];
		gbufferPipeline.depthStencilState.depthTestEnabled = true;
		gbufferPipeline.depthStencilState.depthWriteEnabled = true;
		gbufferPipeline.depthStencilState.stencilTestEnabled = false;
		gbufferPipeline.depthStencilState.depthCompare = COMPARE_OPERATION_LESS;
		gbufferPipeline.dynamicBuffers.Push("PerInstance");
		gbufferPipeline.dynamicBuffers.Push("MaterialModifier");
		gbufferPipeline.dynamicBuffers.Push("BoneBuffer");

		//Material textures bind
		MaxTextureGroupBinds maxTextureBinds;
		maxTextureBinds.set = 2;
		maxTextureBinds.maxBinds = EU_RENDERER3D_MAX_SUBMITIONS_PER_RENDERPASS;

		gbufferPipeline.maxTextureGroupBinds.Push(maxTextureBinds);

		gbufferPass.pipelines.Push(gbufferPipeline);

		Subpass lightPass;
		lightPass.useDepthStencilAttachment = true;
		lightPass.depthStencilAttachment = 1;
		lightPass.numReadAttachments = 3;
		lightPass.readAttachments[0] = 2;
		lightPass.readAttachments[1] = 3;
		lightPass.readAttachments[2] = 4;
		lightPass.numWriteAttachments = 1;
		lightPass.writeAttachments[0] = 0;

		GraphicsPipeline dlightPipeline{};
		dlightPipeline.vertexInputState.numAttributes = 1;
		dlightPipeline.vertexInputState.vertexSize = EU_VERTEX_SIZE_AUTO;
		dlightPipeline.vertexInputState.attributes[0].location = 0;
		dlightPipeline.vertexInputState.attributes[0].name = "POSITION";
		dlightPipeline.vertexInputState.attributes[0].type = VERTEX_ATTRIBUTE_FLOAT2;
		dlightPipeline.shader = directionalShader;
		dlightPipeline.topology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		dlightPipeline.rasterizationState.cullMode = CULL_MODE_BACK;
		dlightPipeline.rasterizationState.depthClampEnabled = false;
		dlightPipeline.rasterizationState.frontFace = FRONT_FACE_CW;
		dlightPipeline.rasterizationState.polygonMode = POLYGON_MODE_FILL;
		dlightPipeline.rasterizationState.discard = false;
		dlightPipeline.depthStencilState.depthTestEnabled = false;
		dlightPipeline.depthStencilState.depthWriteEnabled = false;
		dlightPipeline.depthStencilState.depthCompare = COMPARE_OPERATION_LESS;
		dlightPipeline.depthStencilState.stencilTestEnabled = false;
		dlightPipeline.numBlendStates = 1;
		dlightPipeline.blendStates[0].blendEnabled = true;
		dlightPipeline.blendStates[0].alpha.dstFactor = BLEND_FACTOR_ZERO;
		dlightPipeline.blendStates[0].alpha.srcFactor = BLEND_FACTOR_ONE;
		dlightPipeline.blendStates[0].alpha.operation = BLEND_OPERATION_ADD;
		dlightPipeline.blendStates[0].color.dstFactor = BLEND_FACTOR_ONE;
		dlightPipeline.blendStates[0].color.srcFactor = BLEND_FACTOR_ONE;
		dlightPipeline.blendStates[0].color.operation = BLEND_OPERATION_ADD;
		dlightPipeline.viewportState.viewport.x = 0;
		dlightPipeline.viewportState.viewport.y = 0;
		dlightPipeline.viewportState.scissor.x = 0;
		dlightPipeline.viewportState.scissor.y = 0;
		dlightPipeline.viewportState.useFramebufferSizeForViewport = true;
		dlightPipeline.viewportState.useFramebufferSizeForScissor = true;
		dlightPipeline.dynamicBuffers.Push("LightBuffer");

		GraphicsPipeline plightPipeline {};
		plightPipeline.vertexInputState.numAttributes = 1;
		plightPipeline.vertexInputState.vertexSize = EU_VERTEX_SIZE_AUTO;
		plightPipeline.vertexInputState.attributes[0].location = 0;
		plightPipeline.vertexInputState.attributes[0].name = "POSITION";
		plightPipeline.vertexInputState.attributes[0].type = VERTEX_ATTRIBUTE_FLOAT3;
		plightPipeline.topology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		plightPipeline.shader = pointShader;
		plightPipeline.rasterizationState.cullMode = CULL_MODE_FRONT;
		plightPipeline.rasterizationState.depthClampEnabled = false;
		plightPipeline.rasterizationState.frontFace = FRONT_FACE_CW;
		plightPipeline.rasterizationState.polygonMode = POLYGON_MODE_FILL;
		plightPipeline.rasterizationState.discard = false;
		plightPipeline.depthStencilState.depthTestEnabled = false;
		plightPipeline.depthStencilState.depthWriteEnabled = false;
		plightPipeline.depthStencilState.depthCompare = COMPARE_OPERATION_LESS;
		plightPipeline.depthStencilState.stencilTestEnabled = true;
		plightPipeline.depthStencilState.frontFace.compare = COMPARE_OPERATION_NOT_EQUAL;
		plightPipeline.depthStencilState.frontFace.compareMask = 0xFF;
		plightPipeline.depthStencilState.frontFace.reference = 0;
		plightPipeline.depthStencilState.frontFace.depthStencilPass = STENCIL_OPERATION_KEEP;
		plightPipeline.depthStencilState.frontFace.stencilFail = STENCIL_OPERATION_KEEP;
		plightPipeline.depthStencilState.frontFace.stencilPassDepthFail = STENCIL_OPERATION_DECREMENT_WRAP;
		plightPipeline.depthStencilState.frontFace.writeMask = 0xFF;
		plightPipeline.depthStencilState.backFace = plightPipeline.depthStencilState.frontFace;
		plightPipeline.depthStencilState.backFace.stencilPassDepthFail = STENCIL_OPERATION_INCREMENT_WRAP;
		plightPipeline.numBlendStates = 1;
		plightPipeline.blendStates[0].blendEnabled = true;
		plightPipeline.blendStates[0].alpha.dstFactor = BLEND_FACTOR_ZERO;
		plightPipeline.blendStates[0].alpha.srcFactor = BLEND_FACTOR_ONE;
		plightPipeline.blendStates[0].alpha.operation = BLEND_OPERATION_ADD;
		plightPipeline.blendStates[0].color.dstFactor = BLEND_FACTOR_ONE;
		plightPipeline.blendStates[0].color.srcFactor = BLEND_FACTOR_ONE;
		plightPipeline.blendStates[0].color.operation = BLEND_OPERATION_ADD;
		plightPipeline.viewportState.viewport.x = 0;
		plightPipeline.viewportState.viewport.y = 0;
		plightPipeline.viewportState.scissor.x = 0;
		plightPipeline.viewportState.scissor.y = 0;
		plightPipeline.viewportState.useFramebufferSizeForViewport = true;
		plightPipeline.viewportState.useFramebufferSizeForScissor = true;
		plightPipeline.dynamicBuffers.Push("MVP");
		plightPipeline.dynamicBuffers.Push("Light");

		GraphicsPipeline stencilPipeline {};
		stencilPipeline.vertexInputState.numAttributes = 1;
		stencilPipeline.vertexInputState.vertexSize = sizeof(v3);
		stencilPipeline.vertexInputState.attributes[0].location = 0;
		stencilPipeline.vertexInputState.attributes[0].name = "POSITION";
		stencilPipeline.vertexInputState.attributes[0].type = VERTEX_ATTRIBUTE_FLOAT3;
		stencilPipeline.topology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		stencilPipeline.shader = stencilShader;
		stencilPipeline.rasterizationState.cullMode = CULL_MODE_NONE;
		stencilPipeline.rasterizationState.depthClampEnabled = false;
		stencilPipeline.rasterizationState.frontFace = FRONT_FACE_CW;
		stencilPipeline.rasterizationState.polygonMode = POLYGON_MODE_FILL;
		stencilPipeline.rasterizationState.discard = false;
		stencilPipeline.depthStencilState.depthTestEnabled = true;
		stencilPipeline.depthStencilState.depthWriteEnabled = false;
		stencilPipeline.depthStencilState.depthCompare = COMPARE_OPERATION_LESS;
		stencilPipeline.depthStencilState.stencilTestEnabled = true;
		stencilPipeline.depthStencilState.frontFace.compare = COMPARE_OPERATION_ALWAYS;
		stencilPipeline.depthStencilState.frontFace.compareMask = 0;
		stencilPipeline.depthStencilState.frontFace.reference = 0;
		stencilPipeline.depthStencilState.frontFace.depthStencilPass = STENCIL_OPERATION_KEEP;
		stencilPipeline.depthStencilState.frontFace.stencilFail = STENCIL_OPERATION_KEEP;
		stencilPipeline.depthStencilState.frontFace.stencilPassDepthFail = STENCIL_OPERATION_DECREMENT_WRAP;
		stencilPipeline.depthStencilState.frontFace.writeMask = 0xFF;
		stencilPipeline.depthStencilState.backFace = stencilPipeline.depthStencilState.frontFace;
		stencilPipeline.depthStencilState.backFace.stencilPassDepthFail = STENCIL_OPERATION_INCREMENT_WRAP;
		stencilPipeline.numBlendStates = 1;
		stencilPipeline.blendStates[0].blendEnabled = true;
		stencilPipeline.blendStates[0].alpha.dstFactor = BLEND_FACTOR_ONE;
		stencilPipeline.blendStates[0].alpha.srcFactor = BLEND_FACTOR_ZERO;
		stencilPipeline.blendStates[0].alpha.operation = BLEND_OPERATION_ADD;
		stencilPipeline.blendStates[0].color.dstFactor = BLEND_FACTOR_ONE;
		stencilPipeline.blendStates[0].color.srcFactor = BLEND_FACTOR_ZERO;
		stencilPipeline.blendStates[0].color.operation = BLEND_OPERATION_ADD;
		stencilPipeline.viewportState.viewport.x = 0;
		stencilPipeline.viewportState.viewport.y = 0;
		stencilPipeline.viewportState.scissor.x = 0;
		stencilPipeline.viewportState.scissor.y = 0;
		stencilPipeline.viewportState.useFramebufferSizeForViewport = true;
		stencilPipeline.viewportState.useFramebufferSizeForScissor = true;
		stencilPipeline.dynamicBuffers.Push("MVP");

		lightPass.pipelines.Push(dlightPipeline);
		lightPass.pipelines.Push(plightPipeline);
		lightPass.pipelines.Push(stencilPipeline);

		Subpass bloomThresholdPass;
		bloomThresholdPass.depthStencilAttachment = 0;
		bloomThresholdPass.useDepthStencilAttachment = false;
		bloomThresholdPass.numReadAttachments = 1;
		bloomThresholdPass.readAttachments[0] = 0;
		bloomThresholdPass.numWriteAttachments = 1;
		bloomThresholdPass.writeAttachments[0] = 5;
		
		ShaderID bloomThresholdShader = m_RenderContext->LoadShader("3D/Deferred/BloomThreshold");

		GraphicsPipeline bloomThresholdPipeline {};
		bloomThresholdPipeline.shader = bloomThresholdShader;
		bloomThresholdPipeline.topology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		bloomThresholdPipeline.viewportState.viewport.x = 0;
		bloomThresholdPipeline.viewportState.viewport.y = 0;
		bloomThresholdPipeline.viewportState.scissor.x = 0;
		bloomThresholdPipeline.viewportState.scissor.y = 0;
		bloomThresholdPipeline.viewportState.useFramebufferSizeForViewport = true;
		bloomThresholdPipeline.viewportState.useFramebufferSizeForScissor = true;
		bloomThresholdPipeline.numBlendStates = 1;
		bloomThresholdPipeline.blendStates[0].blendEnabled = false;
		bloomThresholdPipeline.blendStates[0].blendEnabled = false;
		bloomThresholdPipeline.blendStates[0].color.dstFactor = BLEND_FACTOR_ZERO;
		bloomThresholdPipeline.blendStates[0].color.srcFactor = BLEND_FACTOR_ONE;
		bloomThresholdPipeline.blendStates[0].color.operation = BLEND_OPERATION_ADD;
		bloomThresholdPipeline.blendStates[0].alpha = gbufferPipeline.blendStates[0].color;
		bloomThresholdPipeline.rasterizationState.cullMode = CULL_MODE_BACK;
		bloomThresholdPipeline.rasterizationState.depthClampEnabled = false;
		bloomThresholdPipeline.rasterizationState.discard = false;
		bloomThresholdPipeline.rasterizationState.frontFace = FRONT_FACE_CW;
		bloomThresholdPipeline.rasterizationState.polygonMode = POLYGON_MODE_FILL;
		bloomThresholdPipeline.vertexInputState.vertexSize = EU_VERTEX_SIZE_AUTO;
		bloomThresholdPipeline.vertexInputState.numAttributes = 1;
		bloomThresholdPipeline.vertexInputState.attributes[0].name = "POSITION";
		bloomThresholdPipeline.vertexInputState.attributes[0].location = 0;
		bloomThresholdPipeline.vertexInputState.attributes[0].type = VERTEX_ATTRIBUTE_FLOAT2;
		bloomThresholdPipeline.depthStencilState.depthTestEnabled = false;
		bloomThresholdPipeline.depthStencilState.depthWriteEnabled = false;
		bloomThresholdPipeline.depthStencilState.stencilTestEnabled = false;
		bloomThresholdPipeline.depthStencilState.depthCompare = COMPARE_OPERATION_LESS;

		bloomThresholdPass.pipelines.Push(bloomThresholdPipeline);

		Subpass wireFramePass;
		wireFramePass.numReadAttachments = 0;
		wireFramePass.numWriteAttachments = 1;
		wireFramePass.useDepthStencilAttachment = true;
		wireFramePass.depthStencilAttachment = 1;
		wireFramePass.writeAttachments[0] = 0;
		
		ShaderID wireFrameShader = m_RenderContext->LoadShader("3D/Wireframe");

		GraphicsPipeline wireFramePipeline {};
		wireFramePipeline.topology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		wireFramePipeline.viewportState.useFramebufferSizeForScissor = wireFramePipeline.viewportState.useFramebufferSizeForViewport = true;
		wireFramePipeline.viewportState.viewport.x = wireFramePipeline.viewportState.viewport.y =
		wireFramePipeline.viewportState.scissor.x = wireFramePipeline.viewportState.scissor.y = 0;
		wireFramePipeline.shader = wireFrameShader;
		wireFramePipeline.vertexInputState.vertexSize = sizeof(ModelVertex);
		wireFramePipeline.vertexInputState.numAttributes = 1;
		wireFramePipeline.vertexInputState.attributes[0].location = 0;
		wireFramePipeline.vertexInputState.attributes[0].type = VERTEX_ATTRIBUTE_FLOAT3;
		wireFramePipeline.vertexInputState.attributes[0].name = "POSITION";
		wireFramePipeline.rasterizationState.cullMode = CULL_MODE_BACK;
		wireFramePipeline.rasterizationState.depthClampEnabled = false;
		wireFramePipeline.rasterizationState.discard = false;
		wireFramePipeline.rasterizationState.frontFace = FRONT_FACE_CW;
		wireFramePipeline.rasterizationState.polygonMode = POLYGON_MODE_LINE;
		wireFramePipeline.numBlendStates = 1;
		wireFramePipeline.blendStates[0].blendEnabled = false;
		wireFramePipeline.blendStates[0].alpha.dstFactor = BLEND_FACTOR_ZERO;
		wireFramePipeline.blendStates[0].alpha.srcFactor = BLEND_FACTOR_ONE;
		wireFramePipeline.blendStates[0].alpha.operation = BLEND_OPERATION_ADD;
		wireFramePipeline.blendStates[0].color = wireFramePipeline.blendStates[0].alpha;
		wireFramePipeline.dynamicBuffers.Push("PerInstance");
		wireFramePipeline.depthStencilState.stencilTestEnabled = false;
		wireFramePipeline.depthStencilState.depthTestEnabled = true;
		wireFramePipeline.depthStencilState.depthWriteEnabled = false;
		wireFramePipeline.depthStencilState.depthCompare = COMPARE_OPERATION_LESS;
		
		wireFramePass.pipelines.Push(wireFramePipeline);

		deferredPass.subpasses.Push(gbufferPass);
		deferredPass.subpasses.Push(lightPass);
		deferredPass.subpasses.Push(wireFramePass);
		deferredPass.subpasses.Push(bloomThresholdPass);

		m_DeferredPass = m_RenderContext->CreateRenderPass(deferredPass);
		m_Textures.gbufferOutput = m_RenderContext->CreateTextureHandleForFramebufferAttachment(m_DeferredPass, 0);
		m_Textures.gbufferBloomThreshold = m_RenderContext->CreateTextureHandleForFramebufferAttachment(m_DeferredPass, 5);

		if (createTextureHandlesForIntermediateTextures)
		{
			m_Textures.gbufferDepth = m_RenderContext->CreateTextureHandleForFramebufferAttachment(m_DeferredPass, 1);
			m_Textures.gbufferAlbedo = m_RenderContext->CreateTextureHandleForFramebufferAttachment(m_DeferredPass, 2);
			m_Textures.gbufferPosition = m_RenderContext->CreateTextureHandleForFramebufferAttachment(m_DeferredPass, 3);
			m_Textures.gbufferNormal = m_RenderContext->CreateTextureHandleForFramebufferAttachment(m_DeferredPass, 4);
		}
	}

	void Renderer3D::InitGuassianBlurRenderPass()
	{
		const r32 resScale = 1.0f;

		u32 width, height;
		m_RenderContext->GetTextureSize(m_Textures.gbufferBloomThreshold, &width, &height);

		RenderPass gaussRenderPass;
		Framebuffer* framebuffer = &gaussRenderPass.framebuffer;
		framebuffer->useSwapchainSize = false;
		framebuffer->width = (r32)width * resScale;
		framebuffer->height = (r32)height * resScale;
		framebuffer->numAttachments = 1;
		framebuffer->attachments[0].format = TEXTURE_FORMAT_RGBA32_FLOAT;
		framebuffer->attachments[0].isClearAttachment = false;
		framebuffer->attachments[0].isSamplerAttachment = true;
		framebuffer->attachments[0].isStoreAttachment = true;
		framebuffer->attachments[0].isSubpassInputAttachment = false;
		framebuffer->attachments[0].isSwapchainAttachment = false;
		framebuffer->attachments[0].nonClearAttachmentPreserve = false;
		framebuffer->attachments[0].memoryTransferSrc = false;

		Subpass subpass;
		subpass.useDepthStencilAttachment = false;
		subpass.depthStencilAttachment = 0;
		subpass.numReadAttachments = 0;
		subpass.numWriteAttachments = 1;
		subpass.writeAttachments[0] = 0;
		
		ShaderID blurShaderHorizontal = m_RenderContext->LoadShader("3D/GaussianBlurHorizontal");
		ShaderID blurShaderVertical = m_RenderContext->LoadShader("3D/GaussianBlurVertical");

		GraphicsPipeline pipeline {};
		pipeline.shader = blurShaderHorizontal;
		pipeline.topology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipeline.viewportState.useFramebufferSizeForViewport = true;
		pipeline.viewportState.useFramebufferSizeForScissor = true;
		pipeline.viewportState.scissor.x = pipeline.viewportState.scissor.y = 0;
		pipeline.viewportState.viewport.x = pipeline.viewportState.viewport.y = 0;
		pipeline.vertexInputState.vertexSize = EU_VERTEX_SIZE_AUTO;
		pipeline.vertexInputState.numAttributes = 1;
		pipeline.vertexInputState.attributes[0].location = 0;
		pipeline.vertexInputState.attributes[0].name = "POSITION";
		pipeline.vertexInputState.attributes[0].type = VERTEX_ATTRIBUTE_FLOAT2;
		pipeline.rasterizationState.cullMode = CULL_MODE_BACK;
		pipeline.rasterizationState.depthClampEnabled = false;
		pipeline.rasterizationState.discard = false;
		pipeline.rasterizationState.frontFace = FRONT_FACE_CW;
		pipeline.rasterizationState.polygonMode = POLYGON_MODE_FILL;
		pipeline.numBlendStates = 1;
		pipeline.blendStates[0].blendEnabled = false;
		pipeline.blendStates[0].color.dstFactor = BLEND_FACTOR_ZERO;
		pipeline.blendStates[0].color.srcFactor = BLEND_FACTOR_ONE;
		pipeline.blendStates[0].color.operation = BLEND_OPERATION_ADD;
		pipeline.blendStates[0].alpha = pipeline.blendStates[0].color;
		pipeline.depthStencilState.depthTestEnabled = false;
		pipeline.depthStencilState.depthWriteEnabled = false;
		pipeline.depthStencilState.stencilTestEnabled = false;
		pipeline.depthStencilState.depthCompare = COMPARE_OPERATION_LESS;

		MaxTextureGroupBinds maxBinds;
		maxBinds.set = 0;
		maxBinds.maxBinds = EU_RENDERER3D_MAX_BLOOM_BLUR_ITERATIONS;

		pipeline.maxTextureGroupBinds.Push(maxBinds);

		subpass.pipelines.Push(pipeline);
		gaussRenderPass.subpasses.Push(subpass);

		m_GaussIter1RenderPass = m_RenderContext->CreateRenderPass(gaussRenderPass);
		gaussRenderPass.subpasses[0].pipelines[0].shader = blurShaderVertical;
		m_GaussIter2RenderPass = m_RenderContext->CreateRenderPass(gaussRenderPass);
		m_GaussIter1Texture = m_RenderContext->CreateTextureHandleForFramebufferAttachment(m_GaussIter1RenderPass, 0);
		m_GaussIter2Texture = m_RenderContext->CreateTextureHandleForFramebufferAttachment(m_GaussIter2RenderPass, 0);
		m_Textures.bloomTexture = m_GaussIter2Texture;
	}

	void Renderer3D::InitFinalRenderPass()
	{
		u32 width, height;
		m_RenderContext->GetTextureSize(m_Textures.gbufferOutput, &width, &height);

		RenderPass renderPass;
		Framebuffer* framebuffer = &renderPass.framebuffer;
		framebuffer->useSwapchainSize = false;
		framebuffer->width = width;
		framebuffer->height = height;
		framebuffer->numAttachments = 1;
		framebuffer->attachments[0].format = TEXTURE_FORMAT_RGBA32_FLOAT;
		framebuffer->attachments[0].isClearAttachment = false;
		framebuffer->attachments[0].isSamplerAttachment = true;
		framebuffer->attachments[0].isStoreAttachment = true;
		framebuffer->attachments[0].isSubpassInputAttachment = false;
		framebuffer->attachments[0].isSwapchainAttachment = false;
		framebuffer->attachments[0].nonClearAttachmentPreserve = false;
		framebuffer->attachments[0].memoryTransferSrc = false;

		Subpass subpass;
		subpass.useDepthStencilAttachment = false;
		subpass.depthStencilAttachment = 0;
		subpass.numReadAttachments = 0;
		subpass.numWriteAttachments = 1;
		subpass.writeAttachments[0] = 0;

		ShaderID hdrPlusBloomShader = m_RenderContext->LoadShader("3D/HdrPlusBloom");

		GraphicsPipeline pipeline{};
		pipeline.shader = hdrPlusBloomShader;
		pipeline.topology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipeline.viewportState.useFramebufferSizeForViewport = true;
		pipeline.viewportState.useFramebufferSizeForScissor = true;
		pipeline.viewportState.scissor.x = pipeline.viewportState.scissor.y = 0;
		pipeline.viewportState.viewport.x = pipeline.viewportState.viewport.y = 0;
		pipeline.vertexInputState.vertexSize = EU_VERTEX_SIZE_AUTO;
		pipeline.vertexInputState.numAttributes = 1;
		pipeline.vertexInputState.attributes[0].location = 0;
		pipeline.vertexInputState.attributes[0].name = "POSITION";
		pipeline.vertexInputState.attributes[0].type = VERTEX_ATTRIBUTE_FLOAT2;
		pipeline.rasterizationState.cullMode = CULL_MODE_BACK;
		pipeline.rasterizationState.depthClampEnabled = false;
		pipeline.rasterizationState.discard = false;
		pipeline.rasterizationState.frontFace = FRONT_FACE_CW;
		pipeline.rasterizationState.polygonMode = POLYGON_MODE_FILL;
		pipeline.numBlendStates = 1;
		pipeline.blendStates[0].blendEnabled = false;
		pipeline.blendStates[0].color.dstFactor = BLEND_FACTOR_ZERO;
		pipeline.blendStates[0].color.srcFactor = BLEND_FACTOR_ONE;
		pipeline.blendStates[0].color.operation = BLEND_OPERATION_ADD;
		pipeline.blendStates[0].alpha = pipeline.blendStates[0].color;
		pipeline.depthStencilState.depthTestEnabled = false;
		pipeline.depthStencilState.depthWriteEnabled = false;
		pipeline.depthStencilState.stencilTestEnabled = false;
		pipeline.depthStencilState.depthCompare = COMPARE_OPERATION_LESS;

		MaxTextureGroupBinds maxBinds;
		maxBinds.set = 0;
		maxBinds.maxBinds = 1;

		pipeline.maxTextureGroupBinds.Push(maxBinds);
		subpass.pipelines.Push(pipeline);
		renderPass.subpasses.Push(subpass);

		m_FinalRenderPass = m_RenderContext->CreateRenderPass(renderPass);
		m_Textures.outputTexture = m_RenderContext->CreateTextureHandleForFramebufferAttachment(m_FinalRenderPass, 0);
	}

	void Renderer3D::BindMaterial(MaterialID materialID)
	{
		const Material& material = AssetManager::GetMaterial(materialID);

		TextureGroupBind materialBind;
		materialBind.set = 2;
		materialBind.numTextureBinds = 4;

		materialBind.binds[0].binding = 0;
		materialBind.binds[0].sampler = material.sampler;
		materialBind.binds[0].texture[0] = material.textures[MATERIAL_TEXTURE_ALBEDO];
		materialBind.binds[0].textureArrayLength = 1;

		materialBind.binds[1].binding = 1;
		materialBind.binds[1].sampler = material.sampler;
		materialBind.binds[1].texture[0] = material.textures[MATERIAL_TEXTURE_NORMAL];
		materialBind.binds[1].textureArrayLength = 1;

		materialBind.binds[2].binding = 2;
		materialBind.binds[2].sampler = material.sampler;
		materialBind.binds[2].texture[0] = material.textures[MATERIAL_TEXTURE_DISPLACEMENT];
		materialBind.binds[2].textureArrayLength = 1;

		materialBind.binds[3].binding = 3;
		materialBind.binds[3].sampler = material.sampler;
		//materialBind.binds[3].texture[0] depends on lighting model
		materialBind.binds[3].textureArrayLength = 1;

		materialBind.binds[4].binding = 4;
		materialBind.binds[4].sampler = material.sampler;
		//materialBind.binds[4].texture[0] depends on lighting model
		materialBind.binds[4].textureArrayLength = 1;

		switch (m_LightingModel)
		{
			case LIGHTING_MODEL_BLINNPHONG: {
				materialBind.binds[3].texture[0] = material.textures[MATERIAL_TEXTURE_SPECULAR];
				materialBind.binds[4].texture[0] = material.textures[MATERIAL_TEXTURE_GLOSS];
				break;
			} case LIGHTING_MODEL_PBR: {
				materialBind.binds[3].texture[0] = material.textures[MATERIAL_TEXTURE_METALLIC];
				materialBind.binds[4].texture[0] = material.textures[MATERIAL_TEXTURE_ROUGHNESS];
				break;
			}
		}

		m_RenderContext->BindTextureGroup(materialBind);
	}

	void Renderer3D::BindMaterialModifier(MaterialModifierID modifierID)
	{
		const MaterialModifier& modifier = AssetManager::GetMaterialModifier(modifierID);

		GBufferMaterialModifierBuffer materialModifier;
		materialModifier.albedo = modifier.albedo;
		materialModifier.texCoordScale = modifier.texCoordScale;
		materialModifier.ao = modifier.ao;
		materialModifier.dispScale = modifier.dispScale;
		materialModifier.dispOffset = modifier.dispOffset;

		switch (m_LightingModel)
		{
			case LIGHTING_MODEL_BLINNPHONG: {
				materialModifier.specularOrMetallic = modifier.specular;
				materialModifier.glossOrRoughness = modifier.gloss;
				break;
			} case LIGHTING_MODEL_PBR: {
				materialModifier.specularOrMetallic = modifier.metallic;
				materialModifier.glossOrRoughness = modifier.roughness;
				break;
			}
		}

		m_RenderContext->UpdateShaderBuffer(m_GBufferMaterialModifierBuffer, &materialModifier, sizeof(GBufferMaterialModifierBuffer));
	}
}
