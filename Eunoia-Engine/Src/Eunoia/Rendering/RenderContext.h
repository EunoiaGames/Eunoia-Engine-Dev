#pragma once

#include "../Rendering/Display.h"

#define EU_MAX_VERTEX_ATTRIBUTES 8
#define EU_MAX_FRAMEBUFFER_ATTACHMENTS 8
#define EU_MAX_ARRAY_OF_TEXTURES_SIZE 32

#define EU_MAX_TEXTURES_PER_GROUP 16
#define EU_DEFAULT_MAX_TEXTURE_GROUP_BINDS 8
#define EU_VERTEX_SIZE_AUTO 0

#define EU_INVALID_RENDER_CONTEXT_OBJECT_ID 0
#define EU_INVALID_SHADER_ID				EU_INVALID_RENDER_CONTEXT_OBJECT_ID
#define EU_INVALID_RENDER_PASS_ID			EU_INVALID_RENDER_CONTEXT_OBJECT_ID
#define EU_INVALID_UNIFORM_BUFFER_ID		EU_INVALID_RENDER_CONTEXT_OBJECT_ID
#define EU_INVALID_BUFFER_ID				EU_INVALID_RENDER_CONTEXT_OBJECT_ID
#define EU_INVALID_TEXTURE_ID				EU_INVALID_RENDER_CONTEXT_OBJECT_ID
#define EU_INVALID_SAMPLER_ID				EU_INVALID_RENDER_CONTEXT_OBJECT_ID

namespace Eunoia {

	typedef u32 RenderContextObjectID;
	typedef RenderContextObjectID ShaderID;
	typedef RenderContextObjectID RenderPassID;
	typedef RenderContextObjectID BufferID;
	typedef RenderContextObjectID ShaderBufferID;
	typedef RenderContextObjectID TextureID;
	typedef RenderContextObjectID SamplerID;

	enum RenderAPI
	{
		RENDER_API_VULKAN,

		NUM_RENDER_APIS
	};

	enum ShaderType
	{
		SHADER_TYPE_VERTEX,
		SHADER_TYPE_PIXEL,

		NUM_SHADER_TYPES
	};

	enum DynamicState
	{
		DYNAMIC_STATE_VIEWPORT,
		DYNAMIC_STATE_SCISSOR,
		NUM_DYNAMIC_STATES
	};

	enum VertexAttributeType
	{
		VERTEX_ATTRIBUTE_FLOAT,
		VERTEX_ATTRIBUTE_FLOAT2,
		VERTEX_ATTRIBUTE_FLOAT3,
		VERTEX_ATTRIBUTE_FLOAT4,

		VERTEX_ATTRIBUTE_U32,
		VERTEX_ATTRIBUTE_U32_2,
		VERTEX_ATTRIBUTE_U32_3,
		VERTEX_ATTRIBUTE_U32_4,

		NUM_VERTEX_AttRIBUTE_TYPES
	};

	struct VertexAttribute
	{
		VertexAttributeType type;
		u32 location;
		String name;
	};

	struct VertexInputState
	{
		VertexAttribute attributes[EU_MAX_VERTEX_ATTRIBUTES];
		u32 numAttributes;
		u32 vertexSize;
	};

	enum PrimitiveTopology
	{
		PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,

		NUM_PRIMITIVE_TOPOLOGIES
	};

	struct Rect
	{
		u32 x;
		u32 y;
		u32 width;
		u32 height;
	};

	enum PolygonMode
	{
		POLYGON_MODE_FILL,
		POLYGON_MODE_LINE,
		POLYGON_MODE_POINT,

		NUM_POLYGON_MODES
	};

	enum CullMode
	{
		CULL_MODE_FRONT,
		CULL_MODE_BACK,
		CULL_MODE_FRONT_BACK,
		CULL_MODE_NONE,

		NUM_CULL_MODES
	};

	enum FrontFace
	{
		FRONT_FACE_CW,
		FRONT_FACE_CCW,

		NUM_FRONT_FACES
	};

	struct RasterizationState
	{
		b32 depthClampEnabled;
		PolygonMode polygonMode;
		CullMode cullMode;
		FrontFace frontFace;
		b32 discard;
		//TODO: Depth bias
	};

	enum CompareOperation
	{
		COMPARE_OPERATION_NEVER,
		COMPARE_OPERATION_ALWAYS,
		COMPARE_OPERATION_EQUAL,
		COMPARE_OPERATION_NOT_EQUAL,
		COMPARE_OPERATION_GREATER,
		COMPARE_OPERATION_LESS,
		COMPARE_OPERATION_GREATER_EQUAL,
		COMPARE_OPERATION_LESS_EQUAL,

		NUM_COMPARE_OPERATIONS
	};

	enum StencilOperation
	{
		STENCIL_OPERATION_KEEP,
		STENCIL_OPERATION_ZERO,
		STENCIL_OPERATION_REPLACE,
		STENCIL_OPERATION_INCREMENT_WRAP,
		STENCIL_OPERATION_DECREMENT_WRAP,
		STENCIL_OPERATION_INCREMENT_CLAMP,
		STENCIL_OPERATION_DECREMENT_CLAMP,
		STENCIL_OPERATION_INVERT,

		NUM_STENCIL_OPERATIONS
	};

	struct StencilState
	{
		StencilOperation stencilFail;
		StencilOperation depthStencilPass;
		StencilOperation stencilPassDepthFail;
		CompareOperation compare;
		u32 compareMask;
		u32 writeMask;
		u32 reference;
	};

	struct DepthStencilState
	{
		b32 depthTestEnabled;
		b32 depthWriteEnabled;
		CompareOperation depthCompare;
		b32 stencilTestEnabled;
		StencilState frontFace;
		StencilState backFace;
	};

	enum BlendFactor
	{
		BLEND_FACTOR_ONE,
		BLEND_FACTOR_ZERO,
		BLEND_FACTOR_SRC,
		BLEND_FACTOR_DST,
		BLEND_FACTOR_ONE_MINUS_SRC,
		BLEND_FACTOR_ONE_MINUS_DST,
		BLEND_FACTOR_SRC_ALPHA,
		BLEND_FACTOR_DST_ALPHA,
		BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		BLEND_FACTOR_ONE_MINUS_DST_ALPHA,

		NUM_BLEND_FACTORS
	};

	enum BlendOperation
	{
		BLEND_OPERATION_ADD,
		BLEND_OPERATION_SUB,
		BLEND_OPERATION_REVERSE_SUB,
		BLEND_OPERATION_MIN,
		BLEND_OPERATION_MAX,

		NUM_BLEND_OPERATIONS
	};

	struct BlendEquation
	{
		BlendFactor srcFactor;
		BlendFactor dstFactor;
		BlendOperation operation;
	};

	struct BlendState
	{
		b32 blendEnabled;
		BlendEquation color;
		BlendEquation alpha;
	};

	struct ViewportState
	{
		Rect viewport;
		Rect scissor;
		b32 useFramebufferSizeForViewport;
		b32 useFramebufferSizeForScissor;
	};

	struct MaxTextureGroupBinds
	{
		u32 set;
		u32 maxBinds;
	};

	struct GraphicsPipeline
	{
		ShaderID shader;
		VertexInputState vertexInputState;
		PrimitiveTopology topology;
		ViewportState viewportState;
		RasterizationState rasterizationState;
		DepthStencilState depthStencilState;
		BlendState blendStates[EU_MAX_FRAMEBUFFER_ATTACHMENTS];
		u32 numBlendStates;
		b32 dynamicStates[NUM_DYNAMIC_STATES];

		List<MaxTextureGroupBinds> maxTextureGroupBinds;
		List<String> dynamicBuffers;
	};

	struct Subpass
	{
		List<GraphicsPipeline> pipelines;

		u32 readAttachments[EU_MAX_FRAMEBUFFER_ATTACHMENTS];
		u32 numReadAttachments;
		u32 writeAttachments[EU_MAX_FRAMEBUFFER_ATTACHMENTS];
		u32 numWriteAttachments;

		u32 depthStencilAttachment;
		b32 useDepthStencilAttachment;
	};

	enum TextureFormat
	{
		TEXTURE_FORMAT_R8_UNORM,
		TEXTURE_FORMAT_R16_FLOAT,
		TEXTURE_FORMAT_R32_FLOAT,

		TEXTURE_FORMAT_RGBA8_UNORM,
		TEXTURE_FORMAT_RGBA8_SRGB,
		TEXTURE_FORMAT_RGBA16_UNORM,
		TEXTURE_FORMAT_RGBA16_FLOAT,
		TEXTURE_FORMAT_RGBA32_FLOAT,
		TEXTURE_FORMAT_RGBA64_FLOAT,
		TEXTURE_FORMAT_SWAPCHAIN_FORMAT,

		TEXTURE_FORMAT_DEPTH32_FLOAT,
		TEXTURE_FORMAT_DEPTH32_FLOAT_STENCIL8_UINT,

		NUM_TEXTURE_FORMATS
	};

	struct FramebufferAttachment
	{
		TextureFormat format;
		b32 isSwapchainAttachment;
		b32 isClearAttachment;
		b32 nonClearAttachmentPreserve;
		b32 isSamplerAttachment;
		b32 isStoreAttachment;
		b32 isSubpassInputAttachment;
		b32 memoryTransferSrc;
	};

	struct Framebuffer
	{
		FramebufferAttachment attachments[EU_MAX_FRAMEBUFFER_ATTACHMENTS];
		u32 numAttachments;

		u32 width;
		u32 height;
		b32 useSwapchainSize;
	};

	struct RenderPass
	{
		List<Subpass> subpasses;
		Framebuffer framebuffer;
	};

	enum BufferType
	{
		BUFFER_TYPE_VERTEX,
		BUFFER_TYPE_INDEX,
		BUFFER_TYPE_MEMORY_TRANSFER_SRC,
		BUFFER_TYPE_MEMORY_TRANSFER_DST,

		NUM_BUFFER_TYPES
	};

	enum BufferUsage
	{
		BUFFER_USAGE_STATIC,
		BUFFER_USAGE_DYNAMIC,

		NUM_BUFFER_USAGES
	};

	enum ShaderBufferType
	{
		SHADER_BUFFER_UNIFORM_BUFFER,
		SHADER_BUFFER_STORAGE_BUFFER,
		NUM_SHADER_BUFFER_TYPES
	};

	enum IndexType
	{
		INDEX_TYPE_U16,
		INDEX_TYPE_U32,

		NUM_INDEX_TYPES
	};

	struct Color
	{
		r32 r;
		r32 g;
		r32 b;
		r32 a;
	};

	struct ClearValue
	{
		Color color;
		r32 depth;
		u32 stencil;
		b32 clearDepthStencil;
	};

	struct RenderPassBeginInfo
	{
		RenderPassID renderPass;
		ClearValue clearValues[EU_MAX_FRAMEBUFFER_ATTACHMENTS];
		u32 numClearValues;
		u32 initialPipeline;
	};

	struct RenderCommand
	{
		BufferID vertexBuffer;
		BufferID indexBuffer;
		IndexType indexType;
		u32 count;
		u32 vertexOffset;
		u32 indexOffset;
	};

	enum TextureType
	{
		TEXTURE_TYPE_1D,
		TEXTURE_TYPE_2D,
		TEXTURE_TYPE_CUBE,
		TEXTURE_TYPE_3D,

		NUM_TEXTURE_TYPES
	};

	enum TextureFilter
	{
		TEXTURE_FILTER_NEAREST,
		TEXTURE_FILTER_LINEAR,

		NUM_TEXTURE_FILTER_TYPES
	};

	enum TextureAddressMode
	{
		TEXTURE_ADDRESS_MODE_REPEAT,
		TEXTURE_ADDRESS_MODE_MIRROR_REPEAT,
		TEXTURE_ADDRESS_MODE_CLAMP_TO_EDGE,
		TEXTURE_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
		TEXTURE_ADDRESS_MODE_CLAMP_TO_BORDER,

		NUM_TEXTURE_ADDRESS_MODES
	};

	enum TextureBorderColor
	{
		TEXTURE_BORDER_COLOR_BLACK_INT,
		TEXTURE_BORDER_COLOR_WHITE_INT,
		TEXTURE_BORDER_COLOR_TRANSPARENT_INT,

		TEXTURE_BORDER_COLOR_BLACK_FLOAT,
		TEXTURE_BORDER_COLOR_WHITE_FLOAT,
		TEXTURE_BORDER_COLOR_TRANSPARENT_FLOAT
	};

	struct TextureBind
	{
		u32 binding;
		TextureID texture[EU_MAX_ARRAY_OF_TEXTURES_SIZE];
		u32 textureArrayLength;
		SamplerID sampler;
	};

	struct TextureGroupBind
	{
		TextureBind binds[EU_MAX_TEXTURES_PER_GROUP];
		u32 numTextureBinds;
		u32 set;
	};

	struct Sampler
	{
		TextureFilter magFilter;
		TextureFilter minFilter;
		TextureAddressMode addressModes[3];
		b32 anisotropyEnabled;
		r32 maxAnisotropy;
		TextureBorderColor borderColor;
		b32 normalizedCoords;

		//TODO: Mipmaping
	};

	struct ClearFramebufferAttachment
	{
		u32 attachment;
		ClearValue clearValue;
		b32 color;
		b32 depth;
		b32 stencil;
	};

	struct ClearFramebufferAttachmentsCommand
	{
		ClearFramebufferAttachment clearAttachment[EU_MAX_FRAMEBUFFER_ATTACHMENTS];
		u32 numAttachments;
	};

	class EU_API RenderContext
	{
	public:
		virtual ~RenderContext() {}

		virtual void Init(Display* display) = 0;

		virtual ShaderID LoadShader(const String& name) = 0;
		virtual RenderPassID CreateRenderPass(const RenderPass& renderPass) = 0;
		virtual ShaderBufferID CreateShaderBuffer(ShaderBufferType type, mem_size size, u32 initialMaxUpdatesPerFrame = 1) = 0;
		virtual BufferID CreateBuffer(BufferType type, BufferUsage usage, const void* data, mem_size size) = 0;
		virtual TextureID CreateTexture2D(const String& path) = 0;
		virtual TextureID CreateTexture2D(const u8* pixels, u32 width, u32 heigth, TextureFormat format, b32 isFramebufferAttachment = false, const String& path = "_NoPath_") = 0;
		virtual TextureID CreateTextureHandleForFramebufferAttachment(RenderPassID renderPass, u32 attachment) = 0;
		virtual SamplerID CreateSampler(const Sampler& sampler) = 0;

		virtual void DestroyBuffer(BufferID buffer) = 0;
		virtual void RecreateBuffer(BufferID buffer, BufferType type, BufferUsage usage, const void* data, mem_size size) = 0;

		virtual void DestroyRenderPass(RenderPassID renderPass) = 0;
		virtual void DestroyShader(ShaderID shader) = 0;
		virtual void DestroyTexture(TextureID texture) = 0;

		virtual void AttachShaderBufferToRenderPass(RenderPassID renderPass, ShaderBufferID shaderBuffer, u32 subpass, u32 pipeline, u32 set, u32 binding) = 0;

		virtual void BeginFrame() = 0;
		virtual void BeginRenderPass(const RenderPassBeginInfo& beginInfo) = 0;
		virtual void ClearAttachments(const ClearFramebufferAttachmentsCommand& clearCommand) = 0;
		virtual void ClearStencil(u32 stencil) = 0;
		virtual void SetViewport(const Rect& viewport) = 0;
		virtual void SetScissor(const Rect& scissor) = 0;
		virtual void UpdateShaderBuffer(ShaderBufferID shaderBuffer, const void* data, mem_size size) = 0;
		virtual void UpdateShaderBufferAllFrames(ShaderBufferID shaderBuffer, const void* data, mem_size size) = 0;
		virtual void BindTextureGroup(const TextureGroupBind& groupBind) = 0;
		virtual void SubmitRenderCommand(const RenderCommand& renderCommand) = 0;
		virtual void NextSubpass(u32 initialPipeline = 0) = 0;
		virtual u32 SwitchPipeline(u32 pipeline) = 0;
		virtual void EndRenderPass() = 0;
		virtual void Present() = 0;

		virtual void ReadPixelsIntoBuffer(TextureID texture, BufferID buffer) = 0;

		virtual void ResizeFramebuffer(RenderPassID renderPass, u32 width, u32 height) = 0;

		virtual void* MapBuffer(BufferID buffer) = 0;
		virtual void UnmapBuffer(BufferID buffer) = 0;
		virtual void GetTextureSize(TextureID texture, u32* width, u32* height, u32* depth = 0) = 0;
		virtual void GetFramebufferSize(RenderPassID renderPass, u32* width, u32* height) = 0;

		virtual RenderAPI GetRenderAPI() const = 0;
	public:
		static b32 IsStencilFormat(TextureFormat format);
		static b32 IsDepthFormat(TextureFormat format);
		static b32 IsDepthStencilFormat(TextureFormat format);
		static RenderContext* CreateRenderContext(RenderAPI api);
	};

}
