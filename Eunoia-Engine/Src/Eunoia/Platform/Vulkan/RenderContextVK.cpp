#include "RenderContextVK.h"
#include "../../DataStructures/String.h"
#include "../../Utils/Log.h"
#include "../../Utils/FileUtils.h"
#include "../../Core/Engine.h"
#include "../../Rendering/Asset/TextureLoader.h"

#ifdef EU_PLATFORM_WINDOWS
#include <vulkan\vulkan_win32.h>
#include "../Win32/DisplayWin32.h"
#endif

#ifdef EU_DEBUG
#define EU_CHECK_VKRESULT(call, logMsg, ret) if(call != VK_SUCCESS) { EU_LOG_ERROR(logMsg); return ret; }
#else
#define EU_CHECK_VKRESULT(call, logMsg) call
#endif

#ifdef EU_DEBUG || defined(EU_ENABLE_VK_VALIDATION_LAYERS_NON_DEBUG_PROFILE)
#define EU_ENABLE_VK_VALIDATION_LAYERS
#endif

namespace Eunoia {

	const List<const char*> RenderContextVK::s_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
	const List<const char*> RenderContextVK::s_RequiredDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	RenderContextVK::RenderContextVK() :
		m_CurrentFrame(0),
		m_ImageIndex(0),
		m_CurrentRenderPass(EU_INVALID_RENDER_PASS_ID),
		m_CurrentSubpass(0),
		m_CurrentPipeline(0)
	{}

	RenderContextVK::~RenderContextVK()
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func)
			func(m_Instance, m_DebugUtilsMessenger, 0);

		for (u32 i = 0; i < m_Shaders.Size(); i++)
		{
			vkDestroyShaderModule(m_Device, m_Shaders[i].vertex, 0);
			vkDestroyShaderModule(m_Device, m_Shaders[i].fragment, 0);
		}

		for (u32 i = 0; i < m_RenderPasses.Size(); i++)
		{
			RenderPassVK* renderPass = &m_RenderPasses[i];

			for (u32 j = 0; j < renderPass->subpasses.Size(); j++)
			{
				SubpassVK* subpass = &renderPass->subpasses[j];
				
				for (u32 k = 0; k < subpass->pipelines.Size(); k++)
				{
					GraphicsPipelineVK* pipeline = &subpass->pipelines[j];

					vkDestroyPipelineLayout(m_Device, pipeline->pipelineLayout, 0);
					vkDestroyPipeline(m_Device, pipeline->pipeline, 0);
				}
			}

			vkDestroyRenderPass(m_Device, renderPass->renderPass, 0);
		}

		for (u32 i = 0; i < EU_VK_MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(m_Device, m_FramesInFlight[i].imageAvailableSemaphore, 0);
			vkDestroySemaphore(m_Device, m_FramesInFlight[i].renderFinishedSemaphore, 0);
			vkDestroyFence(m_Device, m_FramesInFlight[i].fence, 0);
		}
	
		vkDestroyCommandPool(m_Device, m_CommandPool, 0);
		vkDestroySurfaceKHR(m_Instance, m_Surface, 0);
		vkDestroyDevice(m_Device, 0);
		vkDestroyInstance(m_Instance, 0);
	}

	void RenderContextVK::Init(Display* display)
	{
		InitInstance();
		InitDebugUtilsMessenger();
		InitSurface(display);
		ChoosePhysicalDevice();
		InitLogicalDevice();
		InitSwapchain();
		InitSwapchainImageViews();
		InitCommandPool();
		InitFramesInFlight();
		InitDefaultTextureAndSampler();

		Engine::GetDisplay()->AddDisplayEventCallback(VulkanDisplayCallback, this);

		EU_LOG_INFO("Created Vulkan render context");
	}

	ShaderID RenderContextVK::LoadShader(const String& name)
	{
		ShaderVK shader;
		
		String vertexPath = "Res/Shaders/GLSL-450/" + name + ".vert.spirv";
		String fragmentPath = "Res/Shaders/GLSL-450/" + name + ".frag.spirv";
		mem_size vertexLength, fragmentLength;
		u8* vertexBytes = FileUtils::LoadBinaryFile(vertexPath, &vertexLength);
		u8* fragmentBytes = FileUtils::LoadBinaryFile(fragmentPath, &fragmentLength);

		if (!vertexBytes || !fragmentBytes)
			return EU_INVALID_SHADER_ID;

		VkShaderModuleCreateInfo shader_module_create_info{};
		shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_module_create_info.pCode = (u32*)vertexBytes;
		shader_module_create_info.codeSize = vertexLength;
		EU_CHECK_VKRESULT(vkCreateShaderModule(m_Device, &shader_module_create_info, 0, &shader.vertex), "Could not create Vulkan vertex shader module", EU_INVALID_SHADER_ID);

		shader_module_create_info.pCode = (u32*)fragmentBytes;
		shader_module_create_info.codeSize = fragmentLength;

		EU_CHECK_VKRESULT(vkCreateShaderModule(m_Device, &shader_module_create_info, 0, &shader.fragment), "Could not create Vulkan fragment shader module", EU_INVALID_SHADER_ID);

		shader.parseInfo.name = name;

		ParseShader((char*)vertexBytes, vertexLength, &shader.parseInfo);
		ParseShader((char*)fragmentBytes, fragmentLength, &shader.parseInfo);

		String logMsg = "Compiled and parsed Vulkan shader modules (" + name + ")";
		EU_LOG_INFO(logMsg.C_Str());

		if (!m_FreeShaderIDs.Empty())
		{
			ShaderID id = m_FreeShaderIDs.GetLastElement();
			m_FreeShaderIDs.Pop();
			m_Shaders[id - 1] = shader;
			return id;
		}

		m_Shaders.Push(shader);
		return m_Shaders.Size();
	}

	RenderPassID RenderContextVK::CreateRenderPass(const RenderPass& renderPassSettings)
	{
		RenderPassVK renderPass;
		renderPass.subpasses.SetCapacityAndElementCount(renderPassSettings.subpasses.Size());

		InitRenderPass(&renderPass, renderPassSettings);
		InitFramebuffer(&renderPass, renderPassSettings.framebuffer);

		for (u32 i = 0; i < renderPass.subpasses.Size(); i++)
		{
			SubpassVK* subpass = &renderPass.subpasses[i];
			subpass->inputAttachmentResources.initialized = false;
			const Subpass& subpassSettings = renderPassSettings.subpasses[i];
			renderPass.subpasses[i].pipelines.SetCapacityAndElementCount(subpassSettings.pipelines.Size());

			for (u32 j = 0; j < subpassSettings.pipelines.Size(); j++)
			{
				const GraphicsPipeline& pipelineSettings = subpassSettings.pipelines[j];
				GraphicsPipelineVK* pipeline = &subpass->pipelines[j];
				pipeline->shader = pipelineSettings.shader;

				const ShaderVK& shader = m_Shaders[pipeline->shader - 1];

				InitShaderLayouts(&pipeline->shaderResources.layouts, shader.parseInfo, pipelineSettings.dynamicBuffers);
				InitShaderResources(&renderPass, &pipeline->shaderResources, &subpass->inputAttachmentResources,
					m_Shaders[pipeline->shader - 1].parseInfo, subpassSettings.readAttachments, pipelineSettings.maxTextureGroupBinds, pipelineSettings.dynamicBuffers);
			}

			for (u32 j = 0; j < subpassSettings.pipelines.Size(); j++)
			{
				const GraphicsPipeline& pipelineSettings = subpassSettings.pipelines[j];
				InitGraphicsPipeline(&renderPass.subpasses[i].pipelines[j], pipelineSettings, &renderPass, i);
			}
		}

		if (!m_FreeRenderPassIDs.Empty())
		{
			RenderPassID id = m_FreeRenderPassIDs.GetLastElement();
			m_FreeRenderPassIDs.Pop();

			m_RenderPasses[id - 1] = renderPass;
			EU_LOG_TRACE("Created Vulkan render pass");
			return id;
		}

		m_RenderPasses.Push(renderPass);
		EU_LOG_TRACE("Created Vulkan render pass");

		if (m_RenderPasses.Size() == 7)
		{
			u32 bp = 0;
		}

		return m_RenderPasses.Size();
	}

	ShaderBufferID RenderContextVK::CreateShaderBuffer(ShaderBufferType type, mem_size size, u32 initialMaxUpdatesPerFrame)
	{
		VkPhysicalDeviceProperties physical_device_properties;
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &physical_device_properties);
		u32 bufferAlignment = physical_device_properties.limits.minUniformBufferOffsetAlignment;

		ShaderBufferVK shaderBuffer;
		shaderBuffer.isStorageBuffer = type == SHADER_BUFFER_STORAGE_BUFFER;
		shaderBuffer.isDynamic = initialMaxUpdatesPerFrame > 1;
		memset(shaderBuffer.incrementOffset, false, sizeof(b32) * EU_VK_MAX_FRAMES_IN_FLIGHT);

		if (shaderBuffer.isStorageBuffer)
			bufferAlignment = physical_device_properties.limits.minStorageBufferOffsetAlignment;

		u32 elementSize = size + bufferAlignment - size % bufferAlignment;
		u32 bufferSize = elementSize * initialMaxUpdatesPerFrame;

		shaderBuffer.alignment = elementSize;
		shaderBuffer.ubSize = bufferSize;
		for (u32 i = 0; i < EU_VK_MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkBufferUsageFlags bufferUsage = shaderBuffer.isStorageBuffer ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

			CreateBuffer(bufferSize, bufferUsage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &shaderBuffer.buffer[i].buffer, &shaderBuffer.buffer[i].memory, &shaderBuffer.buffer[i].size);

			shaderBuffer.buffer[i].canBeMapped = true;
			shaderBuffer.currentOffset[i] = 0;
			shaderBuffer.isMapped[i] = false;
			shaderBuffer.wasUpdated[i] = false;
		}

		m_ShaderBuffers.Push(shaderBuffer);
		return m_ShaderBuffers.Size();
	}

	BufferID RenderContextVK::CreateBuffer(BufferType type, BufferUsage usage, const void* data, mem_size size)
	{
		BufferVK buffer;

		VkBufferUsageFlags buffer_usage_flags;

		if (usage == BUFFER_USAGE_STATIC)
		{
			buffer_usage_flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			buffer.canBeMapped = false;
		}
		else if (usage == BUFFER_USAGE_DYNAMIC)
		{
			buffer_usage_flags = GetVkBufferUsage(type);
			buffer.canBeMapped = true;
		}

		CreateBuffer(size, buffer_usage_flags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer.buffer, &buffer.memory, &buffer.size);

		if (data)
		{
			void* mappedData;
			vkMapMemory(m_Device, buffer.memory, 0, buffer.size, 0, &mappedData);
			memcpy(mappedData, data, size);
			vkUnmapMemory(m_Device, buffer.memory);
		}

		if (usage == BUFFER_USAGE_DYNAMIC)
		{
			if (!m_FreeBufferIDs.Empty())
			{
				BufferID bid = m_FreeBufferIDs.GetLastElement();
				m_FreeBufferIDs.Pop();
				m_Buffers[bid - 1] = buffer;
				return bid;
			}
		
			m_Buffers.Push(buffer);
			EU_LOG_TRACE("Created Vulkan buffer");
			return m_Buffers.Size();
		}

		BufferVK localMemoryBuffer;
		CreateBuffer(size, GetVkBufferUsage(type) | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&localMemoryBuffer.buffer, &localMemoryBuffer.memory, &localMemoryBuffer.size);

		CmdCopyBuffer(buffer.buffer, localMemoryBuffer.buffer, size);

		vkFreeMemory(m_Device, buffer.memory, 0);
		vkDestroyBuffer(m_Device, buffer.buffer, 0);

		EU_LOG_TRACE("Created Vulkan buffer");

		if (!m_FreeBufferIDs.Empty())
		{
			BufferID bid = m_FreeBufferIDs.GetLastElement();
			m_FreeBufferIDs.Pop();
			m_Buffers[bid - 1] = buffer;
			return bid;
		}

		m_Buffers.Push(localMemoryBuffer);
		return m_Buffers.Size();
	}

	TextureID RenderContextVK::CreateTexture2D(const String& path)
	{
		const String& p = path;
		u32 width, height;
		u8* pixels = TextureLoader::LoadEutexTexture(p, &width, &height);

		if (!pixels)
			return EU_INVALID_TEXTURE_ID;

		TextureID tid = CreateTexture2D(pixels, width, height, TEXTURE_FORMAT_RGBA8_UNORM, false, p);
		TextureLoader::FreeEutexTexture(pixels);
		return tid;
	}

	TextureID RenderContextVK::CreateTexture2D(const u8* pixels, u32 width, u32 height, TextureFormat format, b32 isFramebufferAttachment, const String& path)
	{
		TextureVK texture;
		texture.width = width;
		texture.height = height;
		texture.depth = 1;
		texture.path = path;
		texture.format = format;

		VkFormat formatVK = GetVkFormat(format);
		CreateTexture2DHelper(pixels, width, height, formatVK, &texture, isFramebufferAttachment);

		VkImageViewCreateInfo image_view_create_info = {};
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.image = texture.image;
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format = formatVK;
		image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_create_info.subresourceRange.baseMipLevel = 0;
		image_view_create_info.subresourceRange.levelCount = 1;
		image_view_create_info.subresourceRange.baseArrayLayer = 0;
		image_view_create_info.subresourceRange.layerCount = 1;

		EU_CHECK_VKRESULT(vkCreateImageView(m_Device, &image_view_create_info, 0, &texture.imageView), "Could not create Vulkan image view", EU_INVALID_TEXTURE_ID);

		if (!m_FreeTextureIDs.Empty())
		{
			TextureID id = m_FreeTextureIDs.GetLastElement();
			m_FreeTextureIDs.Pop();
			m_Textures[id - 1] = texture;
			return id;
		}

		m_Textures.Push(texture);
		return m_Textures.Size();
	}

	TextureID RenderContextVK::CreateTextureHandleForFramebufferAttachment(RenderPassID renderPass, u32 attachment)
	{
		RenderPassVK& renderPassVK = m_RenderPasses[renderPass - 1];

		TextureVK texture;
		texture.width = renderPassVK.framebufferExtent.width;
		texture.height = renderPassVK.framebufferExtent.height;
		texture.depth = 1;
		texture.path = "_NoPath_";
		texture.format = renderPassVK.attachments[attachment].format;
		texture.image = renderPassVK.attachments[attachment].image;
		texture.imageView = renderPassVK.attachments[attachment].imageView;
		texture.memory = renderPassVK.attachments[attachment].imageMemory;
		
		m_Textures.Push(texture);
		renderPassVK.attachments[attachment].texturesThatPointToThisAttachment.Push(m_Textures.Size());

		return m_Textures.Size();
	}

	SamplerID RenderContextVK::CreateSampler(const Sampler& sampler)
	{
		VkSamplerCreateInfo sampler_create_info { };
		sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_create_info.addressModeU = GetVkSamplerAddressMode(sampler.addressModes[0]);
		sampler_create_info.addressModeV = GetVkSamplerAddressMode(sampler.addressModes[1]);
		sampler_create_info.addressModeW = GetVkSamplerAddressMode(sampler.addressModes[2]);
		sampler_create_info.anisotropyEnable = sampler.anisotropyEnabled;
		sampler_create_info.borderColor = GetVkBorderColor(sampler.borderColor);
		sampler_create_info.compareEnable = VK_FALSE;
		sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
		sampler_create_info.minFilter = GetVkFilter(sampler.minFilter);
		sampler_create_info.magFilter = GetVkFilter(sampler.magFilter);
		sampler_create_info.unnormalizedCoordinates = !sampler.normalizedCoords;
		sampler_create_info.maxAnisotropy = sampler.maxAnisotropy;
		sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_create_info.mipLodBias = 0.0f;
		sampler_create_info.minLod = 0.0f;
		sampler_create_info.maxLod = 0.0f;

		SamplerVK samplerVK;
		EU_CHECK_VKRESULT(vkCreateSampler(m_Device, &sampler_create_info, 0, &samplerVK.sampler), "Could not create Vulkan sampler", EU_INVALID_SAMPLER_ID);

		m_Samplers.Push(samplerVK);
		return m_Samplers.Size();
	}

	void RenderContextVK::DestroyRenderPass(RenderPassID renderPass)
	{
		RenderPassVK* renderPassVK = &m_RenderPasses[renderPass - 1];

		vkDestroyRenderPass(m_Device, renderPassVK->renderPass, 0);
		for (u32 i = 0; i < renderPassVK->framebuffers.Size(); i++)
			vkDestroyFramebuffer(m_Device, renderPassVK->framebuffers[i], 0);

		for (u32 i = 0; i < renderPassVK->subpasses.Size(); i++)
		{
			SubpassVK* subpass = &renderPassVK->subpasses[i];
			ShaderInputAttachmentResourcesVK* resources = &subpass->inputAttachmentResources;
			if (resources->hasInputAttachments)
			{
				vkDestroyDescriptorPool(m_Device, resources->descriptorPool, 0);
				vkDestroyDescriptorSetLayout(m_Device, resources->setLayout, 0);
			}

			for (u32 j = 0; j < subpass->pipelines.Size(); j++)
			{
				GraphicsPipelineVK* pipeline = &subpass->pipelines[j];
				vkDestroyPipelineLayout(m_Device, pipeline->pipelineLayout, 0);
				vkDestroyPipeline(m_Device, pipeline->pipeline, 0);
				vkDestroyDescriptorPool(m_Device, pipeline->shaderResources.descriptorPool, 0);

				for (u32 k = 0; k < pipeline->shaderResources.layouts.bufferDescriptorSetLayouts.Size(); k++)
					vkDestroyDescriptorSetLayout(m_Device, pipeline->shaderResources.layouts.bufferDescriptorSetLayouts[k], 0);

				for (u32 k = 0; k < pipeline->shaderResources.layouts.textureDescriptorSetLayouts.Size(); k++)
					vkDestroyDescriptorSetLayout(m_Device, pipeline->shaderResources.layouts.textureDescriptorSetLayouts[k], 0);
			}
		}

		m_FreeRenderPassIDs.Push(renderPass);
	}

	void RenderContextVK::DestroyShader(ShaderID shaderID)
	{
		ShaderVK* shader = &m_Shaders[shaderID - 1];
		vkDestroyShaderModule(m_Device, shader->vertex, 0);
		vkDestroyShaderModule(m_Device, shader->fragment, 0);

		m_FreeShaderIDs.Push(shaderID);
	}

	void RenderContextVK::DestroyTexture(TextureID textureID)
	{
		TextureVK* texture = &m_Textures[textureID - 1];
		vkDestroyImage(m_Device, texture->image, 0);
		vkDestroyImageView(m_Device, texture->imageView, 0);
		vkFreeMemory(m_Device, texture->memory, 0);

		m_FreeTextureIDs.Push(textureID);
	}

	void RenderContextVK::DestroyBuffer(BufferID buffer)
	{
		BufferVK* buf = &m_Buffers[buffer - 1];
		vkFreeMemory(m_Device, buf->memory, 0);
		vkDestroyBuffer(m_Device, buf->buffer, 0);

		m_FreeBufferIDs.Push(buffer);
	}

	void RenderContextVK::RecreateBuffer(BufferID buffer, BufferType type, BufferUsage usage, const void* data, mem_size size)
	{
		DestroyBuffer(buffer);
		BufferID newID = CreateBuffer(type, usage, data, size);
		if (newID != buffer)
			EU_LOG_WARN("RecreateBuffer() id was somehow modified");
	}

	void RenderContextVK::AttachShaderBufferToRenderPass(RenderPassID renderPass, ShaderBufferID shaderBuffer, u32 subpass, u32 pipeline, u32 set, u32 binding)
	{
		GraphicsPipelineVK& pipelineVK = m_RenderPasses[renderPass - 1].subpasses[subpass].pipelines[pipeline];
		ShaderBufferResourcesVK& bufferResources = pipelineVK.shaderResources.bufferResources;
		ShaderBufferVK& shaderBufferVK = m_ShaderBuffers[shaderBuffer - 1];

		VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		if (shaderBufferVK.isDynamic)
			descriptorType = shaderBufferVK.isStorageBuffer ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		else
			descriptorType = shaderBufferVK.isStorageBuffer ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		VkWriteDescriptorSet write_descriptor_set{};
		write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_descriptor_set.descriptorCount = 1;
		write_descriptor_set.descriptorType = descriptorType;
		write_descriptor_set.dstArrayElement = 0; //TODO
		write_descriptor_set.dstBinding = binding;

		s32 setIndex = -1;
		for (u32 i = 0; i < bufferResources.bufferSets.Size(); i++)
		{
			if (bufferResources.bufferSets[i].setNumber == set)
			{
				setIndex = i;
				break;
			}
		}

		if (setIndex == -1)
		{
			EU_LOG_ERROR("Tried to attach shader buffer to an invalid descriptor set");
			return;
		}

		for (u32 i = 0; i < bufferResources.bufferSets[setIndex].buffers.Size(); i++)
		{
			ShaderBufferSetLinkVK* buffer = &bufferResources.bufferSets[setIndex].buffers[i];
			if (buffer->binding == binding)
			{
				if (buffer->buffer != EU_INVALID_UNIFORM_BUFFER_ID)
				{
					EU_LOG_WARN("A shader buffer is already attached to this descriptor set.\nDeattaching previous one");
					//TODO:
				}

				buffer->buffer = shaderBuffer;
			}
		}

		VkDescriptorBufferInfo descriptor_buffer_info{};
		descriptor_buffer_info.offset = 0;
		descriptor_buffer_info.range = shaderBufferVK.alignment;

		VkWriteDescriptorSet write_descriptor_sets[EU_VK_MAX_FRAMES_IN_FLIGHT];
		VkDescriptorBufferInfo descriptor_buffer_infos[EU_VK_MAX_FRAMES_IN_FLIGHT];
		for (u32 i = 0; i < EU_VK_MAX_FRAMES_IN_FLIGHT; i++)
		{
			write_descriptor_set.dstSet = bufferResources.descriptorSets[i][setIndex];
			descriptor_buffer_info.buffer = shaderBufferVK.buffer[i].buffer;

			write_descriptor_sets[i] = write_descriptor_set;
			descriptor_buffer_infos[i] = descriptor_buffer_info;
		}

		for (u32 i = 0; i < EU_VK_MAX_FRAMES_IN_FLIGHT; i++)
			write_descriptor_sets[i].pBufferInfo = &descriptor_buffer_infos[i];

		vkUpdateDescriptorSets(m_Device, EU_VK_MAX_FRAMES_IN_FLIGHT, write_descriptor_sets, 0, 0);
	}

	void RenderContextVK::BeginFrame()
	{
		if (Engine::GetDisplay()->IsMinimized())
			return;

		m_CurrentRenderPass = 0;
		m_CurrentSubpass = 0;

		const FrameInFlightVK& frame = m_FramesInFlight[m_CurrentFrame];

		EU_CHECK_VKRESULT(vkAcquireNextImageKHR(m_Device, m_Swapchain, EU_U64_MAX, frame.imageAvailableSemaphore, 0, &m_ImageIndex), "Could not aquire next Vulkan swapchain image");
		EU_CHECK_VKRESULT(vkWaitForFences(m_Device, 1, &frame.fence, true, EU_U64_MAX), "Error wating for Vulkan fence");
		EU_CHECK_VKRESULT(vkResetFences(m_Device, 1, &frame.fence), "Could not reset Vulkan fence");

		VkCommandBufferBeginInfo command_buffer_begin_info{};
		command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		EU_CHECK_VKRESULT(vkBeginCommandBuffer(frame.commandBuffer, &command_buffer_begin_info), "Could not begin Vulkan command buffer recording");
	}

	void RenderContextVK::BeginRenderPass(const RenderPassBeginInfo& beginInfo)
	{
		if (Engine::GetDisplay()->IsMinimized())
			return;

		RenderPassVK* renderPass = &m_RenderPasses[beginInfo.renderPass - 1];
		m_CurrentRenderPass = beginInfo.renderPass;
		m_CurrentSubpass = 0;
		m_CurrentPipeline = beginInfo.initialPipeline;

		SubpassVK* subpass = &renderPass->subpasses[0];

		//TODO: Reset offsets

		const FrameInFlightVK& frame = m_FramesInFlight[m_CurrentFrame];
		ShaderResourcesVK* resources = &subpass->pipelines[m_CurrentPipeline].shaderResources;

		VkClearValue clearValues[EU_MAX_FRAMEBUFFER_ATTACHMENTS];
		for (u32 i = 0; i < beginInfo.numClearValues; i++)
		{
			if (beginInfo.clearValues[i].clearDepthStencil)
			{
				clearValues[i].depthStencil.depth = beginInfo.clearValues[i].depth;
				clearValues[i].depthStencil.stencil = beginInfo.clearValues[i].stencil;
			}
			else
			{
				memcpy(clearValues[i].color.float32, &beginInfo.clearValues[i].color, sizeof(r32) * 4);
			}
		}

		VkRenderPassBeginInfo render_pass_begin_info{};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = renderPass->renderPass;
		render_pass_begin_info.clearValueCount = beginInfo.numClearValues;
		render_pass_begin_info.pClearValues = clearValues;
		render_pass_begin_info.renderArea.offset = { 0, 0 };
		render_pass_begin_info.renderArea.extent = renderPass->framebufferExtent;

		if (renderPass->framebuffers.Size() > 1)
			render_pass_begin_info.framebuffer = renderPass->framebuffers[m_ImageIndex];
		else
			render_pass_begin_info.framebuffer = renderPass->framebuffers[0];

		vkCmdBeginRenderPass(frame.commandBuffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, subpass->pipelines[m_CurrentPipeline].pipeline);

		if (subpass->inputAttachmentResources.hasInputAttachments)
		{
			vkCmdBindDescriptorSets(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, subpass->pipelines[m_CurrentPipeline].pipelineLayout,
				subpass->inputAttachmentResources.setNumber, 1, &subpass->inputAttachmentResources.descriptorSet[m_CurrentFrame], 0, 0);
		}
	}

	void RenderContextVK::ClearAttachments(const ClearFramebufferAttachmentsCommand& clearCommand)
	{
		if (Engine::GetDisplay()->IsMinimized())
			return;

		EU_PERSISTENT List<VkClearAttachment> clear_attachments;
		clear_attachments.Clear();

		for (u32 i = 0; i < clearCommand.numAttachments; i++)
		{
			VkClearAttachment clear_attachment;
			if (clearCommand.clearAttachment[i].color)
			{
				clear_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				clear_attachment.colorAttachment = clearCommand.clearAttachment[i].attachment;
				memcpy(clear_attachment.clearValue.color.float32, &clearCommand.clearAttachment[i].clearValue.color, sizeof(r32) * 4);
			}
			else
			{
				clear_attachment.colorAttachment = 0;
				clear_attachment.aspectMask = clearCommand.clearAttachment[i].depth ? VK_IMAGE_ASPECT_DEPTH_BIT : 0;
				if (clearCommand.clearAttachment[i].stencil)
					clear_attachment.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
				clear_attachment.clearValue.depthStencil.depth = clearCommand.clearAttachment[i].clearValue.depth;
				clear_attachment.clearValue.depthStencil.stencil = clearCommand.clearAttachment[i].clearValue.stencil;
			}

			clear_attachments.Push(clear_attachment);
		}

		VkClearRect clear_rect;
		clear_rect.baseArrayLayer = 0;
		clear_rect.layerCount = 1;
		clear_rect.rect.offset = { 0, 0 };
		clear_rect.rect.extent = m_RenderPasses[m_CurrentRenderPass - 1].framebufferExtent;

		vkCmdClearAttachments(m_FramesInFlight[m_CurrentFrame].commandBuffer, clearCommand.numAttachments, &clear_attachments[0], 1, &clear_rect);
	}

	void RenderContextVK::ClearStencil(u32 stencil)
	{
		VkCommandBuffer commandBuffer = m_FramesInFlight[m_CurrentFrame].commandBuffer;

		VkClearValue clear_value = {};
		clear_value.depthStencil.depth = 0.0f;
		clear_value.depthStencil.stencil = stencil;

		VkClearAttachment clear_attachment = {};
		clear_attachment.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
		clear_attachment.clearValue = clear_value;

		VkClearRect clear_rect = {};
		clear_rect.baseArrayLayer = 0;
		clear_rect.layerCount = 1;
		clear_rect.rect.offset = { 0, 0 };
		clear_rect.rect.extent = m_RenderPasses[m_CurrentRenderPass - 1].framebufferExtent;

		vkCmdClearAttachments(commandBuffer, 1, &clear_attachment, 1, &clear_rect);
	}

	void RenderContextVK::SetViewport(const Rect& viewport)
	{
		VkCommandBuffer commandBuffer = m_FramesInFlight[m_CurrentFrame].commandBuffer;

		VkViewport vp;
		vp.x = viewport.x;
		vp.y = viewport.y;
		vp.width = viewport.width;
		vp.height = viewport.height;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;

		vkCmdSetViewport(commandBuffer, 0, 1, &vp);
	}

	void RenderContextVK::SetScissor(const Rect& scissor)
	{
		VkCommandBuffer commandBuffer = m_FramesInFlight[m_CurrentFrame].commandBuffer;

		VkRect2D s;
		s.offset.x = scissor.x;
		s.offset.y = scissor.y;
		s.extent.width = scissor.width;
		s.extent.height = scissor.height;

		vkCmdSetScissor(commandBuffer, 0, 1, &s);
	}

	void RenderContextVK::UpdateShaderBuffer(ShaderBufferID shaderBuffer, const void* data, mem_size size)
	{
		if (Engine::GetDisplay()->IsMinimized())
			return;

		ShaderBufferVK& shaderBufferVK = m_ShaderBuffers[shaderBuffer - 1];

		if (shaderBufferVK.incrementOffset[m_CurrentFrame])
		{
			shaderBufferVK.currentOffset[m_CurrentFrame] = (shaderBufferVK.currentOffset[m_CurrentFrame] + shaderBufferVK.alignment) % shaderBufferVK.ubSize;
			shaderBufferVK.incrementOffset[m_CurrentFrame] = false;
		}

		if (!shaderBufferVK.isDynamic && shaderBufferVK.wasUpdated[m_CurrentFrame])
		{
			//EU_LOG_WARN("Trying to updating a nondynamic Vulkan uniform buffer more than once per frame");
			return;
		}

		if (!shaderBufferVK.isMapped[m_CurrentFrame])
		{
			EU_CHECK_VKRESULT(vkMapMemory(m_Device, shaderBufferVK.buffer[m_CurrentFrame].memory, 0, size, 0, &shaderBufferVK.mappedData[m_CurrentFrame]),
				"Could not map Vulkan uniform buffer");

			shaderBufferVK.isMapped[m_CurrentFrame] = true;
		}

		memcpy((u8*)shaderBufferVK.mappedData[m_CurrentFrame] + shaderBufferVK.currentOffset[m_CurrentFrame], data, size);

		shaderBufferVK.wasUpdated[m_CurrentFrame] = true;
	}

	void RenderContextVK::UpdateShaderBufferAllFrames(ShaderBufferID shaderBuffer, const void* data, mem_size size)
	{
		u32 currentFrame = m_CurrentFrame;
		for (m_CurrentFrame = 0; m_CurrentFrame < EU_VK_MAX_FRAMES_IN_FLIGHT; m_CurrentFrame++)
			UpdateShaderBuffer(shaderBuffer, data, size);
		m_CurrentFrame = currentFrame;
	}

	void RenderContextVK::BindTextureGroup(const TextureGroupBind& groupBind)
	{
		if (Engine::GetDisplay()->IsMinimized())
			return;

		SubpassVK* subpass = &m_RenderPasses[m_CurrentRenderPass - 1].subpasses[m_CurrentSubpass];
		ShaderTextureResourcesVK* resources = &subpass->pipelines[m_CurrentPipeline].shaderResources.textureResources;

		ShaderTextureGroupVK* group = 0;
		for (u32 i = 0; i < resources->textureGroups.Size(); i++)
		{
			if (groupBind.set == resources->textureGroups[i].setNumber)
			{
				group = &resources->textureGroups[i];
				break;
			}
		}

		if (!group)
		{
			EU_LOG_WARN("Trying to bind a texture group with a set number that doesn't exist in the shader program");
			return;
		}


		EU_PERSISTENT List<VkDescriptorImageInfo> descriptor_image_infos;
		for (u32 i = 0; i < groupBind.numTextureBinds; i++)
		{
			const TextureBind& bind = groupBind.binds[i];
			
			VkWriteDescriptorSet write_descriptor_set {};
			write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_descriptor_set.dstSet = group->descriptorSets[m_CurrentFrame][group->currentOffset[m_CurrentFrame]];
			write_descriptor_set.dstBinding = bind.binding;
			write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write_descriptor_set.descriptorCount = bind.textureArrayLength;

			descriptor_image_infos.Clear();
			for (u32 j = 0; j < bind.textureArrayLength; j++)
			{
				VkDescriptorImageInfo descriptor_image_info {};
				descriptor_image_info.sampler = m_Samplers[bind.sampler - 1].sampler;
				descriptor_image_info.imageView = m_Textures[bind.texture[j] - 1].imageView;
				descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				descriptor_image_infos.Push(descriptor_image_info);
			}

			write_descriptor_set.pImageInfo = &descriptor_image_infos[0];

			vkUpdateDescriptorSets(m_Device, 1, &write_descriptor_set, 0, 0);
		}

		group->updated[m_CurrentFrame] = true;
	}

	void RenderContextVK::SubmitRenderCommand(const RenderCommand& command)
	{
		if (Engine::GetDisplay()->IsMinimized())
			return;

		VkCommandBuffer commandBuffer = m_FramesInFlight[m_CurrentFrame].commandBuffer;

		VkBuffer vertexBuffer = m_Buffers[command.vertexBuffer - 1].buffer;
		VkDeviceSize offset = 0;

		RenderPassVK* renderPass = &m_RenderPasses[m_CurrentRenderPass - 1];
		SubpassVK* subpass = &renderPass->subpasses[m_CurrentSubpass];
		GraphicsPipelineVK* pipeline = &subpass->pipelines[m_CurrentPipeline];
		ShaderResourcesVK* resources = &pipeline->shaderResources;
		ShaderBufferResourcesVK* bufferResources = &resources->bufferResources;
		ShaderTextureResourcesVK* textureResources = &resources->textureResources;

		for (u32 i = 0; i < bufferResources->bufferSets.Size(); i++)
		{
			u32 dynamicOffsetIndex = 0;
			ShaderBufferSetVK* bufferSet = &bufferResources->bufferSets[i];
			for (u32 j = 0; j < bufferSet->buffers.Size(); j++)
			{
				if (bufferSet->buffers[j].buffer == EU_INVALID_UNIFORM_BUFFER_ID)
				{
					EU_LOG_WARN("Trying to submit render command before all descriptor sets have been assigned a buffer");
					continue;
				}

				ShaderBufferVK& shaderBuffer = m_ShaderBuffers[bufferSet->buffers[j].buffer - 1];
				if (shaderBuffer.isDynamic)
				{
					bufferSet->dynamicOffsets[dynamicOffsetIndex++] = shaderBuffer.currentOffset[m_CurrentFrame];
				}

				if (shaderBuffer.wasUpdated[m_CurrentFrame])
				{
					if (shaderBuffer.isDynamic)
						shaderBuffer.incrementOffset[m_CurrentFrame] = true;

					shaderBuffer.wasUpdated[m_CurrentFrame] = false;
				}
			}

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipelineLayout, bufferSet->setNumber, 1,
				&bufferResources->descriptorSets[m_CurrentFrame][i], bufferSet->dynamicOffsets.Size(), &bufferSet->dynamicOffsets[0]);

		}

		
		for (u32 i = 0; i < textureResources->textureGroups.Size(); i++)
		{
			ShaderTextureGroupVK* group = &textureResources->textureGroups[i];

			if (!group->updated[m_CurrentFrame])
				continue;

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipelineLayout, group->setNumber, 1,
				&group->descriptorSets[m_CurrentFrame][group->currentOffset[m_CurrentFrame]], 0, 0);


			group->currentOffset[m_CurrentFrame]++;
			group->currentOffset[m_CurrentFrame] %= group->descriptorSets[0].Size();
			group->updated[m_CurrentFrame] = false;
		}

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);
		if (command.indexBuffer != EU_INVALID_BUFFER_ID)
		{
			VkBuffer indexBuffer = m_Buffers[command.indexBuffer - 1].buffer;


			VkIndexType indexType = VK_INDEX_TYPE_UINT32;
			switch (command.indexType)
			{
			case INDEX_TYPE_U16: indexType = VK_INDEX_TYPE_UINT16; break;
			case INDEX_TYPE_U32: indexType = VK_INDEX_TYPE_UINT32; break;
			}

			vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, indexType);
			vkCmdDrawIndexed(commandBuffer, command.count, 1, command.indexOffset, command.vertexOffset, 0);
		}
		else
		{
			vkCmdDraw(commandBuffer, command.count, 1, command.vertexOffset, 0);
		}
	}

	void RenderContextVK::NextSubpass(u32 initialPipeline)
	{
		if (Engine::GetDisplay()->IsMinimized())
			return;

		const FrameInFlightVK& frame = m_FramesInFlight[m_CurrentFrame];

		m_CurrentSubpass++;

		SubpassVK* subpass = &m_RenderPasses[m_CurrentRenderPass - 1].subpasses[m_CurrentSubpass];

		vkCmdNextSubpass(frame.commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, subpass->pipelines[initialPipeline].pipeline);

		if (subpass->inputAttachmentResources.hasInputAttachments)
		{
			vkCmdBindDescriptorSets(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, subpass->pipelines[initialPipeline].pipelineLayout,
				subpass->inputAttachmentResources.setNumber, 1, &subpass->inputAttachmentResources.descriptorSet[m_CurrentFrame], 0, 0);
		}

		m_CurrentPipeline = initialPipeline;
	}

	u32 RenderContextVK::SwitchPipeline(u32 pipeline)
	{
		if (Engine::GetDisplay()->IsMinimized())
			return m_CurrentPipeline;

		if (m_CurrentPipeline == pipeline)
			return m_CurrentPipeline;

		const FrameInFlightVK& frame = m_FramesInFlight[m_CurrentFrame];
		RenderPassVK* renderPass = &m_RenderPasses[m_CurrentRenderPass - 1];
		SubpassVK* subpass = &renderPass->subpasses[m_CurrentSubpass];

		vkCmdBindPipeline(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, subpass->pipelines[pipeline].pipeline);

		u32 oldPipeline = m_CurrentPipeline;
		m_CurrentPipeline = pipeline;

		return oldPipeline;
	}

	void RenderContextVK::EndRenderPass()
	{
		if (Engine::GetDisplay()->IsMinimized())
			return;

		vkCmdEndRenderPass(m_FramesInFlight[m_CurrentFrame].commandBuffer);
	}

	void RenderContextVK::Present()
	{
		if (Engine::GetDisplay()->IsMinimized())
			return;

		const FrameInFlightVK& frame = m_FramesInFlight[m_CurrentFrame];

		EU_CHECK_VKRESULT(vkEndCommandBuffer(frame.commandBuffer), "Could not end Vulkan command buffer recording");

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submit_info {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitDstStageMask = &waitStage;
		submit_info.pWaitSemaphores = &frame.imageAvailableSemaphore;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &frame.renderFinishedSemaphore;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &frame.commandBuffer;

		EU_CHECK_VKRESULT(vkQueueSubmit(m_GraphicsQueue, 1, &submit_info, frame.fence), "Could not submit Vulkan graphics queue");

		VkPresentInfoKHR present_info {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &frame.renderFinishedSemaphore;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &m_Swapchain;
		present_info.pImageIndices = &m_ImageIndex;
		present_info.pResults = 0;

		EU_CHECK_VKRESULT(vkQueuePresentKHR(m_PresentQueue, &present_info), "Could not present Vulkan frame");

		for (u32 i = 0; i < m_ResizeFramebuffers.Size(); i++)
			DoFramebufferResize(m_ResizeFramebuffers[i]);

		m_ResizeFramebuffers.Clear();

		m_CurrentFrame = (m_CurrentFrame + 1) % EU_VK_MAX_FRAMES_IN_FLIGHT;
	}

	void RenderContextVK::ReadPixelsIntoBuffer(TextureID texture, BufferID buffer)
	{
		TextureVK tex = m_Textures[texture - 1];
		BufferVK buf = m_Buffers[buffer - 1];

		CmdTransitionImageLayout(tex.image, GetVkFormat(tex.format), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		VkCommandBuffer commandBuffer = BeginCommands();

		VkBufferImageCopy buffer_image_copy = {};
		buffer_image_copy.bufferOffset = 0;
		buffer_image_copy.bufferRowLength = 0;
		buffer_image_copy.bufferImageHeight = 0;
		buffer_image_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		buffer_image_copy.imageSubresource.baseArrayLayer = 0;
		buffer_image_copy.imageSubresource.mipLevel = 0;
		buffer_image_copy.imageSubresource.layerCount = 1;
		buffer_image_copy.imageOffset = { 0, 0, 0 };
		buffer_image_copy.imageExtent = { tex.width, tex.height, 1 };

		vkCmdCopyImageToBuffer(commandBuffer, tex.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buf.buffer, 1, &buffer_image_copy);
		EndCommands(commandBuffer);

		CmdTransitionImageLayout(tex.image, GetVkFormat(tex.format), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void RenderContextVK::ResizeFramebuffer(RenderPassID renderPass, u32 width, u32 height)
	{
		ResizeFramebufferAtEndOfFrameVK resize;
		resize.renderPass = renderPass;
		resize.width = width;
		resize.height = height;

		m_ResizeFramebuffers.Push(resize);
	}

	void* RenderContextVK::MapBuffer(BufferID buffer)
	{
		const BufferVK& bufferVK = m_Buffers[buffer - 1];
		if (!bufferVK.canBeMapped)
			return 0;
		
		void* data;
		EU_CHECK_VKRESULT(vkMapMemory(m_Device, bufferVK.memory, 0, bufferVK.size, 0, &data), "Could not map Vulkan buffer", 0);

		return data;
	}

	void RenderContextVK::UnmapBuffer(BufferID buffer)
	{
		vkUnmapMemory(m_Device, m_Buffers[buffer - 1].memory);
	}

	void RenderContextVK::GetTextureSize(TextureID texture, u32* width, u32* height, u32* depth)
	{
		const TextureVK& textureVK = m_Textures[texture - 1];
		*width = textureVK.width;
		*height = textureVK.height;

		if (depth)
			*depth = textureVK.depth;
	}

	void RenderContextVK::GetFramebufferSize(RenderPassID renderPass, u32* width, u32* height)
	{
		*width = m_RenderPasses[renderPass - 1].framebufferExtent.width;
		*height = m_RenderPasses[renderPass - 1].framebufferExtent.height;
	}

	RenderAPI RenderContextVK::GetRenderAPI() const
	{
		return RENDER_API_VULKAN;
	}

	void RenderContextVK::InitInstance()
	{
		VkApplicationInfo application_info {};
		application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		application_info.apiVersion = VK_MAKE_VERSION(1, 2, 0);
		application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		application_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);
		application_info.pEngineName = "Eunoia";
		application_info.pApplicationName = "Eunoia-App";

		List<const char*> requiredExtensions;

		requiredExtensions.Push(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef EU_PLATFORM_WINDOWS
		requiredExtensions.Push(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
		
		if (!CheckRequiredExtensionSupport(requiredExtensions))
		{
			EU_LOG_FATAL("This device does not support the required Vulkan extensions");
			return;
		}

		VkInstanceCreateInfo instance_create_info{};
		instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pApplicationInfo = &application_info;
		instance_create_info.enabledLayerCount = 0;
		instance_create_info.ppEnabledLayerNames = 0;

#ifdef EU_ENABLE_VK_VALIDATION_LAYERS
		if (CheckValidationLayerSupport())
		{
			requiredExtensions.Push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			instance_create_info.enabledLayerCount = s_ValidationLayers.Size();
			instance_create_info.ppEnabledLayerNames = &s_ValidationLayers[0];
		}
		else
		{
			EU_LOG_WARN("Validation layers are enabled but are not supported. Continuing without validation layers");
		}
#endif

		instance_create_info.enabledExtensionCount = requiredExtensions.Size();
		instance_create_info.ppEnabledExtensionNames = &requiredExtensions[0];

		EU_CHECK_VKRESULT(vkCreateInstance(&instance_create_info, 0, &m_Instance), "Could not create Vulkan instance");
		EU_LOG_TRACE("Created Vulkan instance");
	}

	b32 RenderContextVK::CheckRequiredExtensionSupport(const List<const char*>& requiredExtensions)
	{
		u32 extensionCount;
		vkEnumerateInstanceExtensionProperties(0, &extensionCount, 0);

		List<VkExtensionProperties> properties(extensionCount, extensionCount);
		vkEnumerateInstanceExtensionProperties(0, &extensionCount, &properties[0]);

		b32 missingRequiredExtensions = false;
		String missingExtensionsErrorMsg = "Missing required Vulkan extensions:";
		for (u32 i = 0; i < requiredExtensions.Size(); i++)
		{
			b32 found = false;
			for (u32 j = 0; j < properties.Size(); j++)
			{
				if (strcmp(requiredExtensions[i], properties[j].extensionName) == 0)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				missingExtensionsErrorMsg += (String("\n\t") + requiredExtensions[i]);
				missingRequiredExtensions = true;
			}
		}

		if (missingRequiredExtensions)
		{
			EU_LOG_FATAL(missingExtensionsErrorMsg.C_Str());
			return false;
		}

		return true;

	}

	b32 RenderContextVK::CheckValidationLayerSupport()
	{
		u32 layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, 0);

		List<VkLayerProperties> properties(layerCount, layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, &properties[0]);

		b32 missingValidationLayers = false;
		String missingLayersMsg = "Missing required Vulkan validation layers:";

		for (u32 i = 0; i < s_ValidationLayers.Size(); i++)
		{
			b32 found = false;
			for (u32 j = 0; j < properties.Size(); j++)
			{
				if (strcmp(s_ValidationLayers[i], properties[j].layerName) == 0)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				missingValidationLayers = true;
				missingLayersMsg += (String("\n\t") + s_ValidationLayers[i]);
			}
		}

		if (missingValidationLayers)
		{
			EU_LOG_ERROR(missingLayersMsg.C_Str());
			return false;
		}

		return true;
	}

	void RenderContextVK::InitDebugUtilsMessenger()
	{
#ifdef EU_ENABLE_VK_VALIDATION_LAYERS
		VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info{};
		debug_utils_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debug_utils_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debug_utils_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debug_utils_messenger_create_info.pfnUserCallback = VulkanDebugCallback;
		debug_utils_messenger_create_info.pUserData = this;

		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");
		if (!func)
		{
			EU_LOG_ERROR("Could not find vkCreateDebugUtilsMessengerEXT function");
			return;
		}

		EU_CHECK_VKRESULT(func(m_Instance, &debug_utils_messenger_create_info, 0, &m_DebugUtilsMessenger), "Could not create Vulkan debug messenger");
		EU_LOG_TRACE("Created Vulkan debug messenger");
#endif
	}

	void RenderContextVK::ChoosePhysicalDevice()
	{
		u32 deviceCount;
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, 0);

		List<VkPhysicalDevice> availableDevices(deviceCount, deviceCount);
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, &availableDevices[0]);

		String deviceName;

		for (u32 i = 0; i < availableDevices.Size(); i++)
		{
			if (IsGPUSuitable(availableDevices[i]))
			{
				m_PhysicalDevice = availableDevices[i];
				VkPhysicalDeviceProperties properties;
				vkGetPhysicalDeviceProperties(m_PhysicalDevice, &properties);
				deviceName = properties.deviceName;
			}
		}

		if (!m_PhysicalDevice)
		{
			if (availableDevices.Empty())
			{
				EU_LOG_FATAL("Could not find a GPU that supports Vulkan graphics");
				return;
			}
			else
			{
				EU_LOG_WARN("Could not find a discrete gpu that supports graphics. Using the first avaliable one");
				m_PhysicalDevice = availableDevices[0];
			}
		}

		EU_LOG_TRACE((String("Found a suitale GPU that supports Vulkan: ") + deviceName).C_Str());
	}

	b32 RenderContextVK::IsGPUSuitable(VkPhysicalDevice physicalDevice)
	{
		VkPhysicalDeviceProperties properties = {};
		VkPhysicalDeviceFeatures features = {};
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		vkGetPhysicalDeviceFeatures(physicalDevice, &features);

		QueueFamilyIndices indices = FindQueueFamilyIndices(physicalDevice);

		b32 swapchainValid = false;
		if (CheckDeviceExtensionSuppport(physicalDevice))
		{
			SwapchainSupportDetails details = GetSwapchainSupportDetails(physicalDevice);
			swapchainValid = !details.formats.Empty() && !details.presentModes.Empty();
		}

		return AreQueueFamilyIndicesValid(indices) && swapchainValid &&
			   properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
	}

	QueueFamilyIndices RenderContextVK::FindQueueFamilyIndices(VkPhysicalDevice physicalDevice)
	{
		QueueFamilyIndices indices;
		indices.graphics = -1;
		indices.present = -1;

		u32 queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, 0);
		List<VkQueueFamilyProperties> properties(queueFamilyCount, queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, &properties[0]);

		for (u32 i = 0; i < queueFamilyCount; i++)
		{
			if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				indices.graphics = i;

			VkBool32 presentSupport;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, m_Surface, &presentSupport);

			if (presentSupport)
				indices.present = i;

			if (AreQueueFamilyIndicesValid(indices))
				break;

		}


		return indices;
	}

	b32 RenderContextVK::AreQueueFamilyIndicesValid(const QueueFamilyIndices& indices)
	{
		return indices.graphics != -1 && indices.present != -1;
	}

	void RenderContextVK::InitLogicalDevice()
	{
		QueueFamilyIndices indices = FindQueueFamilyIndices(m_PhysicalDevice);

		u32 uniqueIndices[4];
		VkDeviceQueueCreateInfo device_queue_create_infos[4];

		u32 numUniqueIndices = 1;
		uniqueIndices[0] = indices.graphics;
		if (indices.graphics != indices.present)
		{
			numUniqueIndices++;
			uniqueIndices[1] = indices.present;
		}

		r32 priority = 1.0f;
		for (u32 i = 0; i < numUniqueIndices; i++)
		{
			VkDeviceQueueCreateInfo device_queue_create_info {};
			device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			device_queue_create_info.pQueuePriorities = &priority;
			device_queue_create_info.queueCount = 1;
			device_queue_create_info.queueFamilyIndex = uniqueIndices[i];
			device_queue_create_infos[i] = device_queue_create_info;
		}

		VkPhysicalDeviceFeatures requiredFeatures {};
		requiredFeatures.samplerAnisotropy = true;
		requiredFeatures.depthClamp = true;
		requiredFeatures.fillModeNonSolid = true;

		VkDeviceCreateInfo device_create_info {};
		device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_create_info.queueCreateInfoCount = numUniqueIndices;
		device_create_info.pQueueCreateInfos = device_queue_create_infos;
		device_create_info.pEnabledFeatures = &requiredFeatures;
		device_create_info.enabledExtensionCount = s_RequiredDeviceExtensions.Size();
		device_create_info.ppEnabledExtensionNames = &s_RequiredDeviceExtensions[0];
		device_create_info.enabledLayerCount = 0;
		device_create_info.ppEnabledLayerNames = 0;
#ifdef EU_ENABLE_VK_VALIDATION_LAYERS
		device_create_info.enabledLayerCount = s_ValidationLayers.Size();
		device_create_info.ppEnabledLayerNames = &s_ValidationLayers[0];
#endif

		EU_CHECK_VKRESULT(vkCreateDevice(m_PhysicalDevice, &device_create_info, 0, &m_Device), "Could not create Vulkan logical device");

		vkGetDeviceQueue(m_Device, indices.graphics, 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, indices.present, 0, &m_PresentQueue);

		EU_LOG_TRACE("Created Vulkan logical device");
	}

	void RenderContextVK::InitSurface(Display* display)
	{
#ifdef EU_PLATFORM_WINDOWS
		VkWin32SurfaceCreateInfoKHR win32_surface_create_info {};
		win32_surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		win32_surface_create_info.hinstance = GetModuleHandle(0);
		win32_surface_create_info.hwnd = ((DisplayWin32*)display)->GetHandle();

		EU_CHECK_VKRESULT(vkCreateWin32SurfaceKHR(m_Instance, &win32_surface_create_info, 0, &m_Surface), "Could not create Vulkan Win32 surface");
		EU_LOG_TRACE("Created Vulkan Win32 surface");
#endif
	}

	b32 RenderContextVK::CheckDeviceExtensionSuppport(VkPhysicalDevice physicalDevice)
	{
		u32 extensionCount;
		vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &extensionCount, 0);

		List<VkExtensionProperties> properties(extensionCount, extensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, 0, &extensionCount, &properties[0]);

		b32 missingExtensions = false;
		String missingExtensionsMsg = "Missing required Vulkan device extensions:";

		for (u32 i = 0; i < s_RequiredDeviceExtensions.Size(); i++)
		{
			b32 found = false;
			for (u32 j = 0; j < properties.Size(); j++)
			{
				if (strcmp(s_RequiredDeviceExtensions[i], properties[j].extensionName) == 0)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				missingExtensions = true;
				missingExtensionsMsg = String("\n\t") + s_RequiredDeviceExtensions[i];
			}
		}

		if (missingExtensions)
		{
			EU_LOG_FATAL(missingExtensionsMsg.C_Str());
			return false;
		}

		return true;
	}

	SwapchainSupportDetails RenderContextVK::GetSwapchainSupportDetails(VkPhysicalDevice physicalDevice)
	{
		SwapchainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &details.capabilities);

		u32 formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, 0);

		if (formatCount > 0)
		{
			details.formats.SetCapacityAndElementCount(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, &details.formats[0]);
		}

		u32 presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, 0);

		if (presentModeCount > 0)
		{
			details.presentModes.SetCapacityAndElementCount(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, &details.presentModes[0]);
		}

		return details;
	}

	VkSurfaceFormatKHR RenderContextVK::ChooseSwapchainSurfaceFormat(const SwapchainSupportDetails& details)
	{
		for (u32 i = 0; i < details.formats.Size(); i++)
		{
			const VkSurfaceFormatKHR& format = details.formats[i];
			if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
			{
				return format;
			}
		}

		EU_LOG_WARN("Could not find ideal swapchain surface format. Using first available one");
		return details.formats[0];
	}

	VkPresentModeKHR RenderContextVK::ChooseSwapchainPresentMode(const SwapchainSupportDetails& details)
	{
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D RenderContextVK::ChooseSwapchainExtent(const SwapchainSupportDetails& details, u32 width, u32 height)
	{
		if (details.capabilities.currentExtent.width != EU_U32_MAX)
		{
			return details.capabilities.currentExtent;
		}
		else
		{
			VkExtent2D extent = { width, height };

			extent.width = EU_CLAMP(details.capabilities.minImageExtent.width, details.capabilities.maxImageExtent.width, extent.width);
			extent.height = EU_CLAMP(details.capabilities.minImageExtent.height, details.capabilities.maxImageExtent.height, extent.height);

			return extent;
		}
	}

	void RenderContextVK::InitSwapchain()
	{
		Display* display = Engine::GetDisplay();

		SwapchainSupportDetails details = GetSwapchainSupportDetails(m_PhysicalDevice);

		VkSurfaceFormatKHR format = ChooseSwapchainSurfaceFormat(details);
		VkPresentModeKHR presentMode = ChooseSwapchainPresentMode(details);
		VkExtent2D extent = ChooseSwapchainExtent(details, display->GetWidth(), display->GetHeigth());

		u32 desiredImageCount = 2;
		u32 maxImageCount = details.capabilities.maxImageCount == 0 ? desiredImageCount : details.capabilities.maxImageCount;
		u32 imageCount = EU_CLAMP(details.capabilities.minImageCount, maxImageCount, desiredImageCount);

		VkSwapchainCreateInfoKHR swapchain_create_info{};
		swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_create_info.surface = m_Surface;
		swapchain_create_info.minImageCount = imageCount;
		swapchain_create_info.imageFormat = format.format;
		swapchain_create_info.imageColorSpace = format.colorSpace;
		swapchain_create_info.presentMode = presentMode;
		swapchain_create_info.imageExtent = extent;
		swapchain_create_info.imageArrayLayers = 1;
		swapchain_create_info.preTransform = details.capabilities.currentTransform;
		swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_create_info.clipped = VK_TRUE;
		swapchain_create_info.oldSwapchain = 0;
		//Consider VK_IMAGE_USAGE_TRANSFER_DST_BIT  later
		swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = FindQueueFamilyIndices(m_PhysicalDevice);

		if (indices.graphics != indices.present)
		{
			u32 indicesArray[2] = { indices.graphics, indices.present };
			swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchain_create_info.queueFamilyIndexCount = 2;
			swapchain_create_info.pQueueFamilyIndices = indicesArray;
		}
		else
		{
			swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchain_create_info.queueFamilyIndexCount = 0;
			swapchain_create_info.pQueueFamilyIndices = 0;
		}

		m_SwapchainFormat = format.format;
		m_SwapchainExtent = extent;

		EU_CHECK_VKRESULT(vkCreateSwapchainKHR(m_Device, &swapchain_create_info, 0, &m_Swapchain), "Could not create Vulkan swapchain");
		EU_CHECK_VKRESULT(vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, 0), "Could not get Vulkan swapchain images");
		m_SwapchainImages.SetCapacityAndElementCount(imageCount);
		EU_CHECK_VKRESULT(vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, &m_SwapchainImages[0]), "Could not get Vulkan swapchain images");
		EU_LOG_TRACE("Created Vulkan swapchain");
	}

	void RenderContextVK::InitSwapchainImageViews()
	{
		m_SwapchainImageViews.SetCapacityAndElementCount(m_SwapchainImages.Size());

		for (u32 i = 0; i < m_SwapchainImageViews.Size(); i++)
		{
			VkImageViewCreateInfo image_view_create_info{};
			image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_view_create_info.image = m_SwapchainImages[i];
			image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			image_view_create_info.format = m_SwapchainFormat;
			image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			image_view_create_info.subresourceRange.baseMipLevel = 0;
			image_view_create_info.subresourceRange.levelCount = 1;
			image_view_create_info.subresourceRange.baseArrayLayer = 0;
			image_view_create_info.subresourceRange.layerCount = 1;

			EU_CHECK_VKRESULT(vkCreateImageView(m_Device, &image_view_create_info, 0, &m_SwapchainImageViews[i]), "Could not create Vulkan swapchain image views");
		}

		EU_LOG_TRACE("Created Vulkan swapchain image views");
	}

	void RenderContextVK::ParseShader(const char* spirv, u32 size, ShaderParseInfoVK* parseInfo)
	{
		SpvReflectShaderModule reflect_shader_module;
		spvReflectCreateShaderModule(size, spirv, &reflect_shader_module);
		
		VkShaderStageFlags shaderStage = 0;

		if ((reflect_shader_module.shader_stage & SPV_REFLECT_SHADER_STAGE_VERTEX_BIT) == SPV_REFLECT_SHADER_STAGE_VERTEX_BIT)
			shaderStage = VK_SHADER_STAGE_VERTEX_BIT;

		if ((reflect_shader_module.shader_stage & SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT) == SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT)
			shaderStage = VK_SHADER_STAGE_FRAGMENT_BIT;

		for (u32 i = 0; i < reflect_shader_module.descriptor_binding_count; i++)
		{
			
			const SpvReflectDescriptorBinding* reflect_descriptor_binding = &reflect_shader_module.descriptor_bindings[i];
			if (reflect_descriptor_binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			{
				ShaderParseInfoUniformBufferVK uniformBuffer;
				uniformBuffer.name = reflect_descriptor_binding->type_description->type_name;
				uniformBuffer.binding = reflect_descriptor_binding->binding;
				uniformBuffer.size = reflect_descriptor_binding->block.size;
				uniformBuffer.arrayCount = reflect_descriptor_binding->count;
				uniformBuffer.isStorageBuffer = false;

				b32 inserted = false;
				for (u32 j = 0; j < parseInfo->bufferSets.Size(); j++)
				{
					if (parseInfo->bufferSets[j].set == reflect_descriptor_binding->set)
					{
						parseInfo->bufferSets[j].uniformBuffers.Push(uniformBuffer);
						inserted = true;
						break;
					}
				}

				if (!inserted)
				{
					ShaderParseInfoUniformBufferSetVK uniformBufferSet;
					uniformBufferSet.stage = shaderStage;
					uniformBufferSet.set = reflect_descriptor_binding->set;
					uniformBufferSet.uniformBuffers.Push(uniformBuffer);
					parseInfo->bufferSets.Push(uniformBufferSet);
				}
			}
			else if (reflect_descriptor_binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER)
			{
				ShaderParseInfoUniformBufferVK storageBuffer;
				storageBuffer.name = reflect_descriptor_binding->type_description->type_name;
				storageBuffer.binding = reflect_descriptor_binding->binding;
				storageBuffer.size = reflect_descriptor_binding->block.size;
				storageBuffer.arrayCount = reflect_descriptor_binding->count;
				storageBuffer.isStorageBuffer = true;
				
				b32 inserted = false;
				for (u32 j = 0; j < parseInfo->bufferSets.Size(); j++)
				{
					if (parseInfo->bufferSets[j].set == reflect_descriptor_binding->set)
					{
						parseInfo->bufferSets[j].uniformBuffers.Push(storageBuffer);
						inserted = true;
						break;
					}
				}

				if (!inserted)
				{
					ShaderParseInfoUniformBufferSetVK uniformBufferSet;
					uniformBufferSet.stage = shaderStage;
					uniformBufferSet.set = reflect_descriptor_binding->set;
					uniformBufferSet.uniformBuffers.Push(storageBuffer);
					parseInfo->bufferSets.Push(uniformBufferSet);
				}
			}
			else if (reflect_descriptor_binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				ShaderParseInfoTextureVK texture;
				texture.name = reflect_descriptor_binding->name;
				texture.binding = reflect_descriptor_binding->binding;
				texture.arrayCount = reflect_descriptor_binding->count;

				b32 inserted = false;
				for (u32 j = 0; j < parseInfo->textureSets.Size(); j++)
				{
					if (parseInfo->textureSets[j].set == reflect_descriptor_binding->set)
					{
						parseInfo->textureSets[j].textures.Push(texture);
						inserted = true;
					}
				}

				if (!inserted)
				{
					ShaderParseInfoTextureSetVK textureSet;
					textureSet.stage = shaderStage;
					textureSet.set = reflect_descriptor_binding->set;
					textureSet.textures.Push(texture);
					parseInfo->textureSets.Push(textureSet);
				}
			}
			else if (reflect_descriptor_binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)
			{
				ShaderParseInfoInputAttachmentVK inputAttachment;
				inputAttachment.stage = shaderStage;
				inputAttachment.attachmentIndex = reflect_descriptor_binding->input_attachment_index;
				inputAttachment.binding = reflect_descriptor_binding->binding;
				inputAttachment.set = reflect_descriptor_binding->set;
				inputAttachment.count = reflect_descriptor_binding->count;

				parseInfo->inputAttachments.Push(inputAttachment);
			}
		}
	}

	void RenderContextVK::InitShaderLayouts(ShaderLayoutsVK* layouts, const ShaderParseInfoVK& parseInfo, const List<String>& dynamicBuffers)
	{
		//Uniform buffer layouts
		layouts->bufferDescriptorSetLayouts.SetCapacityAndElementCount(parseInfo.bufferSets.Size());
		for (u32 i = 0; i < parseInfo.bufferSets.Size(); i++)
		{
			const ShaderParseInfoUniformBufferSetVK& bufferSetInfo = parseInfo.bufferSets[i];
			List<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings(bufferSetInfo.uniformBuffers.Size(), bufferSetInfo.uniformBuffers.Size());

			for (u32 j = 0; j < bufferSetInfo.uniformBuffers.Size(); j++)
			{
				const ShaderParseInfoUniformBufferVK& bufferInfo = bufferSetInfo.uniformBuffers[j];
				
				b32 isDynamic = false;
				for (u32 k = 0; k < dynamicBuffers.Size(); k++)
				{
					if (dynamicBuffers[k] == bufferInfo.name)
					{
						isDynamic = true;
						break;
					}
				}

				VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				if (bufferInfo.isStorageBuffer)
					descriptorType = isDynamic ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				else
					descriptorType = isDynamic ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

				descriptor_set_layout_bindings[j].binding = bufferInfo.binding;
				descriptor_set_layout_bindings[j].descriptorCount = bufferInfo.arrayCount;
				descriptor_set_layout_bindings[j].descriptorType = descriptorType;
				descriptor_set_layout_bindings[j].stageFlags = bufferSetInfo.stage;
			}

			VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info {};
			descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptor_set_layout_create_info.bindingCount = descriptor_set_layout_bindings.Size();
			descriptor_set_layout_create_info.pBindings = &descriptor_set_layout_bindings[0];
			descriptor_set_layout_create_info.flags = 0;

			EU_CHECK_VKRESULT(vkCreateDescriptorSetLayout(m_Device, &descriptor_set_layout_create_info, 0, &layouts->bufferDescriptorSetLayouts[i]), "Could not create Vulkan descriptor set layout");
		}

		//Texture layouts
		layouts->textureDescriptorSetLayouts.SetCapacityAndElementCount(parseInfo.textureSets.Size());
		for (u32 i = 0; i < parseInfo.textureSets.Size(); i++)
		{
			const ShaderParseInfoTextureSetVK& textureSetInfo = parseInfo.textureSets[i];

			List<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings(textureSetInfo.textures.Size(), textureSetInfo.textures.Size());
			for (u32 j = 0; j < textureSetInfo.textures.Size(); j++)
			{
				const ShaderParseInfoTextureVK& textureInfo = textureSetInfo.textures[j];
				
				VkDescriptorSetLayoutBinding descriptor_set_layout_binding {};

				descriptor_set_layout_binding.binding = textureInfo.binding;
				descriptor_set_layout_binding.descriptorCount = textureInfo.arrayCount;
				descriptor_set_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptor_set_layout_binding.stageFlags = textureSetInfo.stage;

				descriptor_set_layout_bindings[j] = descriptor_set_layout_binding;
			}

			VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info {};
			descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptor_set_layout_create_info.bindingCount = descriptor_set_layout_bindings.Size();
			descriptor_set_layout_create_info.pBindings = &descriptor_set_layout_bindings[0];
			descriptor_set_layout_create_info.flags = 0;

			EU_CHECK_VKRESULT(vkCreateDescriptorSetLayout(m_Device, &descriptor_set_layout_create_info, 0, &layouts->textureDescriptorSetLayouts[i]), "Could not create Vulkan descriptor set layout");
		}

		//Input attachment layout
		if(!parseInfo.inputAttachments.Empty())
		{
			List<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings(parseInfo.inputAttachments.Size(), parseInfo.inputAttachments.Size());
			for (u32 i = 0; i < parseInfo.inputAttachments.Size(); i++)
			{
				const ShaderParseInfoInputAttachmentVK& inputAttachmentInfo = parseInfo.inputAttachments[i];

				descriptor_set_layout_bindings[i].binding = inputAttachmentInfo.binding;
				descriptor_set_layout_bindings[i].descriptorCount = inputAttachmentInfo.count;
				descriptor_set_layout_bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
				descriptor_set_layout_bindings[i].stageFlags = inputAttachmentInfo.stage;
			}

			VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info {};
			descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptor_set_layout_create_info.bindingCount = descriptor_set_layout_bindings.Size();
			descriptor_set_layout_create_info.pBindings = &descriptor_set_layout_bindings[0];
			descriptor_set_layout_create_info.flags = 0;

			EU_CHECK_VKRESULT(vkCreateDescriptorSetLayout(m_Device, &descriptor_set_layout_create_info, 0, &layouts->inputAttachmentDescriptorSetLayout), "Could not create Vulkan descriptor set layout");
		}
	}

	void RenderContextVK::InitGraphicsPipeline(GraphicsPipelineVK* pipeline, const GraphicsPipeline& pipelineSettings, RenderPassVK* renderPass, u32 subpass)
	{
		ShaderVK* shader = &m_Shaders[pipeline->shader - 1];

		VkPipelineShaderStageCreateInfo pipeline_vertex_shader_stage_create_info{};
		pipeline_vertex_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pipeline_vertex_shader_stage_create_info.module = shader->vertex;
		pipeline_vertex_shader_stage_create_info.pName = "main";
		pipeline_vertex_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;

		VkPipelineShaderStageCreateInfo pipeline_fragment_shader_stage_create_info{};
		pipeline_fragment_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pipeline_fragment_shader_stage_create_info.module = shader->fragment;
		pipeline_fragment_shader_stage_create_info.pName = "main";
		pipeline_fragment_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_infos[2] = { pipeline_vertex_shader_stage_create_info, pipeline_fragment_shader_stage_create_info };

		VkVertexInputBindingDescription vertex_input_binding_description{};
		vertex_input_binding_description.binding = 0;
		vertex_input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		vertex_input_binding_description.stride = pipelineSettings.vertexInputState.vertexSize;

		u32 offset = 0;
		List<VkVertexInputAttributeDescription> vertex_input_attribute_descriptions;
		for (u32 i = 0; i < pipelineSettings.vertexInputState.numAttributes; i++)
		{
			const VertexAttribute& attribute = pipelineSettings.vertexInputState.attributes[i];

			VkVertexInputAttributeDescription vertex_input_attribute_description{};
			vertex_input_attribute_description.binding = 0;
			vertex_input_attribute_description.offset = offset;
			vertex_input_attribute_description.location = attribute.location;

			switch (attribute.type)
			{
			case VERTEX_ATTRIBUTE_FLOAT: {
				vertex_input_attribute_description.format = VK_FORMAT_R32_SFLOAT;
				offset += sizeof(r32) * 1;
			} break;
			case VERTEX_ATTRIBUTE_FLOAT2: {
				vertex_input_attribute_description.format = VK_FORMAT_R32G32_SFLOAT;
				offset += sizeof(r32) * 2;
			} break;
			case VERTEX_ATTRIBUTE_FLOAT3: {
				vertex_input_attribute_description.format = VK_FORMAT_R32G32B32_SFLOAT;
				offset += sizeof(r32) * 3;
			} break;
			case VERTEX_ATTRIBUTE_FLOAT4: {
				vertex_input_attribute_description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
				offset += sizeof(r32) * 4;
			} break;
			case VERTEX_ATTRIBUTE_U32: {
				vertex_input_attribute_description.format = VK_FORMAT_R32_UINT;
				offset += sizeof(u32);
			} break;
			case VERTEX_ATTRIBUTE_U32_2: {
				vertex_input_attribute_description.format = VK_FORMAT_R32G32_UINT;
				offset += sizeof(u32) * 2;
			}
			case VERTEX_ATTRIBUTE_U32_3: {
				vertex_input_attribute_description.format = VK_FORMAT_R32G32B32_UINT;
				offset += sizeof(u32) * 3;
			}
			case VERTEX_ATTRIBUTE_U32_4: {
				vertex_input_attribute_description.format = VK_FORMAT_R32G32B32A32_UINT;
				offset += sizeof(u32) * 4;
			}
			}

			vertex_input_attribute_descriptions.Push(vertex_input_attribute_description);
		}

		if (pipelineSettings.vertexInputState.vertexSize == EU_VERTEX_SIZE_AUTO)
			vertex_input_binding_description.stride = offset;

		VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info{};
		pipeline_vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		pipeline_vertex_input_state_create_info.vertexBindingDescriptionCount = 1;
		pipeline_vertex_input_state_create_info.pVertexBindingDescriptions = &vertex_input_binding_description;
		pipeline_vertex_input_state_create_info.vertexAttributeDescriptionCount = vertex_input_attribute_descriptions.Size();
		pipeline_vertex_input_state_create_info.pVertexAttributeDescriptions = &vertex_input_attribute_descriptions[0];

		VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_create_info{};
		pipeline_input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		pipeline_input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;
		pipeline_input_assembly_state_create_info.topology = GetVkPrimitiveTopology(pipelineSettings.topology);

		VkViewport viewport = {};
		viewport.x = pipelineSettings.viewportState.viewport.x;
		viewport.y = pipelineSettings.viewportState.viewport.y;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		if (pipelineSettings.viewportState.useFramebufferSizeForViewport)
		{
			viewport.width = (r32)renderPass->framebufferExtent.width;
			viewport.height = (r32)renderPass->framebufferExtent.height;
		}
		else
		{
			viewport.width = pipelineSettings.viewportState.viewport.width;
			viewport.height = pipelineSettings.viewportState.viewport.height;
		}

		VkRect2D scissor = {};
		scissor.offset = { (s32)pipelineSettings.viewportState.scissor.x, (s32)pipelineSettings.viewportState.scissor.y };
		if (pipelineSettings.viewportState.useFramebufferSizeForScissor)
			scissor.extent = renderPass->framebufferExtent;
		else
			scissor.extent = { pipelineSettings.viewportState.scissor.width, pipelineSettings.viewportState.scissor.height };

		VkPipelineViewportStateCreateInfo pipeline_viewport_state_create_info{};
		pipeline_viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		pipeline_viewport_state_create_info.viewportCount = 1;
		pipeline_viewport_state_create_info.pViewports = &viewport;
		pipeline_viewport_state_create_info.scissorCount = 1;
		pipeline_viewport_state_create_info.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info{};
		pipeline_rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		pipeline_rasterization_state_create_info.depthClampEnable = pipelineSettings.rasterizationState.depthClampEnabled;
		pipeline_rasterization_state_create_info.cullMode = GetVkCullMode(pipelineSettings.rasterizationState.cullMode);
		pipeline_rasterization_state_create_info.polygonMode = GetVkPolygonMode(pipelineSettings.rasterizationState.polygonMode);
		pipeline_rasterization_state_create_info.frontFace = GetVkFrontFace(pipelineSettings.rasterizationState.frontFace);
		pipeline_rasterization_state_create_info.rasterizerDiscardEnable = pipelineSettings.rasterizationState.discard;
		pipeline_rasterization_state_create_info.lineWidth = 1.0f;
		pipeline_rasterization_state_create_info.depthClampEnable = pipelineSettings.rasterizationState.depthClampEnabled;
		pipeline_rasterization_state_create_info.depthBiasEnable = VK_FALSE;
		pipeline_rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
		pipeline_rasterization_state_create_info.depthBiasClamp = 0.0f;
		pipeline_rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;

		VkPipelineMultisampleStateCreateInfo pipeline_multisample_state_create_info{};
		pipeline_multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		pipeline_multisample_state_create_info.sampleShadingEnable = VK_FALSE;
		pipeline_multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		pipeline_multisample_state_create_info.minSampleShading = 1.0f;
		pipeline_multisample_state_create_info.pSampleMask = 0;
		pipeline_multisample_state_create_info.alphaToCoverageEnable = VK_FALSE;
		pipeline_multisample_state_create_info.alphaToOneEnable = VK_FALSE;

		const DepthStencilState& depthStencil = pipelineSettings.depthStencilState;
		VkPipelineDepthStencilStateCreateInfo pipeline_depth_stencil_state_create_info{};
		pipeline_depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		pipeline_depth_stencil_state_create_info.depthTestEnable = depthStencil.depthTestEnabled;
		pipeline_depth_stencil_state_create_info.depthWriteEnable = depthStencil.depthWriteEnabled;
		pipeline_depth_stencil_state_create_info.depthBoundsTestEnable = false;
		pipeline_depth_stencil_state_create_info.minDepthBounds = 0.0f;
		pipeline_depth_stencil_state_create_info.maxDepthBounds = 1.0f;
		pipeline_depth_stencil_state_create_info.stencilTestEnable = depthStencil.stencilTestEnabled;
		pipeline_depth_stencil_state_create_info.depthCompareOp = GetVkCompareOp(depthStencil.depthCompare);
		GetVkStencilOpState(&pipeline_depth_stencil_state_create_info.front, depthStencil.frontFace);
		GetVkStencilOpState(&pipeline_depth_stencil_state_create_info.back, depthStencil.backFace);

		List<VkPipelineColorBlendAttachmentState> pipeline_color_blend_attachment_states(pipelineSettings.numBlendStates, pipelineSettings.numBlendStates);
		for (u32 i = 0; i < pipelineSettings.numBlendStates; i++)
		{
			const BlendState& blendState = pipelineSettings.blendStates[i];
			VkPipelineColorBlendAttachmentState pipeline_color_blend_attachment_state{};
			pipeline_color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			pipeline_color_blend_attachment_state.blendEnable = blendState.blendEnabled;
			pipeline_color_blend_attachment_state.srcColorBlendFactor = GetVkBlendFactor(blendState.color.srcFactor);
			pipeline_color_blend_attachment_state.dstColorBlendFactor = GetVkBlendFactor(blendState.color.dstFactor);
			pipeline_color_blend_attachment_state.colorBlendOp = GetVkBlendOp(blendState.color.operation);
			pipeline_color_blend_attachment_state.srcAlphaBlendFactor = GetVkBlendFactor(blendState.alpha.srcFactor);
			pipeline_color_blend_attachment_state.dstAlphaBlendFactor = GetVkBlendFactor(blendState.alpha.dstFactor);
			pipeline_color_blend_attachment_state.alphaBlendOp = GetVkBlendOp(blendState.alpha.operation);
			pipeline_color_blend_attachment_states[i] = pipeline_color_blend_attachment_state;
		}

		VkPipelineColorBlendStateCreateInfo pipeline_color_blend_state_create_info{};
		pipeline_color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		pipeline_color_blend_state_create_info.logicOpEnable = false;
		pipeline_color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
		pipeline_color_blend_state_create_info.attachmentCount = pipelineSettings.numBlendStates;
		pipeline_color_blend_state_create_info.pAttachments = &pipeline_color_blend_attachment_states[0];
		pipeline_color_blend_state_create_info.blendConstants[0] = 0.0f;
		pipeline_color_blend_state_create_info.blendConstants[1] = 0.0f;
		pipeline_color_blend_state_create_info.blendConstants[2] = 0.0f;
		pipeline_color_blend_state_create_info.blendConstants[3] = 0.0f;

		List<VkDynamicState> dynamicStates(NUM_DYNAMIC_STATES);
		for (u32 i = 0; i < NUM_DYNAMIC_STATES; i++)
			if (pipelineSettings.dynamicStates[i])
				dynamicStates.Push(GetVkDynamicState((DynamicState)i));

		VkPipelineDynamicStateCreateInfo pipeline_dynamic_state_create_info{};
		pipeline_dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		pipeline_dynamic_state_create_info.dynamicStateCount = dynamicStates.Size();
		pipeline_dynamic_state_create_info.pDynamicStates = &dynamicStates[0];

		const ShaderLayoutsVK& layouts = pipeline->shaderResources.layouts;

		VkDescriptorSetLayout l = 0;
		List<VkDescriptorSetLayout> descriptor_set_layouts(128, 128, &l);
		for (u32 i = 0; i < layouts.bufferDescriptorSetLayouts.Size(); i++)
			descriptor_set_layouts[shader->parseInfo.bufferSets[i].set] = layouts.bufferDescriptorSetLayouts[i];

		for (u32 i = 0; i < layouts.textureDescriptorSetLayouts.Size(); i++)
			descriptor_set_layouts[shader->parseInfo.textureSets[i].set] = layouts.textureDescriptorSetLayouts[i];

		if (!shader->parseInfo.inputAttachments.Empty())
			descriptor_set_layouts[shader->parseInfo.inputAttachments[0].set] = layouts.inputAttachmentDescriptorSetLayout;

		List<VkDescriptorSetLayout> descriptor_set_layouts_sorted;
		for (u32 i = 0; i < descriptor_set_layouts.Size(); i++)
		{
			if (descriptor_set_layouts[i])
				descriptor_set_layouts_sorted.Push(descriptor_set_layouts[i]);
		}

		VkPipelineLayoutCreateInfo pipeline_layout_create_info {};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.setLayoutCount = descriptor_set_layouts_sorted.Size();
		pipeline_layout_create_info.pSetLayouts = &descriptor_set_layouts_sorted[0];
		pipeline_layout_create_info.pushConstantRangeCount = 0;
		pipeline_layout_create_info.pPushConstantRanges = 0;

		EU_CHECK_VKRESULT(vkCreatePipelineLayout(m_Device, &pipeline_layout_create_info, 0, &pipeline->pipelineLayout), "Could not create Vulkan pipeline layout");

		VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
		graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphics_pipeline_create_info.stageCount = 2;
		graphics_pipeline_create_info.pStages = pipeline_shader_stage_create_infos;
		graphics_pipeline_create_info.pVertexInputState = &pipeline_vertex_input_state_create_info;
		graphics_pipeline_create_info.pInputAssemblyState = &pipeline_input_assembly_state_create_info;
		graphics_pipeline_create_info.pViewportState = &pipeline_viewport_state_create_info;
		graphics_pipeline_create_info.pRasterizationState = &pipeline_rasterization_state_create_info;
		graphics_pipeline_create_info.pMultisampleState = &pipeline_multisample_state_create_info;
		graphics_pipeline_create_info.pDepthStencilState = &pipeline_depth_stencil_state_create_info;
		graphics_pipeline_create_info.pColorBlendState = &pipeline_color_blend_state_create_info;
		graphics_pipeline_create_info.pDynamicState = &pipeline_dynamic_state_create_info;
		graphics_pipeline_create_info.layout = pipeline->pipelineLayout;
		graphics_pipeline_create_info.renderPass = renderPass->renderPass;
		graphics_pipeline_create_info.subpass = subpass;
		graphics_pipeline_create_info.basePipelineHandle = 0;
		graphics_pipeline_create_info.basePipelineIndex = -1;

		pipeline->pipelineSettings = pipelineSettings;

		EU_CHECK_VKRESULT(vkCreateGraphicsPipelines(m_Device, 0, 1, &graphics_pipeline_create_info, 0, &pipeline->pipeline), "Could not create Vulkan graphics pipeline");
	}

	void RenderContextVK::InitRenderPass(RenderPassVK* renderPass, const RenderPass& renderPassSettings)
	{
		const Framebuffer& framebufferSettings = renderPassSettings.framebuffer;

		VkAttachmentDescription attachment_descriptions[EU_MAX_FRAMEBUFFER_ATTACHMENTS];
		for (u32 i = 0; i < framebufferSettings.numAttachments; i++)
		{
			const FramebufferAttachment& attachmentSettings = renderPassSettings.framebuffer.attachments[i];

			VkAttachmentDescription attachment_description{};

			TextureFormat format = attachmentSettings.format;

			attachment_description.format = GetVkFormat(format);
			attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;

			if (attachmentSettings.isClearAttachment)
			{
				attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			}
			else
			{
				if (attachmentSettings.nonClearAttachmentPreserve)
				{
					attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

					if (attachmentSettings.isSwapchainAttachment)
					{
						attachment_description.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
					}
					else
					{
						if (attachmentSettings.isSamplerAttachment)
							attachment_description.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						else
							attachment_description.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					}
				}
				else
				{
					attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				}
			}

			if (RenderContext::IsStencilFormat(format))
			{
				if (attachmentSettings.isClearAttachment)
					attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				else
					attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

				if (attachmentSettings.isStoreAttachment)
					attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
				else
					attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			}
			else
			{
				attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			}

			if (attachmentSettings.isSwapchainAttachment)
			{
				if (RenderContext::IsDepthFormat(format) || RenderContext::IsStencilFormat(format))
				{
					EU_LOG_ERROR("Vulkan swapchain output texture cannot be a depth/stencil attachment");
					return;
				}

				attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			}
			else
			{
				if (RenderContext::IsDepthFormat(format) || RenderContext::IsStencilFormat(format))
				{
					if (attachmentSettings.isSamplerAttachment)
						attachment_description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					else
						attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				}
				else
				{
					if (attachmentSettings.isSamplerAttachment)
						attachment_description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					else
						attachment_description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				}

				if (attachmentSettings.isStoreAttachment)
					attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				else
					attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			}

			attachment_descriptions[i] = attachment_description;
		}

		List<VkSubpassDescription> subpass_descriptions(renderPassSettings.subpasses.Size(), renderPassSettings.subpasses.Size());
		List<VkSubpassDependency> subpass_dependencies(renderPassSettings.subpasses.Size() + 1, renderPassSettings.subpasses.Size() + 1);

		List<List<VkAttachmentReference>> write_color_attachment_lists(renderPassSettings.subpasses.Size(), renderPassSettings.subpasses.Size());
		List<List<VkAttachmentReference>> input_color_attachment_lists(renderPassSettings.subpasses.Size(), renderPassSettings.subpasses.Size());
		VkAttachmentReference depth_stencil_attachment_reference;

		for (u32 i = 0; i < renderPassSettings.subpasses.Size(); i++)
		{
			const Subpass& subpassSettings = renderPassSettings.subpasses[i];

			VkSubpassDescription subpass_description{};
			subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

			if (i != 0)
			{
				List<VkAttachmentReference> input_attachment_references(subpassSettings.numReadAttachments, subpassSettings.numReadAttachments);
				for (u32 j = 0; j < subpassSettings.numReadAttachments; j++)
				{
					VkAttachmentReference input_attachment_reference;
					input_attachment_reference.attachment = subpassSettings.readAttachments[j];
					input_attachment_reference.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					input_attachment_references[j] = input_attachment_reference;
				}

				subpass_description.inputAttachmentCount = subpassSettings.numReadAttachments;
				input_color_attachment_lists[i] = input_attachment_references;
				subpass_description.pInputAttachments = &input_color_attachment_lists[i][0];
			}
			else if (i == 0 && subpassSettings.numReadAttachments > 0)
			{
				EU_LOG_WARN("First subpass cannot have any read attachments");
				return;
			}

			if (subpassSettings.numWriteAttachments == 0 && !subpassSettings.useDepthStencilAttachment)
			{
				EU_LOG_WARN("subpass must write to atleast one attachment");
				return;
			}

			if (subpassSettings.numWriteAttachments > 0)
			{
				List<VkAttachmentReference> write_color_attachment_references(subpassSettings.numWriteAttachments, subpassSettings.numWriteAttachments);
				for (u32 j = 0; j < subpassSettings.numWriteAttachments; j++)
				{
					VkAttachmentReference write_attachment_reference;
					write_attachment_reference.attachment = subpassSettings.writeAttachments[j];
					write_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					write_color_attachment_references[j] = write_attachment_reference;
				}

				subpass_description.colorAttachmentCount = subpassSettings.numWriteAttachments;
				write_color_attachment_lists[i] = write_color_attachment_references;
				subpass_description.pColorAttachments = &write_color_attachment_lists[i][0];
			}
			else
			{
				subpass_description.colorAttachmentCount = 0;
			}

			if (subpassSettings.useDepthStencilAttachment)
			{
				depth_stencil_attachment_reference.attachment = subpassSettings.depthStencilAttachment;
				depth_stencil_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				subpass_description.pDepthStencilAttachment = &depth_stencil_attachment_reference;
			}

			subpass_descriptions[i] = subpass_description;

			VkSubpassDependency subpass_dependency{};
			subpass_dependency.dstSubpass = i;
			subpass_dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			if (i == 0)
			{
				subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
				subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				subpass_dependency.srcAccessMask = 0; //TODO: Make sure this doesn't need to be VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
				subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			}
			else
			{
				subpass_dependency.srcSubpass = i - 1;
				subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				subpass_dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				subpass_dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			}

			subpass_dependencies[i] = subpass_dependency;
		}

		VkSubpassDependency subpass_dependency;
		subpass_dependency.srcSubpass = renderPassSettings.subpasses.Size() - 1;
		subpass_dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
		subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		subpass_dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpass_dependency.dstAccessMask = 0;
		subpass_dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		subpass_dependencies[renderPassSettings.subpasses.Size()] = subpass_dependency;

		VkRenderPassCreateInfo render_pass_create_info{};
		render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_create_info.attachmentCount = framebufferSettings.numAttachments;
		render_pass_create_info.pAttachments = attachment_descriptions;
		render_pass_create_info.dependencyCount = subpass_dependencies.Size();
		render_pass_create_info.pDependencies = &subpass_dependencies[0];
		render_pass_create_info.subpassCount = subpass_descriptions.Size();
		render_pass_create_info.pSubpasses = &subpass_descriptions[0];

		EU_CHECK_VKRESULT(vkCreateRenderPass(m_Device, &render_pass_create_info, 0, &renderPass->renderPass), "Could not create Vulkan render pass");
	}

	void RenderContextVK::InitFramebuffer(RenderPassVK* renderPass, const Framebuffer& framebufferSettings)
	{
		renderPass->framebuffers.SetCapacityAndElementCount(1);
		renderPass->numAttachments = framebufferSettings.numAttachments;
		renderPass->framebufferExtent = { framebufferSettings.width, framebufferSettings.height };
		if (framebufferSettings.useSwapchainSize)
			renderPass->framebufferExtent = m_SwapchainExtent;

		for (u32 i = 0; i < framebufferSettings.numAttachments; i++)
		{
			const FramebufferAttachment& attachmentSettings = framebufferSettings.attachments[i];
			FramebufferAttachmentVK* attachment = &renderPass->attachments[i];

			if (attachmentSettings.isSwapchainAttachment)
			{
				if (renderPass->framebuffers.Size() != m_SwapchainImageViews.Size())
					renderPass->framebuffers.SetCapacityAndElementCount(m_SwapchainImageViews.Size());

				attachment->isSwapchainAttachment = true;
			}
			else
			{
				attachment->isSwapchainAttachment = false;

				TextureFormat format = attachmentSettings.format;

				VkFormat formatVK = GetVkFormat(format);
				b32 isDepthStencilFormat = RenderContext::IsDepthFormat(format) || RenderContext::IsStencilFormat(format);

				VkImageUsageFlags imageUsage = isDepthStencilFormat ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

				if (attachmentSettings.isSamplerAttachment)
					imageUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;

				if (attachmentSettings.isSubpassInputAttachment)
					imageUsage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

				if (attachmentSettings.memoryTransferSrc)
					imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

				VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				if (!attachmentSettings.isClearAttachment)
				{
					if (attachmentSettings.isSwapchainAttachment)
						initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				}

				CreateImage(renderPass->framebufferExtent.width, renderPass->framebufferExtent.height, formatVK, VK_IMAGE_TILING_OPTIMAL, imageUsage, initialLayout,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &attachment->image, &attachment->imageMemory);

				attachment->format = format;
				attachment->usage = imageUsage;
				attachment->initialLayout = initialLayout;

				VkImageViewCreateInfo image_view_create_info{};

				image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				image_view_create_info.image = attachment->image;
				image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
				image_view_create_info.format = formatVK;
				image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
				image_view_create_info.subresourceRange.aspectMask = RenderContext::IsDepthFormat(format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
				image_view_create_info.subresourceRange.baseMipLevel = 0;
				image_view_create_info.subresourceRange.levelCount = 1;
				image_view_create_info.subresourceRange.baseArrayLayer = 0;
				image_view_create_info.subresourceRange.layerCount = 1;

				EU_CHECK_VKRESULT(vkCreateImageView(m_Device, &image_view_create_info, 0, &attachment->imageView), "Could not create Vulkan image view");
			}
		}

		VkFramebufferCreateInfo framebuffer_create_info{};
		framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_create_info.renderPass = renderPass->renderPass;
		framebuffer_create_info.attachmentCount = renderPass->numAttachments;
		framebuffer_create_info.width = renderPass->framebufferExtent.width;
		framebuffer_create_info.height = renderPass->framebufferExtent.height;
		framebuffer_create_info.layers = 1;
		framebuffer_create_info.flags = 0;

		for (u32 i = 0; i < renderPass->framebuffers.Size(); i++)
		{
			List<VkImageView> attachments(renderPass->numAttachments, renderPass->numAttachments);
			for (u32 j = 0; j < attachments.Size(); j++)
			{
				if (renderPass->attachments[j].isSwapchainAttachment)
					attachments[j] = m_SwapchainImageViews[i];
				else
					attachments[j] = renderPass->attachments[j].imageView;
			}

			framebuffer_create_info.pAttachments = &attachments[0];
			EU_CHECK_VKRESULT(vkCreateFramebuffer(m_Device, &framebuffer_create_info, 0, &renderPass->framebuffers[i]), "Could not create Vulkan framebuffer");
		}
	}

	void RenderContextVK::InitCommandPool()
	{
		QueueFamilyIndices indices = FindQueueFamilyIndices(m_PhysicalDevice);

		VkCommandPoolCreateInfo command_pool_create_info{};
		command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_create_info.queueFamilyIndex = indices.graphics;
		command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

		EU_CHECK_VKRESULT(vkCreateCommandPool(m_Device, &command_pool_create_info, 0, &m_CommandPool), "Could not create Vulkan command pool");
		EU_LOG_TRACE("Created Vulkan command pool");
	}

	void RenderContextVK::InitFramesInFlight()
	{
		VkSemaphoreCreateInfo semaphore_create_info{};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_create_info{};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkCommandBufferAllocateInfo command_buffer_allocate_info{};
		command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		command_buffer_allocate_info.commandBufferCount = 1;
		command_buffer_allocate_info.commandPool = m_CommandPool;

		for (u32 i = 0; i < EU_VK_MAX_FRAMES_IN_FLIGHT; i++)
		{
			EU_CHECK_VKRESULT(vkCreateSemaphore(m_Device, &semaphore_create_info, 0, &m_FramesInFlight[i].imageAvailableSemaphore), "Could not create Vulkan semaphore");
			EU_CHECK_VKRESULT(vkCreateSemaphore(m_Device, &semaphore_create_info, 0, &m_FramesInFlight[i].renderFinishedSemaphore), "Could not create Vulkan semaphore");
			EU_CHECK_VKRESULT(vkCreateFence(m_Device, &fence_create_info, 0, &m_FramesInFlight[i].fence), "Could not create Vulkan fence");
			EU_CHECK_VKRESULT(vkAllocateCommandBuffers(m_Device, &command_buffer_allocate_info, &m_FramesInFlight[i].commandBuffer), "Could not allocate Vulkan command buffer");
		}

		EU_LOG_TRACE("Created Vulkan frames in flight");
	}

	void RenderContextVK::InitShaderResources(RenderPassVK* renderPass, ShaderResourcesVK* resources, ShaderInputAttachmentResourcesVK* inputAttachmentResources,
		const ShaderParseInfoVK& parseInfo, const u32* readAttachments, const List<MaxTextureGroupBinds>& maxGroupBinds, const List<String>& dynamicBuffers)
	{
		const ShaderLayoutsVK& layouts = resources->layouts;

		u32 maxSets = parseInfo.bufferSets.Size();
		u32 textureCount = 0;

		resources->textureResources.textureGroups.SetCapacityAndElementCount(parseInfo.textureSets.Size());
		for (u32 i = 0; i < parseInfo.textureSets.Size(); i++)
		{
			u32 maxBinds = EU_DEFAULT_MAX_TEXTURE_GROUP_BINDS;
			for (u32 j = 0; j < maxGroupBinds.Size(); j++)
			{
				if (parseInfo.textureSets[i].set == maxGroupBinds[j].set)
				{
					maxBinds = maxGroupBinds[j].maxBinds;
					break;
				}
			}

			resources->textureResources.textureGroups[i].setNumber = parseInfo.textureSets[i].set;

			for (u32 j = 0; j < EU_VK_MAX_FRAMES_IN_FLIGHT; j++)
			{
				resources->textureResources.textureGroups[i].currentOffset[j] = 0;
				resources->textureResources.textureGroups[i].updated[j] = false;
				resources->textureResources.textureGroups[i].descriptorSets[j].SetCapacityAndElementCount(maxBinds);
			}

			maxSets += maxBinds;
			textureCount += parseInfo.textureSets[i].textures.Size();
		}

		List<VkDescriptorPoolSize> descriptor_pool_sizes;

		if (!dynamicBuffers.Empty())
		{
			u32 numDynamicUniformBuffers = 0;
			u32 numDynamicStorageBuffers = 0;

			for (u32 i = 0; i < parseInfo.bufferSets.Size(); i++)
			{
				for (u32 j = 0; j < parseInfo.bufferSets[i].uniformBuffers.Size(); j++)
				{
					for (u32 k = 0; k < dynamicBuffers.Size(); k++)
					{
						if (parseInfo.bufferSets[i].uniformBuffers[j].name == dynamicBuffers[k])
						{
							if (parseInfo.bufferSets[i].uniformBuffers[j].isStorageBuffer)
								numDynamicStorageBuffers++;
							else
								numDynamicUniformBuffers++;
						}
					}
				}
			}

			if (numDynamicUniformBuffers > 0)
			{
				VkDescriptorPoolSize buffer_descriptor_pool_size{};
				buffer_descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				buffer_descriptor_pool_size.descriptorCount = numDynamicUniformBuffers * EU_VK_MAX_FRAMES_IN_FLIGHT;
				descriptor_pool_sizes.Push(buffer_descriptor_pool_size);
			}
			if (numDynamicStorageBuffers > 0)
			{
				VkDescriptorPoolSize buffer_descriptor_pool_size{};
				buffer_descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
				buffer_descriptor_pool_size.descriptorCount = numDynamicStorageBuffers * EU_VK_MAX_FRAMES_IN_FLIGHT;
				descriptor_pool_sizes.Push(buffer_descriptor_pool_size);
			}
		}

		if (!parseInfo.bufferSets.Empty())
		{
			u32 numNonDynamicUniformBuffers = 0;
			u32 numNonDynamicStorageBuffers = 0;
			for (u32 i = 0; i < parseInfo.bufferSets.Size(); i++)
			{
				for (u32 j = 0; j < parseInfo.bufferSets[i].uniformBuffers.Size(); j++)
				{
					b32 isDynamic = false;
					for (u32 k = 0; k < dynamicBuffers.Size(); k++)
					{
						if (parseInfo.bufferSets[i].uniformBuffers[j].name == dynamicBuffers[k])
						{
							isDynamic = true;
							break;
						}
					}

					if (!isDynamic)
					{
						if (parseInfo.bufferSets[i].uniformBuffers[j].isStorageBuffer)
							numNonDynamicStorageBuffers++;
						else
							numNonDynamicUniformBuffers++;
					}
				}
			}

			if (numNonDynamicUniformBuffers > 0)
			{
				VkDescriptorPoolSize buffer_descriptor_pool_size{};
				buffer_descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				buffer_descriptor_pool_size.descriptorCount = numNonDynamicUniformBuffers * EU_VK_MAX_FRAMES_IN_FLIGHT;
				descriptor_pool_sizes.Push(buffer_descriptor_pool_size);
			}
			if (numNonDynamicStorageBuffers > 0)
			{
				VkDescriptorPoolSize buffer_descriptor_pool_size{};
				buffer_descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				buffer_descriptor_pool_size.descriptorCount = numNonDynamicStorageBuffers * EU_VK_MAX_FRAMES_IN_FLIGHT;
				descriptor_pool_sizes.Push(buffer_descriptor_pool_size);
			}
		}

		if (textureCount > 0)
		{
			VkDescriptorPoolSize texture_descriptor_pool_size {};
			texture_descriptor_pool_size.descriptorCount = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			texture_descriptor_pool_size.descriptorCount = textureCount * EU_VK_MAX_FRAMES_IN_FLIGHT;
			descriptor_pool_sizes.Push(texture_descriptor_pool_size);
		}

		if (!inputAttachmentResources->initialized)
		{
			if (!parseInfo.inputAttachments.Empty())
			{
				maxSets++;
				inputAttachmentResources->hasInputAttachments = true;

				VkDescriptorPoolSize input_attachment_descriptor_pool_size {};
				input_attachment_descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
				input_attachment_descriptor_pool_size.descriptorCount = parseInfo.inputAttachments.Size() * EU_VK_MAX_FRAMES_IN_FLIGHT;
				descriptor_pool_sizes.Push(input_attachment_descriptor_pool_size);
			}
			else
			{
				inputAttachmentResources->hasInputAttachments = false;
			}
		}

		if (!descriptor_pool_sizes.Empty())
		{
			VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
			descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptor_pool_create_info.maxSets = maxSets * EU_VK_MAX_FRAMES_IN_FLIGHT;
			descriptor_pool_create_info.poolSizeCount = descriptor_pool_sizes.Size();
			descriptor_pool_create_info.pPoolSizes = &descriptor_pool_sizes[0];
			if(!parseInfo.inputAttachments.Empty())
				descriptor_pool_create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

			EU_CHECK_VKRESULT(vkCreateDescriptorPool(m_Device, &descriptor_pool_create_info, 0, &resources->descriptorPool), "Could not create Vulkan descriptor pool");
		}


		if(!parseInfo.bufferSets.Empty())
			resources->bufferResources.bufferSets.SetCapacityAndElementCount(parseInfo.bufferSets.Size());
		
		List<VkWriteDescriptorSet> texture_write_descriptor_sets;

		for (u32 i = 0; i < EU_VK_MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (!parseInfo.bufferSets.Empty())
			{
				resources->bufferResources.descriptorSets[i].SetCapacityAndElementCount(parseInfo.bufferSets.Size());

				VkDescriptorSetAllocateInfo buffer_descriptor_set_allocate_info{};
				buffer_descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				buffer_descriptor_set_allocate_info.descriptorPool = resources->descriptorPool;
				buffer_descriptor_set_allocate_info.descriptorSetCount = parseInfo.bufferSets.Size();
				buffer_descriptor_set_allocate_info.pSetLayouts = &layouts.bufferDescriptorSetLayouts[0];

				EU_CHECK_VKRESULT(vkAllocateDescriptorSets(m_Device, &buffer_descriptor_set_allocate_info, &resources->bufferResources.descriptorSets[i][0]), "Could not allocate Vulkan buffer descriptor sets");
			}

			if (!parseInfo.textureSets.Empty())
			{
				VkDescriptorSetAllocateInfo texture_descriptor_set_allocate_info{};
				texture_descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				texture_descriptor_set_allocate_info.descriptorPool = resources->descriptorPool;

				for (u32 j = 0; j < parseInfo.textureSets.Size(); j++)
				{
					u32 maxBinds = resources->textureResources.textureGroups[j].descriptorSets[i].Size();

					List<VkDescriptorSetLayout> a_list_of_the_same_layouts(maxBinds, maxBinds);
					for (u32 k = 0; k < maxBinds; k++)
						a_list_of_the_same_layouts[k] = layouts.textureDescriptorSetLayouts[j];

					texture_descriptor_set_allocate_info.descriptorSetCount = maxBinds;
					texture_descriptor_set_allocate_info.pSetLayouts = &a_list_of_the_same_layouts[0];

					EU_CHECK_VKRESULT(vkAllocateDescriptorSets(m_Device, &texture_descriptor_set_allocate_info, &resources->textureResources.textureGroups[j].descriptorSets[i][0]),
						"Could not allocate Vulkan texture descriptor sets");

					for (u32 k = 0; k < parseInfo.textureSets[j].textures.Size(); k++)
					{
						VkWriteDescriptorSet write_descriptor_set{};
						write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
						write_descriptor_set.descriptorCount = parseInfo.textureSets[j].textures[k].arrayCount;
						write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
						write_descriptor_set.dstArrayElement = 0;
						write_descriptor_set.dstBinding = parseInfo.textureSets[j].textures[k].binding;

						for (u32 l = 0; l < maxBinds; l++)
						{
							write_descriptor_set.dstSet = resources->textureResources.textureGroups[j].descriptorSets[i][l];
							texture_write_descriptor_sets.Push(write_descriptor_set);
						}
					}
				}
			}
		}

		VkDescriptorImageInfo descriptor_image_info {};
		descriptor_image_info.sampler = m_DefaultSampler.sampler;
		descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		descriptor_image_info.imageView = m_DefaultTexture.imageView;

		List<VkDescriptorImageInfo> descriptor_image_infos;
		for (u32 i = 0; i < texture_write_descriptor_sets.Size(); i++)
		{
			while (descriptor_image_infos.Size() < texture_write_descriptor_sets[i].descriptorCount)
				descriptor_image_infos.Push(descriptor_image_info);

			texture_write_descriptor_sets[i].pImageInfo = &descriptor_image_infos[0];
		}

		if (!texture_write_descriptor_sets.Empty())
			vkUpdateDescriptorSets(m_Device, texture_write_descriptor_sets.Size(), &texture_write_descriptor_sets[0], 0, 0);

		if (!parseInfo.inputAttachments.Empty() && !inputAttachmentResources->initialized)
		{
			inputAttachmentResources->setNumber = parseInfo.inputAttachments[0].set;
			inputAttachmentResources->descriptorPool = resources->descriptorPool;
			inputAttachmentResources->setLayout = layouts.inputAttachmentDescriptorSetLayout;
			inputAttachmentResources->parseInfos = parseInfo.inputAttachments;
			for (u32 i = 0; i < EU_MAX_FRAMEBUFFER_ATTACHMENTS; i++)
				inputAttachmentResources->readAttachments[i] = readAttachments[i];

			VkDescriptorSetLayout inputAttachmentLayout[EU_VK_MAX_FRAMES_IN_FLIGHT];
			for (u32 i = 0; i < EU_VK_MAX_FRAMES_IN_FLIGHT; i++)
				inputAttachmentLayout[i] = layouts.inputAttachmentDescriptorSetLayout;

			VkDescriptorSetAllocateInfo input_attachment_descriptor_set_allocate_info {};
			input_attachment_descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			input_attachment_descriptor_set_allocate_info.descriptorPool = resources->descriptorPool;
			input_attachment_descriptor_set_allocate_info.descriptorSetCount = EU_VK_MAX_FRAMES_IN_FLIGHT;
			input_attachment_descriptor_set_allocate_info.pSetLayouts = inputAttachmentLayout;

			EU_CHECK_VKRESULT(vkAllocateDescriptorSets(m_Device, &input_attachment_descriptor_set_allocate_info, inputAttachmentResources->descriptorSet),
				"Could not allocate Vulkan descriptor sets");

			List<VkWriteDescriptorSet> input_attachment_write_descriptor_sets;
			List<VkDescriptorImageInfo> input_attachment_descriptor_image_infos;

			VkWriteDescriptorSet input_attachment_write_descriptor_set {};
			input_attachment_write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			input_attachment_write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			input_attachment_write_descriptor_set.descriptorCount = 1;
			input_attachment_write_descriptor_set.dstArrayElement = 0;

			b32 allSameSetNumber = true;
			for (u32 i = 0; i < parseInfo.inputAttachments.Size(); i++)
			{
				if (parseInfo.inputAttachments[i].set != inputAttachmentResources->setNumber)
					allSameSetNumber = false;

				const ShaderParseInfoInputAttachmentVK& inputAttachmentInfo = parseInfo.inputAttachments[i];
				
				input_attachment_write_descriptor_set.dstBinding = inputAttachmentInfo.binding;

				VkDescriptorImageInfo input_attachment_descriptor_image_info {};
				input_attachment_descriptor_image_info.sampler = 0;
				input_attachment_descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				input_attachment_descriptor_image_info.imageView = renderPass->attachments[readAttachments[inputAttachmentInfo.attachmentIndex]].imageView;

				for (u32 j = 0; j < EU_VK_MAX_FRAMES_IN_FLIGHT; j++)
				{
					input_attachment_write_descriptor_sets.Push(input_attachment_write_descriptor_set);
					input_attachment_descriptor_image_infos.Push(input_attachment_descriptor_image_info);
				}
			}

			if (!allSameSetNumber)
			{
				EU_LOG_ERROR("Input attachments in shader must all belong to the same set number");
				return;
			}

			for (u32 i = 0; i < input_attachment_write_descriptor_sets.Size(); i++)
			{
				input_attachment_write_descriptor_sets[i].dstSet = inputAttachmentResources->descriptorSet[i % EU_VK_MAX_FRAMES_IN_FLIGHT];
				input_attachment_write_descriptor_sets[i].pImageInfo = &input_attachment_descriptor_image_infos[i];
			}

			vkUpdateDescriptorSets(m_Device, input_attachment_write_descriptor_sets.Size(), &input_attachment_write_descriptor_sets[0], 0, 0);
		}

		for (u32 i = 0; i < parseInfo.bufferSets.Size(); i++)
		{
			u32 numDynamicBuffersInSet = 0;
			for (u32 j = 0; j < parseInfo.bufferSets[i].uniformBuffers.Size(); j++)
			{
				for (u32 k = 0; k < dynamicBuffers.Size(); k++)
				{
					if (parseInfo.bufferSets[i].uniformBuffers[j].name == dynamicBuffers[k])
					{
						numDynamicBuffersInSet++;
						break;
					}
				}
			}

			resources->bufferResources.bufferSets[i].setNumber = parseInfo.bufferSets[i].set;
			resources->bufferResources.bufferSets[i].dynamicOffsets.SetCapacityAndElementCount(numDynamicBuffersInSet);
			resources->bufferResources.bufferSets[i].buffers.SetCapacityAndElementCount(parseInfo.bufferSets[i].uniformBuffers.Size());

			for (u32 j = 0; j < parseInfo.bufferSets[i].uniformBuffers.Size(); j++)
			{
				resources->bufferResources.bufferSets[i].buffers[j].binding = parseInfo.bufferSets[i].uniformBuffers[j].binding;
				resources->bufferResources.bufferSets[i].buffers[j].name = parseInfo.bufferSets[i].uniformBuffers[j].name;
				resources->bufferResources.bufferSets[i].buffers[j].buffer = EU_INVALID_UNIFORM_BUFFER_ID;
			}

			
		}

		inputAttachmentResources->initialized = true;
	}

	void RenderContextVK::InitDefaultTextureAndSampler()
	{
		u8 pixels[4];
		memset(pixels, 255, 4);

		m_DefaultTexture.width = 1;
		m_DefaultTexture.height = 1;
		m_DefaultTexture.depth = 1;
		m_DefaultTexture.format = TEXTURE_FORMAT_RGBA8_UNORM;
		m_DefaultTexture.path = "_NoPath";

		CreateTexture2DHelper(pixels, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, &m_DefaultTexture, false);

		VkSamplerCreateInfo sampler_create_info {};
		sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_create_info.minFilter = VK_FILTER_NEAREST;
		sampler_create_info.magFilter = VK_FILTER_NEAREST;
		sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		sampler_create_info.anisotropyEnable = false;
		sampler_create_info.maxAnisotropy = 0.0f;
		sampler_create_info.compareEnable = VK_FALSE;
		sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
		//TODO: MIPMAP
		sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_create_info.mipLodBias = 0.0f;
		sampler_create_info.minLod = 0.0f;
		sampler_create_info.maxLod = 0.0f;

		EU_CHECK_VKRESULT(vkCreateSampler(m_Device, &sampler_create_info, 0, &m_DefaultSampler.sampler), "Could not create Vulkan sampler");
	}

	void RenderContextVK::DoFramebufferResize(const ResizeFramebufferAtEndOfFrameVK& resize)
	{
		RenderPassID renderPass = resize.renderPass;
		u32 width = resize.width;
		u32 height = resize.height;
		vkDeviceWaitIdle(m_Device);

		RenderPassVK* renderPassVK = &m_RenderPasses[renderPass - 1];

		if (renderPassVK->framebuffers.Size() > 1)
			return;

		vkDestroyFramebuffer(m_Device, renderPassVK->framebuffers[0], 0);
		List<VkImageView> attachments(renderPassVK->numAttachments, renderPassVK->numAttachments);
		for (u32 i = 0; i < renderPassVK->numAttachments; i++)
		{
			FramebufferAttachmentVK* attachment = &renderPassVK->attachments[i];

			vkDestroyImage(m_Device, attachment->image, 0);
			vkDestroyImageView(m_Device, attachment->imageView, 0);
			vkFreeMemory(m_Device, attachment->imageMemory, 0);

			renderPassVK->framebufferExtent.width = width;
			renderPassVK->framebufferExtent.height = height;

			VkFormat formatVK = GetVkFormat(attachment->format);

			CreateImage(width, height, formatVK, VK_IMAGE_TILING_OPTIMAL, attachment->usage, attachment->initialLayout,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &attachment->image, &attachment->imageMemory);

			VkImageViewCreateInfo image_view_create_info{};

			image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_view_create_info.image = attachment->image;
			image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			image_view_create_info.format = formatVK;
			image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.subresourceRange.aspectMask = RenderContext::IsDepthFormat(attachment->format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
			image_view_create_info.subresourceRange.baseMipLevel = 0;
			image_view_create_info.subresourceRange.levelCount = 1;
			image_view_create_info.subresourceRange.baseArrayLayer = 0;
			image_view_create_info.subresourceRange.layerCount = 1;

			EU_CHECK_VKRESULT(vkCreateImageView(m_Device, &image_view_create_info, 0, &attachment->imageView), "Could not recreate Vulkan image view");

			attachments[i] = attachment->imageView;

			for (u32 j = 0; j < attachment->texturesThatPointToThisAttachment.Size(); j++)
			{
				TextureVK* texture = &m_Textures[attachment->texturesThatPointToThisAttachment[j] - 1];
				texture->image = attachment->image;
				texture->imageView = attachment->imageView;
				texture->memory = attachment->imageMemory;
				texture->width = width;
				texture->height = height;
			}
		}

		for (u32 i = 0; i < renderPassVK->subpasses.Size(); i++)
		{
			SubpassVK* subpass = &renderPassVK->subpasses[i];

			for (u32 j = 0; j < subpass->pipelines.Size(); j++)
			{
				GraphicsPipelineVK* pipeline = &subpass->pipelines[j];
				if (pipeline->pipelineSettings.viewportState.useFramebufferSizeForScissor ||
					pipeline->pipelineSettings.viewportState.useFramebufferSizeForViewport)
				{
					vkDestroyPipelineLayout(m_Device, pipeline->pipelineLayout, 0);
					vkDestroyPipeline(m_Device, pipeline->pipeline, 0);
					InitGraphicsPipeline(pipeline, pipeline->pipelineSettings, renderPassVK, i);
				}
			}

			if (subpass->inputAttachmentResources.hasInputAttachments)
			{
				ShaderInputAttachmentResourcesVK* inputAttachmentResources = &subpass->inputAttachmentResources;

				EU_CHECK_VKRESULT(vkFreeDescriptorSets(m_Device, inputAttachmentResources->descriptorPool, EU_VK_MAX_FRAMES_IN_FLIGHT, inputAttachmentResources->descriptorSet),
					"Could not free Vulkan input attachment descriptor sets");

				VkDescriptorSetLayout inputAttachmentLayout[EU_VK_MAX_FRAMES_IN_FLIGHT];
				for (u32 i = 0; i < EU_VK_MAX_FRAMES_IN_FLIGHT; i++)
					inputAttachmentLayout[i] = inputAttachmentResources->setLayout;

				VkDescriptorSetAllocateInfo descriptor_set_allocate_info{ };
				descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				descriptor_set_allocate_info.descriptorPool = inputAttachmentResources->descriptorPool;
				descriptor_set_allocate_info.descriptorSetCount = EU_VK_MAX_FRAMES_IN_FLIGHT;
				descriptor_set_allocate_info.pSetLayouts = inputAttachmentLayout;

				EU_CHECK_VKRESULT(vkAllocateDescriptorSets(m_Device, &descriptor_set_allocate_info, inputAttachmentResources->descriptorSet),
					"Could not allocate Vulkan descriptor sets");

				List<VkWriteDescriptorSet> input_attachment_write_descriptor_sets;
				List<VkDescriptorImageInfo> input_attachment_descriptor_image_infos;

				VkWriteDescriptorSet input_attachment_write_descriptor_set{};
				input_attachment_write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				input_attachment_write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
				input_attachment_write_descriptor_set.descriptorCount = 1;
				input_attachment_write_descriptor_set.dstArrayElement = 0;

				for (u32 i = 0; i < inputAttachmentResources->parseInfos.Size(); i++)
				{

					const ShaderParseInfoInputAttachmentVK& inputAttachmentInfo = inputAttachmentResources->parseInfos[i];

					input_attachment_write_descriptor_set.dstBinding = inputAttachmentInfo.binding;

					VkDescriptorImageInfo input_attachment_descriptor_image_info{};
					input_attachment_descriptor_image_info.sampler = 0;
					input_attachment_descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					input_attachment_descriptor_image_info.imageView = renderPassVK->attachments[inputAttachmentResources->readAttachments[inputAttachmentInfo.attachmentIndex]].imageView;

					for (u32 j = 0; j < EU_VK_MAX_FRAMES_IN_FLIGHT; j++)
					{
						input_attachment_write_descriptor_sets.Push(input_attachment_write_descriptor_set);
						input_attachment_descriptor_image_infos.Push(input_attachment_descriptor_image_info);
					}
				}

				for (u32 i = 0; i < input_attachment_write_descriptor_sets.Size(); i++)
				{
					input_attachment_write_descriptor_sets[i].dstSet = inputAttachmentResources->descriptorSet[i % EU_VK_MAX_FRAMES_IN_FLIGHT];
					input_attachment_write_descriptor_sets[i].pImageInfo = &input_attachment_descriptor_image_infos[i];
				}

				vkUpdateDescriptorSets(m_Device, input_attachment_write_descriptor_sets.Size(), &input_attachment_write_descriptor_sets[0], 0, 0);
			}
		}

		VkFramebufferCreateInfo framebuffer_create_info{};
		framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_create_info.width = width;
		framebuffer_create_info.height = height;
		framebuffer_create_info.layers = 1;
		framebuffer_create_info.pAttachments = &attachments[0];
		framebuffer_create_info.attachmentCount = attachments.Size();
		framebuffer_create_info.renderPass = renderPassVK->renderPass;

		EU_CHECK_VKRESULT(vkCreateFramebuffer(m_Device, &framebuffer_create_info, 0, &renderPassVK->framebuffers[0]), "Error recreating renderpass framebuffer");
	}

	void RenderContextVK::DestroySwapchainResources()
	{
		m_CurrentFrame = 0;
		m_ImageIndex = 0;
		vkDeviceWaitIdle(m_Device);
		for (u32 i = 0; i < m_SwapchainImageViews.Size(); i++)
			vkDestroyImageView(m_Device, m_SwapchainImageViews[i], 0);

		for (u32 i = 0; i < m_RenderPasses.Size(); i++)
		{
			RenderPassVK* renderPass = &m_RenderPasses[i];

			//If there is more that one frame buffer then this renderpass renders to the swapchain
			if (renderPass->framebuffers.Size() > 1)
			{
				for (u32 j = 0; j < renderPass->framebuffers.Size(); j++)
					vkDestroyFramebuffer(m_Device, renderPass->framebuffers[j], 0);

				for (u32 j = 0; j < renderPass->numAttachments; j++)
				{
					if (renderPass->attachments[j].isSwapchainAttachment)
						continue;

					vkDestroyImage(m_Device, renderPass->attachments[j].image, 0);
					vkDestroyImageView(m_Device, renderPass->attachments[j].imageView, 0);
					vkFreeMemory(m_Device, renderPass->attachments[j].imageMemory, 0);
				}

				for (u32 j = 0; j < renderPass->subpasses.Size(); j++)
				{
					SubpassVK* subpass = &renderPass->subpasses[j];

					for (u32 k = 0; k < subpass->pipelines.Size(); k++)
					{
						GraphicsPipelineVK* pipeline = &subpass->pipelines[k];
						vkDestroyPipelineLayout(m_Device, pipeline->pipelineLayout, 0);
						vkDestroyPipeline(m_Device, pipeline->pipeline, 0);
					}
				}
			}
		}

		vkDestroySwapchainKHR(m_Device, m_Swapchain, 0);
	}

	void RenderContextVK::RecreateSwapchainResources(u32 width, u32 height)
	{
		InitSwapchain();
		InitSwapchainImageViews();

		for (u32 i = 0; i < m_RenderPasses.Size(); i++)
		{
			RenderPassVK* renderPass = &m_RenderPasses[i];

			//If there is more that one frame buffer then this renderpass renders to the swapchain
			if (renderPass->framebuffers.Size() > 1)
			{
				renderPass->framebufferExtent = { width, height };
				for (u32 j = 0; j < renderPass->framebuffers.Size(); j++)
				{
					List<VkImageView> attachments(renderPass->numAttachments, renderPass->numAttachments);
					for (u32 k = 0; k < renderPass->numAttachments; k++)
					{
						FramebufferAttachmentVK* attachment = &renderPass->attachments[k];
						
						if (attachment->isSwapchainAttachment)
						{
							attachments[k] = m_SwapchainImageViews[j];
						}
						else
						{
							VkFormat formatVK = GetVkFormat(attachment->format);

							CreateImage(width, height, formatVK, VK_IMAGE_TILING_OPTIMAL, attachment->usage, attachment->initialLayout,
								VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &attachment->image, &attachment->imageMemory);

							VkImageViewCreateInfo image_view_create_info{};

							image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
							image_view_create_info.image = attachment->image;
							image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
							image_view_create_info.format = formatVK;
							image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
							image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
							image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
							image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
							image_view_create_info.subresourceRange.aspectMask = RenderContext::IsDepthFormat(attachment->format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
							image_view_create_info.subresourceRange.baseMipLevel = 0;
							image_view_create_info.subresourceRange.levelCount = 1;
							image_view_create_info.subresourceRange.baseArrayLayer = 0;
							image_view_create_info.subresourceRange.layerCount = 1;

							EU_CHECK_VKRESULT(vkCreateImageView(m_Device, &image_view_create_info, 0, &attachment->imageView), "Could not recreate Vulkan image view");

							attachments[k] = attachment->imageView;
						}

						for (u32 l = 0; l < attachment->texturesThatPointToThisAttachment.Size(); l++)
						{
							TextureVK* texture = &m_Textures[attachment->texturesThatPointToThisAttachment[l] - 1];
							texture->width = width;
							texture->height = height;
							texture->image = attachment->image;
							texture->memory = attachment->imageMemory;
							texture->imageView = attachment->imageView;
						}
					}

					VkFramebufferCreateInfo framebuffer_create_info{};
					framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
					framebuffer_create_info.width = width;
					framebuffer_create_info.height = height;
					framebuffer_create_info.layers = 1;
					framebuffer_create_info.pAttachments = &attachments[0];
					framebuffer_create_info.attachmentCount = attachments.Size();
					framebuffer_create_info.renderPass = renderPass->renderPass;

					EU_CHECK_VKRESULT(vkCreateFramebuffer(m_Device, &framebuffer_create_info, 0, &renderPass->framebuffers[j]), "Error recreating swapchain framebuffer");
				}

				for (u32 j = 0; j < renderPass->subpasses.Size(); j++)
				{
					SubpassVK* subpass = &renderPass->subpasses[j];

					for (u32 k = 0; k < subpass->pipelines.Size(); k++)
					{
						GraphicsPipelineVK* pipeline = &subpass->pipelines[k];
						InitGraphicsPipeline(pipeline, pipeline->pipelineSettings, renderPass, j);
					}
				}
			}
		}
	}

	void RenderContextVK::CreateTexture2DHelper(const u8* pixels, u32 width, u32 height, VkFormat format, TextureVK* texture, b32 isFramebufferAttachment)
	{
		VkBuffer buffer;
		VkDeviceMemory memory;
		mem_size size;

		u32 imageSize = width * height * GetPixelSizeFromFormat(format);
		CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &buffer, &memory, &size);

		if (pixels)
		{
			void* data;
			vkMapMemory(m_Device, memory, 0, size, 0, &data);
			memcpy(data, pixels, imageSize);
			vkUnmapMemory(m_Device, memory);
		}

		VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		if (isFramebufferAttachment)
			imageUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		CreateImage(width, height, format, VK_IMAGE_TILING_OPTIMAL, imageUsage, VK_IMAGE_LAYOUT_UNDEFINED, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture->image, &texture->memory);

		CmdTransitionImageLayout(texture->image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		CmdCopyBufferToImage(buffer, texture->image, width, height);
		CmdTransitionImageLayout(texture->image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkFreeMemory(m_Device, memory, 0);
		vkDestroyBuffer(m_Device, buffer, 0);

		VkImageViewCreateInfo image_view_create_info = {};
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.image = texture->image;
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format = format;
		image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_create_info.subresourceRange.baseMipLevel = 0;
		image_view_create_info.subresourceRange.levelCount = 1;
		image_view_create_info.subresourceRange.baseArrayLayer = 0;
		image_view_create_info.subresourceRange.layerCount = 1;

		EU_CHECK_VKRESULT(vkCreateImageView(m_Device, &image_view_create_info, 0, &texture->imageView), "Could not create Vulkan image view");
	}

	void RenderContextVK::CreateImage(u32 width, u32 height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usageFlags, VkImageLayout initialLayout, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* memory)
	{
		VkImageCreateInfo image_create_info{};
		image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_create_info.imageType = VK_IMAGE_TYPE_2D;
		image_create_info.extent.width = width;
		image_create_info.extent.height = height;
		image_create_info.extent.depth = 1;
		image_create_info.mipLevels = 1;
		image_create_info.arrayLayers = 1;
		image_create_info.format = format;
		image_create_info.tiling = tiling;
		image_create_info.initialLayout = initialLayout;
		image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		image_create_info.usage = usageFlags;
		image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_create_info.flags = 0;

		EU_CHECK_VKRESULT(vkCreateImage(m_Device, &image_create_info, 0, image), "Could not create Vulkan image");

		VkMemoryRequirements memory_requirments;
		vkGetImageMemoryRequirements(m_Device, *image, &memory_requirments);

		s32 memoryTypeIndex = FindMemoryType(memory_requirments.memoryTypeBits, properties);

		if (memoryTypeIndex == -1)
		{
			EU_LOG_ERROR("Could not find correct memory type to allocate Vulkan image");
			return;
		}

		VkMemoryAllocateInfo memory_allocate_info{};
		memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memory_allocate_info.allocationSize = memory_requirments.size;
		memory_allocate_info.memoryTypeIndex = memoryTypeIndex;

		EU_CHECK_VKRESULT(vkAllocateMemory(m_Device, &memory_allocate_info, 0, memory), "Could not allocate Vulkan image memory");
		vkBindImageMemory(m_Device, *image, *memory, 0);
	}

	s32 RenderContextVK::FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags memoryProperties)
	{
		VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
		vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &physical_device_memory_properties);

		for (u32 i = 0; i < physical_device_memory_properties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (physical_device_memory_properties.memoryTypes[i].propertyFlags & memoryProperties) == memoryProperties)
				return i;
		}

		return -1;
	}

	void RenderContextVK::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* memory, mem_size* actualSize)
	{
		VkBufferCreateInfo buffer_create_info{};
		buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size = size;
		buffer_create_info.usage = usage;
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		buffer_create_info.flags = 0;

		EU_CHECK_VKRESULT(vkCreateBuffer(m_Device, &buffer_create_info, 0, buffer), "Could not create Vulkan buffer");

		VkMemoryRequirements memory_requirements{};
		vkGetBufferMemoryRequirements(m_Device, *buffer, &memory_requirements);
		*actualSize = memory_requirements.size;

		s32 memoryTypeIndex = FindMemoryType(memory_requirements.memoryTypeBits, properties);
		if (memoryTypeIndex == -1)
		{
			EU_LOG_ERROR("Could not find suitable memory type to allocate Vulkan buffer");
			return;
		}

		VkMemoryAllocateInfo memory_allocate_info{};
		memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memory_allocate_info.allocationSize = memory_requirements.size;
		memory_allocate_info.memoryTypeIndex = memoryTypeIndex;

		EU_CHECK_VKRESULT(vkAllocateMemory(m_Device, &memory_allocate_info, 0, memory), "Could not allocate Vulkan buffer memory");
		EU_CHECK_VKRESULT(vkBindBufferMemory(m_Device, *buffer, *memory, 0), "Could not bind Vulkan memory to buffer");
	}

	VkCommandBuffer RenderContextVK::BeginCommands() const
	{
		VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
		command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_allocate_info.commandPool = m_CommandPool;
		command_buffer_allocate_info.commandBufferCount = 1;
		command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		VkCommandBuffer command_buffer;
		vkAllocateCommandBuffers(m_Device, &command_buffer_allocate_info, &command_buffer);
		VkCommandBufferBeginInfo command_buffer_begin_info = {};
		command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);

		return command_buffer;
	}

	void RenderContextVK::EndCommands(VkCommandBuffer commandBuffer) const
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(m_GraphicsQueue, 1, &submit_info, 0);
		vkQueueWaitIdle(m_GraphicsQueue);
		vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &commandBuffer);
	}

	void RenderContextVK::CmdCopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size) const
	{
		VkCommandBuffer command_buffer = BeginCommands();

		VkBufferCopy buffer_copy = {};
		buffer_copy.srcOffset = 0;
		buffer_copy.dstOffset = 0;
		buffer_copy.size = size;
		vkCmdCopyBuffer(command_buffer, src, dst, 1, &buffer_copy);
		EndCommands(command_buffer);
	}

	void RenderContextVK::CmdTransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkCommandBuffer command_buffer = BeginCommands();

		VkImageMemoryBarrier image_memory_barrier = {};
		image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		image_memory_barrier.oldLayout = oldLayout;
		image_memory_barrier.newLayout = newLayout;
		image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		image_memory_barrier.image = image;
		image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_memory_barrier.subresourceRange.baseMipLevel = 0;
		image_memory_barrier.subresourceRange.levelCount = 1;
		image_memory_barrier.subresourceRange.baseArrayLayer = 0;
		image_memory_barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;
		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = 0;
			image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			//TODO: change when textures can be used in other shader types, like tesselation
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = 0;
			image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
		{
			EU_LOG_WARN("Unsupported image layout transition");
			return;
		}

		vkCmdPipelineBarrier(command_buffer, srcStage, dstStage, 0, 0, 0, 0, 0, 1, &image_memory_barrier);
		EndCommands(command_buffer);
	}

	void RenderContextVK::CmdCopyBufferToImage(VkBuffer src, VkImage dst, u32 width, u32 height)
	{
		VkCommandBuffer buffer = BeginCommands();

		VkBufferImageCopy buffer_image_copy = {};
		buffer_image_copy.bufferOffset = 0;
		buffer_image_copy.bufferRowLength = 0;
		buffer_image_copy.bufferImageHeight = 0;
		buffer_image_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		buffer_image_copy.imageSubresource.baseArrayLayer = 0;
		buffer_image_copy.imageSubresource.mipLevel = 0;
		buffer_image_copy.imageSubresource.layerCount = 1;
		buffer_image_copy.imageOffset = { 0, 0, 0 };
		buffer_image_copy.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(buffer, src, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_image_copy);

		EndCommands(buffer);
	}

	VkFormat RenderContextVK::GetVkFormat(TextureFormat format)
	{
		switch (format)
		{
		case TEXTURE_FORMAT_R8_UNORM: return VK_FORMAT_R8_UNORM;
		case TEXTURE_FORMAT_R16_FLOAT: return VK_FORMAT_R16_SFLOAT;
		case TEXTURE_FORMAT_R32_FLOAT: return VK_FORMAT_R32_SFLOAT;
		case TEXTURE_FORMAT_RGBA8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
		case TEXTURE_FORMAT_RGBA8_SRGB: return VK_FORMAT_R8G8B8A8_SRGB;
		case TEXTURE_FORMAT_RGBA16_UNORM: return VK_FORMAT_R16G16B16A16_UNORM;
		case TEXTURE_FORMAT_RGBA16_FLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
		case TEXTURE_FORMAT_RGBA32_FLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
		case TEXTURE_FORMAT_RGBA64_FLOAT: return VK_FORMAT_R64G64B64A64_SFLOAT;
		case TEXTURE_FORMAT_DEPTH32_FLOAT_STENCIL8_UINT: return VK_FORMAT_D32_SFLOAT_S8_UINT;
		case TEXTURE_FORMAT_DEPTH32_FLOAT: return VK_FORMAT_D32_SFLOAT;
		case TEXTURE_FORMAT_SWAPCHAIN_FORMAT: return m_SwapchainFormat;
		}
		
		return VK_FORMAT_R8G8B8A8_UNORM;
	}

	u32 RenderContextVK::GetPixelSizeFromFormat(VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_R8_UNORM: return sizeof(u8);
		case VK_FORMAT_R16_SFLOAT: return sizeof(u16);
		case VK_FORMAT_R32_SFLOAT: return sizeof(r32);
		case VK_FORMAT_R8G8B8A8_UNORM: return sizeof(u8) * 4;
		case VK_FORMAT_R8G8B8A8_SRGB: return sizeof(u8) * 4;
		case VK_FORMAT_R16G16B16A16_UNORM: return sizeof(u16) * 4;
		case VK_FORMAT_R16G16B16A16_SFLOAT: return sizeof(u16) * 4;
		case VK_FORMAT_R32G32B32A32_SFLOAT: return sizeof(r32) * 4;
		case VK_FORMAT_D32_SFLOAT_S8_UINT: return sizeof(r32) * sizeof(u8);
		case VK_FORMAT_D32_SFLOAT: return sizeof(r32);
		}
	}

	void RenderContextVK::VulkanDisplayCallback(const DisplayEvent& e, void* userPtr)
	{
		RenderContextVK* rc = (RenderContextVK*)userPtr;
		if (e.type == DISPLAY_EVENT_RESIZE)
		{
			rc->DestroySwapchainResources();
			rc->RecreateSwapchainResources(e.width, e.height);
		}
	}

	VkPrimitiveTopology RenderContextVK::GetVkPrimitiveTopology(PrimitiveTopology topology)
	{
		switch (topology)
		{
		case PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}

		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	}

	VkCullModeFlags RenderContextVK::GetVkCullMode(CullMode cullMode)
	{
		switch (cullMode)
		{
		case CULL_MODE_FRONT: return VK_CULL_MODE_FRONT_BIT;
		case CULL_MODE_BACK: return VK_CULL_MODE_BACK_BIT;
		case CULL_MODE_FRONT_BACK: return VK_CULL_MODE_FRONT_AND_BACK;
		case CULL_MODE_NONE: return VK_CULL_MODE_NONE;
		}

		return VK_CULL_MODE_NONE;
	}

	VkPolygonMode RenderContextVK::GetVkPolygonMode(PolygonMode polygonMode)
	{
		switch (polygonMode)
		{
		case POLYGON_MODE_FILL: return VK_POLYGON_MODE_FILL;
		case POLYGON_MODE_LINE: return VK_POLYGON_MODE_LINE;
		case POLYGON_MODE_POINT: return VK_POLYGON_MODE_POINT;
		}

		return VK_POLYGON_MODE_FILL;
	}

	VkFrontFace RenderContextVK::GetVkFrontFace(FrontFace frontFace)
	{
		switch (frontFace)
		{
		case FRONT_FACE_CCW: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
		case FRONT_FACE_CW: return VK_FRONT_FACE_CLOCKWISE;
		}

		return VK_FRONT_FACE_CLOCKWISE;
	}

	VkCompareOp RenderContextVK::GetVkCompareOp(CompareOperation compareOperation)
	{
		switch (compareOperation)
		{
		case COMPARE_OPERATION_NEVER: return VK_COMPARE_OP_NEVER;
		case COMPARE_OPERATION_ALWAYS: return VK_COMPARE_OP_ALWAYS;
		case COMPARE_OPERATION_EQUAL: return VK_COMPARE_OP_EQUAL;
		case COMPARE_OPERATION_NOT_EQUAL: return VK_COMPARE_OP_NOT_EQUAL;
		case COMPARE_OPERATION_GREATER: return VK_COMPARE_OP_GREATER;
		case COMPARE_OPERATION_LESS: return VK_COMPARE_OP_LESS;
		case COMPARE_OPERATION_GREATER_EQUAL: return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case COMPARE_OPERATION_LESS_EQUAL: return VK_COMPARE_OP_LESS_OR_EQUAL;
		}

		return VK_COMPARE_OP_LESS;
	}

	VkStencilOp RenderContextVK::GetVkStencilOp(StencilOperation stencilOperation)
	{
		switch (stencilOperation)
		{
		case STENCIL_OPERATION_KEEP: return VK_STENCIL_OP_KEEP;
		case STENCIL_OPERATION_ZERO: return VK_STENCIL_OP_ZERO;
		case STENCIL_OPERATION_REPLACE: return VK_STENCIL_OP_REPLACE;
		case STENCIL_OPERATION_INCREMENT_WRAP: return VK_STENCIL_OP_INCREMENT_AND_WRAP;
		case STENCIL_OPERATION_DECREMENT_WRAP: return VK_STENCIL_OP_DECREMENT_AND_WRAP;
		case STENCIL_OPERATION_INCREMENT_CLAMP: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
		case STENCIL_OPERATION_DECREMENT_CLAMP: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
		case STENCIL_OPERATION_INVERT: return VK_STENCIL_OP_INVERT;
		}

		return VK_STENCIL_OP_KEEP;
	}

	void RenderContextVK::GetVkStencilOpState(VkStencilOpState* vkState, const StencilState& state)
	{
		vkState->reference = state.reference;
		vkState->compareMask = state.compareMask;
		vkState->writeMask = state.writeMask;
		vkState->compareOp = GetVkCompareOp(state.compare);
		vkState->depthFailOp = GetVkStencilOp(state.stencilPassDepthFail);
		vkState->failOp = GetVkStencilOp(state.stencilFail);
		vkState->passOp = GetVkStencilOp(state.depthStencilPass);
	}

	VkBlendFactor RenderContextVK::GetVkBlendFactor(BlendFactor blendFactor)
	{
		switch (blendFactor)
		{
		case BLEND_FACTOR_ONE: return VK_BLEND_FACTOR_ONE;
		case BLEND_FACTOR_ZERO: return VK_BLEND_FACTOR_ZERO;
		case BLEND_FACTOR_SRC: return VK_BLEND_FACTOR_SRC_COLOR;
		case BLEND_FACTOR_DST: return VK_BLEND_FACTOR_DST_COLOR;
		case BLEND_FACTOR_ONE_MINUS_SRC: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case BLEND_FACTOR_ONE_MINUS_DST: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case BLEND_FACTOR_SRC_ALPHA: return VK_BLEND_FACTOR_SRC_ALPHA;
		case BLEND_FACTOR_DST_ALPHA: return VK_BLEND_FACTOR_DST_ALPHA;
		case BLEND_FACTOR_ONE_MINUS_SRC_ALPHA: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case BLEND_FACTOR_ONE_MINUS_DST_ALPHA: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		}

		return VK_BLEND_FACTOR_ONE;
	}

	VkBlendOp RenderContextVK::GetVkBlendOp(BlendOperation blendOperation)
	{
		switch (blendOperation)
		{
		case BLEND_OPERATION_ADD: return VK_BLEND_OP_ADD;
		case BLEND_OPERATION_SUB: return VK_BLEND_OP_SUBTRACT;
		case BLEND_OPERATION_REVERSE_SUB: return VK_BLEND_OP_REVERSE_SUBTRACT;
		case BLEND_OPERATION_MAX: return VK_BLEND_OP_MAX;
		case BLEND_OPERATION_MIN: return VK_BLEND_OP_MIN;
		}

		return VK_BLEND_OP_ADD;
	}

	VkDynamicState RenderContextVK::GetVkDynamicState(DynamicState dynamicState)
	{
		switch (dynamicState)
		{
		case DYNAMIC_STATE_VIEWPORT: return VK_DYNAMIC_STATE_VIEWPORT;
		case DYNAMIC_STATE_SCISSOR: return VK_DYNAMIC_STATE_SCISSOR;
		}
	}

	VkBufferUsageFlags RenderContextVK::GetVkBufferUsage(BufferType type)
	{
		switch (type)
		{
		case BUFFER_TYPE_VERTEX: return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		case BUFFER_TYPE_INDEX: return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		case BUFFER_TYPE_MEMORY_TRANSFER_SRC: return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		case BUFFER_TYPE_MEMORY_TRANSFER_DST: return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}

		return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	}

	VkFilter RenderContextVK::GetVkFilter(TextureFilter filter)
	{
		switch (filter)
		{
		case TEXTURE_FILTER_LINEAR: return VK_FILTER_LINEAR;
		case TEXTURE_FILTER_NEAREST: return VK_FILTER_NEAREST;
		}

		return VK_FILTER_LINEAR;
	}

	VkSamplerAddressMode RenderContextVK::GetVkSamplerAddressMode(TextureAddressMode addressMode)
	{
		switch (addressMode)
		{
		case TEXTURE_ADDRESS_MODE_REPEAT: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case TEXTURE_ADDRESS_MODE_MIRROR_REPEAT: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case TEXTURE_ADDRESS_MODE_CLAMP_TO_EDGE: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case TEXTURE_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE: return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
		case TEXTURE_ADDRESS_MODE_CLAMP_TO_BORDER: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		}

		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	}

	VkBorderColor RenderContextVK::GetVkBorderColor(TextureBorderColor borderColor)
	{
		switch (borderColor)
		{
		case TEXTURE_BORDER_COLOR_BLACK_INT: return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		case TEXTURE_BORDER_COLOR_WHITE_INT: return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
		case TEXTURE_BORDER_COLOR_TRANSPARENT_INT: return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
		case TEXTURE_BORDER_COLOR_BLACK_FLOAT: return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		case TEXTURE_BORDER_COLOR_WHITE_FLOAT: return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		case TEXTURE_BORDER_COLOR_TRANSPARENT_FLOAT: return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		}
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{

		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: EU_LOG_WARN(pCallbackData->pMessage); break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: EU_LOG_ERROR(pCallbackData->pMessage); break;
		}

		RenderContextVK* rc = (RenderContextVK*)pUserData;
		for (u32 i = 0; i < pCallbackData->objectCount; i++)
		{
			const VkDebugUtilsObjectNameInfoEXT& object = pCallbackData->pObjects[i];
			switch (object.objectType)
			{
				case VK_OBJECT_TYPE_SHADER_MODULE:
				{
					for (u32 j = 0; j < rc->m_Shaders.Size(); j++)
					{
						if (object.objectHandle == (uint64_t)rc->m_Shaders[j].vertex ||
							object.objectHandle == (uint64_t)rc->m_Shaders[j].fragment)
						{
							EU_LOG_WARN(rc->m_Shaders[j].parseInfo.name.C_Str());
						}
					}
				} break;
				case VK_OBJECT_TYPE_DESCRIPTOR_SET:
				{
					//for(u32 j = 0;)
				}
			}
		}

		std::cout << "\n\n";

		return VK_FALSE;
	}


}
