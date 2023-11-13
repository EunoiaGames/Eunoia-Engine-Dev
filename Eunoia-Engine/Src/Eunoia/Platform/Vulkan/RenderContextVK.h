#pragma once

#include <vulkan\vulkan.h>
#include <shaderc\shaderc.h>
#include "../../../Vendor/SPIRV-Reflect/spirv_reflect.h"

#include "../../Rendering/RenderContext.h"
#include "../../DataStructures/List.h"

#define EU_VK_MAX_FRAMES_IN_FLIGHT 3

namespace Eunoia {

	struct QueueFamilyIndices
	{
		s32 graphics;
		s32 present;
	};

	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		List<VkSurfaceFormatKHR> formats;
		List<VkPresentModeKHR> presentModes;
	};

	struct ShaderParseInfoInputAttachmentVK
	{
		VkShaderStageFlags stage;
		u32 attachmentIndex;
		u32 set;
		u32 binding;
		u32 count;
	};

	struct ShaderParseInfoUniformBufferVK
	{
		String name;
		u32 binding;
		u32 size;
		u32 arrayCount;
		b32 isStorageBuffer;
	};

	struct ShaderParseInfoUniformBufferSetVK
	{
		u32 set;
		VkShaderStageFlags stage;
		List<ShaderParseInfoUniformBufferVK> uniformBuffers;
	};

	struct ShaderParseInfoTextureVK
	{
		String name;
		u32 binding;
		u32 arrayCount;
	};

	struct ShaderParseInfoTextureSetVK
	{
		u32 set;
		VkShaderStageFlags stage;
		List<ShaderParseInfoTextureVK> textures;
	};

	struct ShaderStageParseInfoVK
	{
		List<ShaderParseInfoInputAttachmentVK> inputAttachments;
		List<ShaderParseInfoUniformBufferSetVK> bufferSets;
		List<ShaderParseInfoTextureSetVK> textureSets;
	};

	struct ShaderParseInfoVK
	{
		String name;
		List<ShaderParseInfoInputAttachmentVK> inputAttachments;
		List<ShaderParseInfoUniformBufferSetVK> bufferSets;
		List<ShaderParseInfoTextureSetVK> textureSets;
	};

	struct ShaderLayoutsVK
	{
		List<VkDescriptorSetLayout> bufferDescriptorSetLayouts;
		List<VkDescriptorSetLayout> textureDescriptorSetLayouts;
		VkDescriptorSetLayout inputAttachmentDescriptorSetLayout;
	};

	struct ShaderVK
	{
		VkShaderModule vertex;
		VkShaderModule fragment;

		ShaderParseInfoVK parseInfo;
	};

	struct ShaderTextureGroupVK
	{
		u32 setNumber;
		u32 currentOffset[EU_VK_MAX_FRAMES_IN_FLIGHT];
		List<VkDescriptorSet> descriptorSets[EU_VK_MAX_FRAMES_IN_FLIGHT];
		b32 updated[EU_VK_MAX_FRAMES_IN_FLIGHT];
	};

	struct ShaderTextureResourcesVK
	{
		List<ShaderTextureGroupVK> textureGroups;
	};

	struct BufferVK
	{
		VkBuffer buffer;
		VkDeviceMemory memory;
		mem_size size;
		b32 canBeMapped;
	};

	struct ShaderBufferVK
	{
		BufferVK buffer[EU_VK_MAX_FRAMES_IN_FLIGHT];
		u32 alignment;
		b32 isMapped[EU_VK_MAX_FRAMES_IN_FLIGHT];
		void* mappedData[EU_VK_MAX_FRAMES_IN_FLIGHT];
		mem_size currentOffset[EU_VK_MAX_FRAMES_IN_FLIGHT];
		b32 incrementOffset[EU_VK_MAX_FRAMES_IN_FLIGHT];
		b32 wasUpdated[EU_VK_MAX_FRAMES_IN_FLIGHT];
		mem_size ubSize;
		b32 isDynamic;
		b32 isStorageBuffer;
	};

	struct ShaderBufferSetLinkVK
	{
		String name;
		u32 binding;
		ShaderBufferID buffer;
	};

	struct ShaderBufferSetVK
	{
		u32 setNumber;
		List<u32> dynamicOffsets;
		List<ShaderBufferSetLinkVK> buffers;
	};

	struct ShaderBufferResourcesVK
	{
		List<VkDescriptorSet> descriptorSets[EU_VK_MAX_FRAMES_IN_FLIGHT];
		List<ShaderBufferSetVK> bufferSets;
	};

	struct ShaderInputAttachmentResourcesVK
	{
		VkDescriptorSet descriptorSet[EU_VK_MAX_FRAMES_IN_FLIGHT];
		b32 hasInputAttachments;
		b32 initialized;
		u32 setNumber;
		VkDescriptorPool descriptorPool;

		VkDescriptorSetLayout setLayout; //Used to create on framebuffer resize
		List<ShaderParseInfoInputAttachmentVK> parseInfos; //Used to create on framebuffer resize
		u32 readAttachments[EU_MAX_FRAMEBUFFER_ATTACHMENTS]; //Used to create on framebuffer resize
	};

	struct ShaderResourcesVK
	{
		VkDescriptorPool descriptorPool;
		ShaderBufferResourcesVK bufferResources;
		ShaderTextureResourcesVK textureResources;
		ShaderLayoutsVK layouts;
	};

	struct GraphicsPipelineVK
	{
		ShaderID shader;
		ShaderResourcesVK shaderResources;

		GraphicsPipeline pipelineSettings;
		VkPipelineLayout pipelineLayout;
		VkPipeline pipeline;
	};

	struct FramebufferAttachmentVK
	{
		b32 isSwapchainAttachment;
		VkImage image;
		VkImageView imageView;
		VkDeviceMemory imageMemory;

		TextureFormat format;
		VkImageUsageFlags usage;
		VkImageLayout initialLayout;
		List<TextureID> texturesThatPointToThisAttachment;
	};

	struct SubpassVK
	{
		List<GraphicsPipelineVK> pipelines;
		ShaderInputAttachmentResourcesVK inputAttachmentResources;
	};

	struct RenderPassVK
	{
		VkRenderPass renderPass;
		List<SubpassVK> subpasses;

		VkExtent2D framebufferExtent;
		List<VkFramebuffer> framebuffers;
		FramebufferAttachmentVK attachments[EU_MAX_FRAMEBUFFER_ATTACHMENTS];
		u32 numAttachments;
	};

	struct FrameInFlightVK
	{
		VkSemaphore imageAvailableSemaphore;
		VkSemaphore renderFinishedSemaphore;
		VkFence fence;

		VkCommandBuffer commandBuffer;
	};

	struct TextureVK
	{
		String path;
		VkImage image;
		VkImageView imageView;
		VkDeviceMemory memory;
		TextureFormat format;
		u32 width;
		u32 height;
		u32 depth;
	};

	struct ResizeFramebufferAtEndOfFrameVK
	{
		RenderPassID renderPass;
		u32 width;
		u32 height;
	};

	struct SamplerVK
	{
		VkSampler sampler;
	};

	class EU_API RenderContextVK : public RenderContext
	{
	public:
		RenderContextVK();
		~RenderContextVK();
		virtual void Init(Display* display) override;

		virtual ShaderID LoadShader(const String& name) override;
		virtual RenderPassID CreateRenderPass(const RenderPass& renderPass) override;
		virtual ShaderBufferID CreateShaderBuffer(ShaderBufferType type, mem_size size, u32 initialMaxUpdatesPerFrame = 1) override;
		virtual BufferID CreateBuffer(BufferType type, BufferUsage usage, const void* data, mem_size size) override;
		virtual TextureID CreateTexture2D(const String& path) override;
		virtual TextureID CreateTexture2D(const u8* pixels, u32 width, u32 height, TextureFormat format, b32 isFramebufferAttachment = false, const String& path = "_NoPath_") override;
		virtual TextureID CreateTextureHandleForFramebufferAttachment(RenderPassID renderPass, u32 attachment) override;
		virtual SamplerID CreateSampler(const Sampler& sampler) override;

		virtual void DestroyRenderPass(RenderPassID renderPass) override;
		virtual void DestroyShader(ShaderID shader) override;
		virtual void DestroyTexture(TextureID texture) override;

		virtual void DestroyBuffer(BufferID buffer) override;
		virtual void RecreateBuffer(BufferID buffer, BufferType type, BufferUsage usage, const void* data, mem_size size) override;

		virtual void AttachShaderBufferToRenderPass(RenderPassID renderPass, ShaderBufferID shaderBuffer, u32 subpass, u32 pipeline, u32 set, u32 binding) override;

		virtual void BeginFrame() override;
		virtual void BeginRenderPass(const RenderPassBeginInfo& beginInfo) override;
		virtual void ClearAttachments(const ClearFramebufferAttachmentsCommand& clearCommand) override;
		virtual void ClearStencil(u32 stencil) override;
		virtual void SetViewport(const Rect& viewport) override;
		virtual void SetScissor(const Rect& scissor) override;
		virtual void UpdateShaderBuffer(ShaderBufferID shaderBuffer, const void* data, mem_size size) override;
		virtual void UpdateShaderBufferAllFrames(ShaderBufferID shaderBuffer, const void* data, mem_size size) override;
		virtual void BindTextureGroup(const TextureGroupBind& groupBind) override;
		virtual void SubmitRenderCommand(const RenderCommand& renderCommand) override;
		virtual void NextSubpass(u32 initialPipeline = 0) override;
		virtual u32 SwitchPipeline(u32 pipeline) override;
		virtual void EndRenderPass() override;
		virtual void Present() override;

		virtual void ReadPixelsIntoBuffer(TextureID texture, BufferID buffer) override;

		virtual void ResizeFramebuffer(RenderPassID renderPass, u32 width, u32 height) override;

		virtual void* MapBuffer(BufferID buffer) override;
		virtual void UnmapBuffer(BufferID buffer) override;
		virtual void GetTextureSize(TextureID texture, u32* width, u32* height, u32* depth = 0) override;
		virtual void GetFramebufferSize(RenderPassID renderPass, u32* width, u32* height) override;

		virtual RenderAPI GetRenderAPI() const override;
	private:
		void InitInstance();
		b32 CheckRequiredExtensionSupport(const List<const char*>& requiredExtensions);
		b32 CheckValidationLayerSupport();
		void InitDebugUtilsMessenger();
		void ChoosePhysicalDevice();
		b32 IsGPUSuitable(VkPhysicalDevice physicalDevice);
		QueueFamilyIndices FindQueueFamilyIndices(VkPhysicalDevice physicalDevice);
		b32 AreQueueFamilyIndicesValid(const QueueFamilyIndices& indices);
		void InitLogicalDevice();
		void InitSurface(Display* display);
		b32 CheckDeviceExtensionSuppport(VkPhysicalDevice physicalDevice);
		SwapchainSupportDetails GetSwapchainSupportDetails(VkPhysicalDevice physicalDevice);
		VkSurfaceFormatKHR ChooseSwapchainSurfaceFormat(const SwapchainSupportDetails& details);
		VkPresentModeKHR ChooseSwapchainPresentMode(const SwapchainSupportDetails& details);
		VkExtent2D ChooseSwapchainExtent(const SwapchainSupportDetails& details, u32 width, u32 height);
		void InitSwapchain();
		void InitSwapchainImageViews();
		void ParseShader(const char* spirv, u32 size, ShaderParseInfoVK* parseInfo);
		void InitShaderLayouts(ShaderLayoutsVK* layouts, const ShaderParseInfoVK& parseInfo, const List<String>& dynamicBuffers);
		void InitGraphicsPipeline(GraphicsPipelineVK* pipeline, const GraphicsPipeline& pipelineSettings, RenderPassVK* renderPass, u32 subpass);
		void InitRenderPass(RenderPassVK* renderPass, const RenderPass& renderPassSettings);
		void InitFramebuffer(RenderPassVK* renderPass, const Framebuffer& framebufferSettings);
		void InitCommandPool();
		void InitFramesInFlight();
		void InitShaderResources(RenderPassVK* renderPass, ShaderResourcesVK* resources, ShaderInputAttachmentResourcesVK* inputAttachmentResources,
			const ShaderParseInfoVK& parseInfo, const u32* readAttachments, const List<MaxTextureGroupBinds>& maxBinds, const List<String>& dynamicBuffers);
		void InitDefaultTextureAndSampler();

		void DoFramebufferResize(const ResizeFramebufferAtEndOfFrameVK& resize);

		void DestroySwapchainResources();
		void RecreateSwapchainResources(u32 width, u32 height);

		void CreateTexture2DHelper(const u8* pixels, u32 width, u32 height, VkFormat format, TextureVK* texture, b32 isFramebufferAttachment);
		void CreateImage(u32 width, u32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usageFlags, VkImageLayout initialLayout, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* memory);
		s32 FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags memoryProperties);
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* memory, mem_size* actualSize);

		VkCommandBuffer BeginCommands() const;
		void EndCommands(VkCommandBuffer commandBuffer) const;
		void CmdCopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size) const;
		void CmdTransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CmdCopyBufferToImage(VkBuffer src, VkImage dst, u32 width, u32 height);

		VkFormat GetVkFormat(TextureFormat format);
		u32 GetPixelSizeFromFormat(VkFormat format);
	private:
		static void VulkanDisplayCallback(const DisplayEvent& e, void* userPtr);

		static VkPrimitiveTopology GetVkPrimitiveTopology(PrimitiveTopology topology);
		static VkCullModeFlags GetVkCullMode(CullMode cullMode);
		static VkPolygonMode GetVkPolygonMode(PolygonMode polygonMode);
		static VkFrontFace GetVkFrontFace(FrontFace frontFace);
		static VkCompareOp GetVkCompareOp(CompareOperation compareOperation);
		static VkStencilOp GetVkStencilOp(StencilOperation stencilOperation);
		static void GetVkStencilOpState(VkStencilOpState* vkState, const StencilState& state);
		static VkBlendFactor GetVkBlendFactor(BlendFactor blendFactor);
		static VkBlendOp GetVkBlendOp(BlendOperation blendOperation);
		static VkDynamicState GetVkDynamicState(DynamicState dynamicState);
		static VkBufferUsageFlags GetVkBufferUsage(BufferType type);
		static VkFilter GetVkFilter(TextureFilter filter);
		static VkSamplerAddressMode GetVkSamplerAddressMode(TextureAddressMode addressMode);
		static VkBorderColor GetVkBorderColor(TextureBorderColor borderColor);

		friend VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);
	private:
		VkInstance										m_Instance;
		VkDebugUtilsMessengerEXT						m_DebugUtilsMessenger;
		VkPhysicalDevice								m_PhysicalDevice;
		VkDevice										m_Device;
		VkQueue											m_GraphicsQueue;
		VkQueue											m_PresentQueue;
		VkSurfaceKHR									m_Surface;
		VkSwapchainKHR									m_Swapchain;
		VkFormat										m_SwapchainFormat;
		VkExtent2D										m_SwapchainExtent;
		List<VkImage>									m_SwapchainImages;
		List<VkImageView>								m_SwapchainImageViews;
		VkCommandPool									m_CommandPool;
		FrameInFlightVK									m_FramesInFlight[EU_VK_MAX_FRAMES_IN_FLIGHT];
		u32												m_CurrentFrame;
		u32												m_ImageIndex;
		RenderPassID									m_CurrentRenderPass;
		u32												m_CurrentSubpass;
		u32												m_CurrentPipeline;
		TextureVK										m_DefaultTexture;
		SamplerVK										m_DefaultSampler;
		List<ResizeFramebufferAtEndOfFrameVK>			m_ResizeFramebuffers;
	private:
		List<ShaderVK>									m_Shaders;
		List<RenderPassVK>								m_RenderPasses;
		List<BufferVK>									m_Buffers;
		List<ShaderBufferVK>							m_ShaderBuffers;
		List<TextureVK>									m_Textures;
		List<SamplerVK>									m_Samplers;
	private:
		List<BufferID>									m_FreeBufferIDs;
		List<RenderPassID>								m_FreeRenderPassIDs;
		List<ShaderID>									m_FreeShaderIDs;
		List<TextureID>									m_FreeTextureIDs;
	private:
		static const List<const char*> s_ValidationLayers;
		static const List<const char*> s_RequiredDeviceExtensions;
	};

}
