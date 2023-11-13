#pragma once

#include "RenderContext.h"
#include "Asset/Model.h"
#include "../Math/Math.h"
#include "Light3D.h"
#include "../ECS/ECSTypes.h"

#define EU_RENDERER3D_MAX_SUBMITIONS_PER_RENDERPASS 512
#define EU_RENDERER3D_MAX_WIREFRAME_SUBMITIONS_PER_RENDERPASS 256
#define EU_RENDERER3D_MAX_DIRECTIONAL_LIGHTS 8
#define EU_RENDERER3D_MAX_POINT_LIGHTS 32
#define EU_RENDERER3D_MAX_BLOOM_BLUR_ITERATIONS 16
#define EU_RENDERER3D_MAX_BONES 150

namespace Eunoia {

	enum LightingModel
	{
		LIGHTING_MODEL_BLINNPHONG,
		LIGHTING_MODEL_PBR,

		NUM_LIGHTING_MODELS
	};

	struct Renderer3DOutputTextures
	{
		TextureID shadowMap;
		TextureID gbufferOutput;
		TextureID gbufferDepth;
		TextureID gbufferAlbedo;
		TextureID gbufferPosition;
		TextureID gbufferNormal;
		TextureID gbufferBloomThreshold;
		TextureID bloomTexture;
		TextureID outputTexture;
	};

	struct SubmittedRenderable
	{
		BufferID vertexBuffer;
		BufferID indexBuffer;
		EntityID entityID;
		u32 totalIndexCount;
		List<LoadedMesh> meshes;
		List<MaterialID> materials;
		List<MaterialModifierID> modifiers;
		m4* boneTransforms;
		u32 numBoneTransforms;
		m4 transform;
		b32 animated;
	};

	struct SubmittedWireframeRenderable
	{
		BufferID vertexBuffer;
		BufferID indexBuffer;
		u32 totalIndexCount;
		m4 transform;
	};

	struct DirectionalLight
	{
		v4 color;
		v3 direction;
		r32 p0;
		m4 lightMatrix;
	};

	struct PointLight
	{
		v4 color;
		v3 position;
		r32 p0;
		Attenuation attenuation;
		r32 p1;
	};

	struct GBufferPerInstanceBuffer
	{
		m4 model;
		u32 animated;
		u32 entityID;
	};

	struct DirectionalLightSubmission
	{
		DirectionalLight light;
		ShadowInfo shadowInfo;
	};

	class EU_API Renderer3D
	{
	public:
		Renderer3D(RenderContext* renderContext, Display* display);
		~Renderer3D();

		void SetCamera(const m4& view, const m4& projection, const v3& pos, const quat& rot);
		void SetAmbient(const v3& ambient);
		void SetBloomThreshold(r32 threshold);
		void SetBloomBlurIterationCount(u32 iterationCount);
		void SetWireframeColor(const v3& color);
		
		TextureID Init(LightingModel lightingModel = LIGHTING_MODEL_BLINNPHONG);
		void BeginFrame();
		void SubmitModel(const Model& model, const m4& transform, m4* boneTransforms = 0, u32 numBoneTransforms = 0, b32 animated = false, EntityID entity = EU_ECS_INVALID_ENTITY_ID);
		void SubmitWireframeModel(const Model& model, const m4& transform);
		void SubmitLight(const Light3D& light);
		void EndFrame();
		void RenderFrame();
		
		LightingModel GetLightingModel();
		v3& GetAmbient();
		r32& GetBloomThreshold();
		u32& GetBloomBlurIterCount();
		v3& GetWireframeColor();
		
		const m4& GetView();
		const m4& GetProjection();
		const m4& GetViewProjection();
		const v3& GetCameraPos();
		const quat& GetCameraRot();
		
		EntityID GetEntityIDFromGBuffer(u32 px, u32 py);
		
		const Renderer3DOutputTextures& GetOutputTextures();
		
		void ResizeOutput(u32 width, u32 height);
	private:
		friend void DisplayResizeCallback(const DisplayEvent& e, void* userPtr);
		void InitShadowMapPass(u32 resolution);
		void InitDeferredRenderPass(LightingModel lightingModel);
		void InitGuassianBlurRenderPass();
		void InitFinalRenderPass();
		void BindMaterial(MaterialID material);
		void BindMaterialModifier(MaterialModifierID modifier);

		void DoShadowMapPass();
		void DoDeferredPass();
		void DoBloomPass();
		void DoFinalPass();
	private:
		RenderContext* m_RenderContext;
		Display* m_Display;

		RenderPassID m_DeferredPass;
		RenderPassID m_GaussIter1RenderPass;
		RenderPassID m_GaussIter2RenderPass;
		RenderPassID m_FinalRenderPass;

		BufferID m_AlbedoTexturePixels;

		TextureID m_GaussIter1Texture;
		TextureID m_GaussIter2Texture;
		SamplerID m_NearestSampler;
		SamplerID m_LinearSampler;
		SamplerID m_ShadowMapSampler;

		Renderer3DOutputTextures m_Textures;

		GBufferPerInstanceBuffer m_GBufferPerInstanceBufferData;
		ShaderBufferID m_GBufferPerFrameBuffer;
		ShaderBufferID m_GBufferPerInstanceBuffer;
		ShaderBufferID m_GBufferBoneBuffer;
		ShaderBufferID m_GBufferMaterialModifierBuffer;
		ShaderBufferID m_LightPerFrameBuffer;
		ShaderBufferID m_DLightLightBuffer;
		ShaderBufferID m_PLightBuffer;
		ShaderBufferID m_PLightMVPBuffer;
		ShaderBufferID m_BloomThresholdBuffer;
		ShaderBufferID m_WireframePerInstanceBuffer;
		ShaderBufferID m_WireframeBuffer;

		LightingModel m_LightingModel;

		m4 m_View;
		m4 m_Projection;
		m4 m_ViewProjection;
		v3 m_CamPos;
		quat m_CamRot;

		v3 m_Ambient;
		r32 m_BloomThreshold;
		u32 m_BloomBlurIterationCount;
		v3 m_WireframeColor;

		RenderCommand m_DrawQuad;
		RenderCommand m_DrawBoundingSphere;
		List<SubmittedRenderable> m_Renderables;
		List< SubmittedWireframeRenderable> m_WireframeRenderables;
		List<DirectionalLightSubmission> m_DLights;
		List<PointLight> m_PLights;
	};

}