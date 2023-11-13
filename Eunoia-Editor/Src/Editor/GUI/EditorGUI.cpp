#include "EditorGUI.h"

#include <Eunoia\Core\Engine.h>
#include <Eunoia\Core\Input.h>
#include <Eunoia\Math\Math.h>
#include <Eunoia\Rendering\RenderContext.h>
#include <Eunoia\Rendering\Asset\AssetManager.h>
#include <Eunoia\Rendering\Renderer2D.h>
#include <Eunoia\Rendering\Renderer3D.h>
#include <Eunoia\Rendering\MasterRenderer.h>
#include <Eunoia\Rendering\Asset\MaterialLoader.h>
#include <Eunoia\Utils\Log.h>
#include <Eunoia\ECS\Components\Components.h>
#include <Eunoia\ECS\Events\RigidBodyTransformModifiedEvent.h>
#include <Eunoia\Physics\PhysicsEngine3D.h>
#include <Eunoia\ECS\Systems\ViewProjectionSystem.h>

#include "../../Vendor/imgui/imgui.h"
#include "../../Vendor/imgui/imgui_internal.h"
#include "../../Vendor/imgui/ImGuizmo.h"

#ifdef EU_PLATFORM_WINDOWS
#include <Eunoia\Platform\Win32\DisplayWin32.h>
#include "../../Vendor/imgui/imgui_impl_win32.h"
#endif

#include "../ProjectManager.h"

IMGUI_IMPL_API LRESULT  ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define EU_PROJECT_APPLICATION 1

#define EU_TEXT_BUFFER_SIZE_GENERAL			64
#define EU_TEXT_BUFFER_SIZE_ASSET_SEARCH	512
#define EU_TEXT_BUFFER_SIZE_FILE_NAME		64
#define EU_TEXT_BUFFER_SIZE_ENTITY_NAME		64

namespace Eunoia_Editor {

	using namespace Eunoia;

	enum AssetModify
	{
		ASSET_MODIFY_RENAME,
		ASSET_MODIFY_DELETE,
		ASSET_MODIFY_NONE
	};

	struct Editor_Data
	{
		ImGuiContext* imguiContext;
		ShaderID guiShader;
		RenderPassID renderPass;
		ShaderBufferID guiPerFrameBuffer;

		BufferID vertexBuffer;
		BufferID indexBuffer;

		TextureID gameWindowTexture;
		TextureID fontTexture;
		TextureID defaultTexture;
		TextureID directoryIcon;
		TextureID fileIcon;

		s32 selectedEntity;

		EUDirectory* previousDirectory;
		EUDirectory* currentDirectory;
		b32 modifyAssetName;
		AssetModify dirModify;
		AssetModify fileModify;
		u32 assetModifyIndex;

		String openPopup;
		char generalNameBuffer[EU_TEXT_BUFFER_SIZE_GENERAL];
		char assetSearchBuffer[EU_TEXT_BUFFER_SIZE_ASSET_SEARCH];
		char fileNameBuffer[EU_TEXT_BUFFER_SIZE_FILE_NAME];
		char entityNameBuffer[EU_TEXT_BUFFER_SIZE_ENTITY_NAME];
	};

	static Editor_Data s_Data;

	static void DisplayEvent_UpdateImGuiInput(const Eunoia::DisplayEvent& eventInfo, void* userPtr)
	{
		if (eventInfo.type == Eunoia::DISPLAY_EVENT_KEY && eventInfo.inputType == Eunoia::DISPLAY_INPUT_EVENT_PRESS)
		{
			ImGuiIO& io = ImGui::GetIO();
			if (eventInfo.input == Eunoia::EU_KEY_BACKSPACE)
				io.KeysDown[io.KeyMap[ImGuiKey_Backspace]] = true;
			else if (eventInfo.input == Eunoia::EU_KEY_ENTER)
				io.KeysDown[io.KeyMap[ImGuiKey_Enter]] = true;
			else
				io.AddInputCharacter(Eunoia::EUInput::GetChar((Eunoia::Key)eventInfo.input));

			Editor_Data* data = (Editor_Data*)userPtr;
			/*if (data->waitForKeySelection)
			{
				data->waitForKeySelection = false;
				*data->keyValueToSet = (Key)eventInfo.input;
			}*/
		}
		if (eventInfo.type == Eunoia::DISPLAY_EVENT_KEY && eventInfo.inputType == Eunoia::DISPLAY_INPUT_EVENT_RELEASE)
		{
			ImGuiIO& io = ImGui::GetIO();
			if (eventInfo.input == Eunoia::EU_KEY_BACKSPACE)
				io.KeysDown[io.KeyMap[ImGuiKey_Backspace]] = false;
			else if (eventInfo.input == Eunoia::EU_KEY_ENTER)
				io.KeysDown[io.KeyMap[ImGuiKey_Enter]] = false;

		}
	}

	void EditorGUI::Init()
	{
		s_Data.selectedEntity = EU_ECS_INVALID_ENTITY_ID;
		s_Data.modifyAssetName = false;
		s_Data.previousDirectory = 0;
		s_Data.modifyAssetName = false;
		s_Data.assetModifyIndex = -1;
		s_Data.dirModify = ASSET_MODIFY_NONE;
		s_Data.fileModify = ASSET_MODIFY_NONE;
		

		Engine::GetDisplay()->AddDisplayEventCallback(DisplayEvent_UpdateImGuiInput, &s_Data);

		s_Data.gameWindowTexture = Engine::GetRenderer()->GetFinalOutput();

		InitImGui();
		InitRenderPass();
		InitResources();
	}

	void EditorGUI::InitImGui()
	{
		s_Data.imguiContext = ImGui::CreateContext();
		ImGui::SetCurrentContext(s_Data.imguiContext);

		ImGuizmo::SetImGuiContext(s_Data.imguiContext);
		ImGuizmo::Enable(true);

		ImGuiIO& io = ImGui::GetIO();
		InitDarkTheme();
		io.FontDefault = io.Fonts->AddFontFromFileTTF("Res/Fonts/OpenSans/static/OpenSans/OpenSans-Regular.ttf", 18.0f);
		io.KeyMap[ImGuiKey_Backspace] = EU_KEY_BACKSPACE;
		io.DisplaySize.x = Engine::GetDisplay()->GetWidth();
		io.DisplaySize.y = Engine::GetDisplay()->GetHeigth();
		io.Framerate = 1.0f / 60.0f;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

#ifdef EU_PLATFORM_WINDOWS
		((DisplayWin32*)Engine::GetDisplay())->AddWindowProc(ImGui_ImplWin32_WndProcHandler);
#endif

		io.Fonts->AddFontDefault();

		u8* pixels;
		s32 width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
		s_Data.fontTexture = Engine::GetRenderContext()->CreateTexture2D(pixels, width, height, Eunoia::TEXTURE_FORMAT_RGBA8_UNORM);
		io.Fonts->SetTexID(s_Data.fontTexture);


#ifdef EU_PLATFORM_WINDOWS
		if (!ImGui_ImplWin32_Init(((Eunoia::DisplayWin32*)Eunoia::Engine::GetDisplay())->GetHandle()))
		{
			EU_LOG_FATAL("Could not initialize ImGuiWin32");
			return;
		}

		EU_LOG_TRACE("Initialized ImGui for Win32");
#endif
	}

	void EditorGUI::InitDarkTheme()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowRounding = 5.3f;
		style.FrameRounding = 2.3f;
		style.ScrollbarRounding = 0;

		style.Colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 0.90f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.15f, 1.00f);
		style.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.85f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.70f, 0.70f, 0.70f, 0.65f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.01f, 1.00f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.90f, 0.80f, 0.80f, 0.40f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.90f, 0.65f, 0.65f, 0.45f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.83f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.87f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.01f, 0.01f, 0.02f, 0.80f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.55f, 0.53f, 0.55f, 0.51f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.91f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.83f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.70f, 0.70f, 0.70f, 0.62f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.30f, 0.30f, 0.30f, 0.84f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.48f, 0.72f, 0.89f, 0.49f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.50f, 0.69f, 0.99f, 0.68f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.80f, 0.50f, 0.50f, 1.00f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.30f, 0.69f, 1.00f, 0.53f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.44f, 0.61f, 0.86f, 1.00f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.38f, 0.62f, 0.83f, 1.00f);
		style.Colors[ImGuiCol_Separator] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.70f, 0.60f, 0.60f, 1.00f);
		style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.90f, 0.70f, 0.70f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.85f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
		style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
	}

	void EditorGUI::InitRenderPass()
	{
		RenderContext* rc = Engine::GetRenderContext();

		RenderPass renderPass;
		Framebuffer* framebuffer = &renderPass.framebuffer;
		framebuffer->useSwapchainSize = true;
		framebuffer->numAttachments = 1;
		framebuffer->attachments[0].format = TEXTURE_FORMAT_SWAPCHAIN_FORMAT;
		framebuffer->attachments[0].isClearAttachment = true;
		framebuffer->attachments[0].isSamplerAttachment = false;
		framebuffer->attachments[0].isStoreAttachment = true;
		framebuffer->attachments[0].isSubpassInputAttachment = false;
		framebuffer->attachments[0].isSwapchainAttachment = true;
		framebuffer->attachments[0].nonClearAttachmentPreserve = false;
		framebuffer->attachments[0].memoryTransferSrc = false;

		Subpass subpass;
		subpass.useDepthStencilAttachment = false;
		subpass.depthStencilAttachment = 0;
		subpass.numReadAttachments = 0;
		subpass.numWriteAttachments = 1;
		subpass.writeAttachments[0] = 0;

		s_Data.guiShader = rc->LoadShader("EditorGUI");

		GraphicsPipeline pipeline{};
		pipeline.shader = s_Data.guiShader;
		pipeline.topology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipeline.viewportState.useFramebufferSizeForScissor = pipeline.viewportState.useFramebufferSizeForViewport = true;
		pipeline.viewportState.viewport.x = pipeline.viewportState.viewport.y =
		pipeline.viewportState.scissor.x = pipeline.viewportState.scissor.y = 0;
		pipeline.vertexInputState.vertexSize = EU_VERTEX_SIZE_AUTO;
		pipeline.vertexInputState.numAttributes = 3;
		pipeline.vertexInputState.attributes[0].location = 0;
		pipeline.vertexInputState.attributes[0].type = VERTEX_ATTRIBUTE_FLOAT2;
		pipeline.vertexInputState.attributes[0].name = "POSITION";
		pipeline.vertexInputState.attributes[1].location = 1;
		pipeline.vertexInputState.attributes[1].type = VERTEX_ATTRIBUTE_FLOAT2;
		pipeline.vertexInputState.attributes[1].name = "TEXCOORD";
		pipeline.vertexInputState.attributes[2].location = 2;
		pipeline.vertexInputState.attributes[2].type = VERTEX_ATTRIBUTE_U32;
		pipeline.vertexInputState.attributes[2].name = "POSITION";
		pipeline.rasterizationState.cullMode = CULL_MODE_BACK;
		pipeline.rasterizationState.depthClampEnabled = false;
		pipeline.rasterizationState.discard = false;
		pipeline.rasterizationState.frontFace = FRONT_FACE_CW;
		pipeline.rasterizationState.polygonMode = POLYGON_MODE_FILL;
		pipeline.numBlendStates = 1;
		pipeline.blendStates[0].blendEnabled = true;
		pipeline.blendStates[0].color.dstFactor = BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		pipeline.blendStates[0].color.srcFactor = BLEND_FACTOR_SRC_ALPHA;
		pipeline.blendStates[0].color.operation = BLEND_OPERATION_ADD;
		pipeline.blendStates[0].alpha = pipeline.blendStates[0].color;
		pipeline.blendStates[0].alpha.operation = BLEND_OPERATION_SUB;
		pipeline.depthStencilState.depthTestEnabled = false;
		pipeline.depthStencilState.depthWriteEnabled = false;
		pipeline.depthStencilState.stencilTestEnabled = false;
		pipeline.depthStencilState.depthCompare = COMPARE_OPERATION_LESS;
		pipeline.dynamicStates[DYNAMIC_STATE_SCISSOR] = true;

		MaxTextureGroupBinds maxBinds;
		maxBinds.set = 1;
		maxBinds.maxBinds = 128;

		pipeline.maxTextureGroupBinds.Push(maxBinds);

		subpass.pipelines.Push(pipeline);
		pipeline.blendStates[0].blendEnabled = false;
		subpass.pipelines.Push(pipeline);

		renderPass.subpasses.Push(subpass);

		s_Data.renderPass = rc->CreateRenderPass(renderPass);

		s_Data.guiPerFrameBuffer = rc->CreateShaderBuffer(SHADER_BUFFER_UNIFORM_BUFFER, sizeof(v2), 1);
		rc->AttachShaderBufferToRenderPass(s_Data.renderPass, s_Data.guiPerFrameBuffer, 0, 0, 0, 0);
		rc->AttachShaderBufferToRenderPass(s_Data.renderPass, s_Data.guiPerFrameBuffer, 0, 1, 0, 0);
	}

	void EditorGUI::InitResources()
	{
		RenderContext* rc = Engine::GetRenderContext();

		const u32 MAX_VERTICES = 5000;
		const u32 MAX_INDICES = MAX_VERTICES * 6;

		u8 white[4] = { 255, 255, 255, 255 };

		s_Data.vertexBuffer = rc->CreateBuffer(BUFFER_TYPE_VERTEX, BUFFER_USAGE_DYNAMIC, EU_NULL, sizeof(ImDrawVert) * MAX_VERTICES);
		s_Data.indexBuffer = rc->CreateBuffer(BUFFER_TYPE_INDEX, BUFFER_USAGE_DYNAMIC, EU_NULL, sizeof(ImDrawIdx) * MAX_INDICES);
		s_Data.defaultTexture = rc->CreateTexture2D(white, 1, 1, TEXTURE_FORMAT_RGBA8_UNORM);
		s_Data.directoryIcon = rc->CreateTexture2D("Res/Editor/DirectoryIcon.eutex");
		s_Data.fileIcon = rc->CreateTexture2D("Res/Editor/FileIcon.eutex");
	}

	void EditorGUI::Destroy()
	{
#ifdef EU_PLATFORM_WINDOWS
		ImGui_ImplWin32_Shutdown();
#endif
		ImGui::DestroyContext();
	}

	void EditorGUI::UpdateInput()
	{
#ifdef EU_PLATFORM_WINDOWS
		ImGui_ImplWin32_NewFrame();
#endif
	}

	void EditorGUI::BeginFrame()
	{
		ImGui::SetCurrentContext(s_Data.imguiContext);
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void EditorGUI::EndFrame()
	{
		Eunoia::RenderContext* rc = Eunoia::Engine::GetRenderContext();
		ImGui::Render();
		ImDrawData* drawData = ImGui::GetDrawData();

		int fbWidth = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
		int fbHeight = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
		if (fbWidth <= 0 || fbHeight <= 0 || drawData->TotalVtxCount == 0)
			return;

		ImDrawVert* vertices = (ImDrawVert*)rc->MapBuffer(s_Data.vertexBuffer);
		ImDrawIdx* indices = (ImDrawIdx*)rc->MapBuffer(s_Data.indexBuffer);

		for (u32 i = 0; i < drawData->CmdListsCount; i++)
		{
			ImDrawList* drawList = drawData->CmdLists[i];
			memcpy(vertices, &drawList->VtxBuffer[0], drawList->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(indices, &drawList->IdxBuffer[0], drawList->IdxBuffer.Size * sizeof(ImDrawIdx));
			vertices += drawList->VtxBuffer.Size;
			indices += drawList->IdxBuffer.Size;
		}

		Eunoia::RenderPassBeginInfo beginInfo;
		beginInfo.initialPipeline = 0;
		beginInfo.renderPass = s_Data.renderPass;
		beginInfo.numClearValues = 1;
		beginInfo.clearValues[0].color = { 0.0, 0.0, 0.0, 0.0 };

		rc->BeginRenderPass(beginInfo);

		u32 width = Eunoia::Engine::GetDisplay()->GetWidth();
		u32 height = Eunoia::Engine::GetDisplay()->GetHeigth();
		Eunoia::v2 screenSize(width, height);
		rc->UpdateShaderBuffer(s_Data.guiPerFrameBuffer, &screenSize, sizeof(v2));
		
		Eunoia::RenderCommand command;
		command.vertexBuffer = s_Data.vertexBuffer;
		command.indexBuffer = s_Data.indexBuffer;
		command.indexType = Eunoia::INDEX_TYPE_U16;

		ImVec2 clipOff = drawData->DisplayPos;
		ImVec2 clipScale = drawData->FramebufferScale;

		u32 vertexOffset = 0;
		u32 indexOffset = 0;
		for (u32 i = 0; i < drawData->CmdListsCount; i++)
		{
			ImDrawList* drawList = drawData->CmdLists[i];


			for (u32 j = 0; j < drawList->CmdBuffer.Size; j++)
			{
				const ImDrawCmd* drawCmd = &drawList->CmdBuffer[j];

				Rect clip_rect;
				clip_rect.x = (drawCmd->ClipRect.x - clipOff.x) * clipScale.x;
				clip_rect.y = (drawCmd->ClipRect.y - clipOff.y) * clipScale.y;
				clip_rect.width = (drawCmd->ClipRect.z - clipOff.x) * clipScale.x;
				clip_rect.height = (drawCmd->ClipRect.w - clipOff.y) * clipScale.y;

				if (clip_rect.x < fbWidth && clip_rect.y < fbHeight && clip_rect.width >= 0.0f && clip_rect.height >= 0.0f)
				{
					if (clip_rect.x < 0.0f)
						clip_rect.x = 0.0f;
					if (clip_rect.y < 0.0f)
						clip_rect.y = 0.0f;
					
					clip_rect.width = clip_rect.width - clip_rect.x;
					clip_rect.height = clip_rect.height - clip_rect.y;
					rc->SetScissor(clip_rect);

					TextureID id = s_Data.defaultTexture;
					if (drawCmd->TextureId != EU_INVALID_TEXTURE_ID)
						id = drawCmd->TextureId;

					if (id == s_Data.fontTexture || id == s_Data.directoryIcon || id == s_Data.fileIcon)
						rc->SwitchPipeline(0);
					else
						rc->SwitchPipeline(1);

					Eunoia::TextureGroupBind bind;
					bind.numTextureBinds = 1;
					bind.set = 1;
					bind.binds[0].binding = 0;
					bind.binds[0].textureArrayLength = 1;
					bind.binds[0].sampler = EU_SAMPLER_LINEAR_CLAMP_TO_EDGE;
					bind.binds[0].texture[0] = id;

					rc->BindTextureGroup(bind);

					command.indexOffset = drawCmd->IdxOffset + indexOffset;
					command.vertexOffset = drawCmd->VtxOffset + vertexOffset;
					command.count = drawCmd->ElemCount;

					rc->SubmitRenderCommand(command);
				}
			}

			vertexOffset += drawList->VtxBuffer.Size;
			indexOffset += drawList->IdxBuffer.Size;
		}

		rc->UnmapBuffer(s_Data.vertexBuffer);
		rc->UnmapBuffer(s_Data.indexBuffer);

		rc->EndRenderPass();
	}

	static void DrawMainMenuBar()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File##MenuFile"))
			{
				if (ImGui::Selectable("CreateProject"))
				{
					s_Data.openPopup = "Create Project";
				}

				if (ImGui::Selectable("Load Project"))
				{
					s_Data.openPopup = "Load Project";
				}
				if (ImGui::Selectable("Save Project"))
				{
					ProjectManager::SaveProject();
				}
				if (ImGui::Selectable("Open IDE"))
				{
					ProjectManager::OpenVS2019();
				}

				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

	static void DrawGameWindow()
	{
		if (ImGui::Begin("Game Window"))
		{
			if (!ProjectManager::GetProject()->loaded)
			{
				ImGui::End();
				return;
			}

			u32 width, height;
			RenderContext* rc = Engine::GetRenderContext();
			MasterRenderer* mr = Engine::GetRenderer();
			rc->GetTextureSize(s_Data.gameWindowTexture, &width, &height);

			ImVec2 windowSize = ImGui::GetWindowSize();
			windowSize.y -= 40;
			if (windowSize.x != width || windowSize.y != height)
			{
				mr->ResizeOutput(windowSize.x, windowSize.y);
			}

			ImGui::Image(s_Data.gameWindowTexture, ImVec2(width, height));
		}
		ImGui::End();
	}

	static void DrawEntityEntityHierarchyHelper(EntityID entity)
	{
		ECS* ecs = ProjectManager::GetProject()->application->GetECS();
		ECSEntityContainer* container = &ecs->GetAllEntities_()[entity - 2];

		b32 treeOpen = ImGui::TreeNodeEx((container->name + "##EH" + container->name).C_Str(), ImGuiTreeNodeFlags_SpanFullWidth);
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(EU_BUTTON_RIGHT))
		{
			s_Data.openPopup = "AddEntityContent";
			s_Data.selectedEntity = entity;
			memcpy(s_Data.entityNameBuffer, container->name.C_Str(), container->name.Length());
		}
		else if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(EU_BUTTON_LEFT))
		{
			s_Data.selectedEntity = entity;
			memcpy(s_Data.entityNameBuffer, container->name.C_Str(), container->name.Length());
		}
		if (treeOpen)
		{
			for (u32 i = 0; i < container->children.Size(); i++)
			{
				DrawEntityEntityHierarchyHelper(container->children[i]);
			}
			ImGui::TreePop();
		}
	}

	static void DrawEntityEntityHierarchyWindow()
	{
		if (ImGui::Begin("Entity Hierarchy"))
		{
			if (!ProjectManager::GetProject()->loaded)
			{
				ImGui::End();
				return;
			}

			ECS* ecs = ProjectManager::GetProject()->application->GetECS();

			ImGui::Text("Scene: "); ImGui::SameLine();
			if (ecs->HasActiveScene())
				ImGui::Text(ecs->GetActiveSceneName().C_Str());
			
			if (ImGui::IsWindowHovered())
			{
				if (ImGui::IsMouseClicked(EU_BUTTON_RIGHT))
					s_Data.openPopup = "AddEntity";
			}

			if (ecs->HasActiveScene())
			{
				EntityID entity = ecs->GetRootEntity();
				const ECSEntityContainer& container = ecs->GetAllEntities_()[entity - 2];
				for (u32 i = 0; i < container.children.Size(); i++)
				{
					DrawEntityEntityHierarchyHelper(container.children[i]);
				}
			}
		}
		ImGui::End();
	}

	static void DrawAssetBrowser()
	{
		if (ImGui::Begin("Asset Browser"))
		{
			if (!ProjectManager::GetProject()->loaded)
			{
				ImGui::End();
				return;
			}

			r32 padding = 8.0f;
			r32 thumbnailSize = 128;
			r32 cellSize = thumbnailSize + padding;
			r32 contentWidth = ImGui::GetContentRegionAvail().x;
			u32 columnCount = EU_MAX(contentWidth / cellSize, 1);

			if (!ProjectManager::GetProject()->loaded)
			{
				ImGui::End();
				return;
			}

			if (ImGui::ArrowButton("AssetBrowsertBackDirButton", ImGuiDir_Left))
			{
				if (s_Data.currentDirectory->parentDirectory)
				{
					s_Data.previousDirectory = s_Data.currentDirectory;
					s_Data.currentDirectory = s_Data.currentDirectory->parentDirectory;
				}
			} ImGui::SameLine();
			if (ImGui::ArrowButton("AssetBrowserPrevButton", ImGuiDir_Right))
			{
				if (s_Data.previousDirectory)
					s_Data.currentDirectory = s_Data.previousDirectory;
			} ImGui::SameLine();
			if (ImGui::Button("Refresh##AssetBrowserRefresh"))
			{
				s_Data.currentDirectory->Refresh();
			}
			ImGui::Separator();

			EU_PERSISTENT b32 RenamingDirectory = false;
			EU_PERSISTENT b32 RenamingFile = false;
			if (s_Data.dirModify == ASSET_MODIFY_DELETE)
			{
				s_Data.currentDirectory->directories[s_Data.assetModifyIndex]->Delete();
				s_Data.dirModify = ASSET_MODIFY_NONE;
			}
			else if (s_Data.dirModify == ASSET_MODIFY_RENAME)
			{
				RenamingDirectory = true;
				s_Data.dirModify = ASSET_MODIFY_NONE;
				u32 ami = s_Data.assetModifyIndex;
				const String& fileName = s_Data.currentDirectory->directories[s_Data.assetModifyIndex]->name;
				memcpy(s_Data.fileNameBuffer, fileName.C_Str(), fileName.Length() + 1);
			}
			if (s_Data.fileModify == ASSET_MODIFY_DELETE)
			{
				s_Data.currentDirectory->files[s_Data.assetModifyIndex].Delete();
				s_Data.fileModify = ASSET_MODIFY_NONE;
			}
			else if (s_Data.fileModify == ASSET_MODIFY_RENAME)
			{
				RenamingFile = true;
				s_Data.fileModify = ASSET_MODIFY_NONE;
				const String& fileName = s_Data.currentDirectory->files[s_Data.assetModifyIndex].name;
				memcpy(s_Data.fileNameBuffer, fileName.C_Str(), fileName.Length() + 1);
			}

			ImGui::Columns(columnCount, 0, false);

			ImVec4 iconBgColor = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
			b32 iconRightClicked = false;
			EUDirectory* newDirectory = s_Data.currentDirectory;
			for (u32 i = 0; i < s_Data.currentDirectory->directories.Size(); i++)
			{
				EUDirectory* dir = s_Data.currentDirectory->directories[i];
				ImGui::PushID(("AssetBrowserDirectory" + String::S32ToString(i)).C_Str());
				ImGui::ImageButton(s_Data.directoryIcon, ImVec2(thumbnailSize, thumbnailSize), ImVec2(0, 0), ImVec2(1, 1), 0, iconBgColor);
				if (ImGui::BeginDragDropTarget())
				{
					const ImGuiPayload* assetFile = ImGui::AcceptDragDropPayload("AssetFile");
					const ImGuiPayload* assetDir = ImGui::AcceptDragDropPayload("AssetDir");

					if (assetFile)
					{
						u32 assetFileIndex = *(u32*)assetFile->Data;
						s_Data.currentDirectory->MoveFileIntoDir(i, assetFileIndex);
					}
					if (assetDir)
					{
						u32 assetDirIndex = *(u32*)assetDir->Data;
						s_Data.currentDirectory->MoveDirectory(i, assetDirIndex);
					}

					ImGui::EndDragDropTarget();
				}
				if (ImGui::BeginDragDropSource())
				{
					ImGui::ImageButton(s_Data.directoryIcon, ImVec2(thumbnailSize / 2, thumbnailSize / 2), ImVec2(0, 0), ImVec2(1, 1), 0, iconBgColor);
					ImGui::SetDragDropPayload("AssetDir", &i, sizeof(u32));
					ImGui::EndDragDropSource();
				}
				ImGui::PopID();
				if (ImGui::IsItemHovered())
				{
					if (ImGui::IsMouseDoubleClicked(EU_BUTTON_LEFT))
					{
						newDirectory = dir;
					}
					else if (ImGui::IsMouseClicked(EU_BUTTON_RIGHT))
					{
						iconRightClicked = true;
						s_Data.openPopup = "Modify Directory";
						s_Data.assetModifyIndex = i;
					}
				}

				if (RenamingDirectory && (s_Data.assetModifyIndex == i))
				{
					ImGui::SetKeyboardFocusHere(0);
					if (ImGui::InputText(("##AssetDirectoryRename" + String::S32ToString(i)).C_Str(), s_Data.fileNameBuffer, EU_TEXT_BUFFER_SIZE_FILE_NAME,
						ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll) ||
						((ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)) && !ImGui::IsItemHovered()))
					{
						RenamingDirectory = false;
						s_Data.assetModifyIndex = -1;
						if (s_Data.fileNameBuffer[0] != '\0')
							s_Data.currentDirectory->directories[i]->Rename(s_Data.fileNameBuffer);
					}
				}
				else
				{
					ImGui::Text(dir->name.C_Str());
				}

				ImGui::NextColumn();
			}

			for (u32 i = 0; i < s_Data.currentDirectory->files.Size(); i++)
			{
				EUFile* file = &s_Data.currentDirectory->files[i];
				ImGui::PushID(("AssetBrowserFile" + String::S32ToString(i)).C_Str());
				ImGui::ImageButton(s_Data.fileIcon, ImVec2(thumbnailSize, thumbnailSize), ImVec2(0, 0), ImVec2(1, 1), 0, iconBgColor);
				if (ImGui::BeginDragDropSource())
				{
					ImGui::ImageButton(s_Data.fileIcon, ImVec2(thumbnailSize / 2, thumbnailSize / 2), ImVec2(0, 0), ImVec2(1, 1), 0, iconBgColor, ImVec4(0.6f, 0.6f, 0.6f, 0.85f));
					ImGui::SetDragDropPayload("AssetFile", &i, sizeof(u32));
					ImGui::EndDragDropSource();
				}
				ImGui::PopID();
				if (ImGui::IsItemHovered())
				{
					if (ImGui::IsMouseDoubleClicked(EU_BUTTON_LEFT))
					{
						if (file->extension == "euscene")
						{
							ECS* ecs = ProjectManager::GetProject()->application->GetECS();

							ECSLoadedScene loadedScene;
							ECSLoader::LoadECSSceneFromFile(&loadedScene, file->path);
							SceneID scene = ecs->LoadSceneFromLoadedDataFormat(loadedScene, true);
							file->userData = malloc(sizeof(SceneID)); //TODO: free data if file is deleted
							memcpy(file->userData, &scene, sizeof(SceneID));
							ecs->SetActiveScene(scene);
						}
					}
					else if (ImGui::IsMouseClicked(EU_BUTTON_RIGHT))
					{
						iconRightClicked = true;
						s_Data.openPopup = "Modify File";
						s_Data.assetModifyIndex = i;
					}
				}

				if (RenamingFile && (s_Data.assetModifyIndex == i))
				{
					ImGui::SetKeyboardFocusHere(0);
					if (ImGui::InputText(("##AssetFileRename" + String::S32ToString(i)).C_Str(), s_Data.fileNameBuffer, EU_TEXT_BUFFER_SIZE_FILE_NAME, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll) ||
						((ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)) && !ImGui::IsItemHovered()))
					{
						RenamingFile = false;
						s_Data.assetModifyIndex = -1;
						if (s_Data.fileNameBuffer[0] != '\0')
							s_Data.currentDirectory->files[i].Rename(s_Data.fileNameBuffer);
					}
				}
				else
				{
					ImGui::Text(file->name.C_Str());
				}

				ImGui::NextColumn();
			}

			s_Data.currentDirectory = newDirectory;

			ImGui::Columns(1);

			if (ImGui::IsWindowHovered() & ImGui::IsMouseClicked(EU_BUTTON_RIGHT) && !iconRightClicked)
			{
				s_Data.openPopup = "Create Asset";
			}
		}
		ImGui::End();
	}

	static void DrawPopupWindows()
	{
		if (!s_Data.openPopup.Empty())
			ImGui::OpenPopup(s_Data.openPopup.C_Str());

		s_Data.openPopup = "";

		if (ImGui::BeginPopupModal("Create Project"))
		{
			ImGui::SetWindowSize(ImVec2(700, 450));
			if (ImGui::InputText("##CreateProjectProjectName", s_Data.generalNameBuffer, EU_TEXT_BUFFER_SIZE_GENERAL, ImGuiInputTextFlags_CharsNoBlank |
				ImGuiInputTextFlags_EnterReturnsTrue))
			{
				ProjectManager::CreateNewProject(s_Data.generalNameBuffer);
				s_Data.generalNameBuffer[0] = 0;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
		if (ImGui::BeginPopupModal("Load Project"))
		{
			ImGui::SetWindowSize(ImVec2(700, 450));
			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			EU_PERSISTENT b32 CompileProject;
			ImGui::Checkbox("Compile##CompileProject", (bool*)&CompileProject);
			const List<String>& projectFileNames = ProjectManager::GetProjectNamesInProjectFolder();
			if (ImGui::TreeNodeEx("Select Project", ImGuiTreeNodeFlags_SpanAvailWidth))
			{
				for (u32 i = 0; i < projectFileNames.Size(); i++)
				{
					if (ImGui::Selectable((projectFileNames[i] + "##LoadProject" + projectFileNames[i]).C_Str(),
						ImGuiSelectableFlags_SpanAllColumns))
					{
						ProjectManager::LoadProject(projectFileNames[i], CompileProject);
						s_Data.previousDirectory = ProjectManager::GetProject()->assetDirectory;
						s_Data.currentDirectory = ProjectManager::GetProject()->assetDirectory;
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::TreePop();
			}

			ImGui::EndPopup();
		}
		if (ImGui::BeginPopup("AddEntity"))
		{
			if (ImGui::Selectable("Add Entity"))
			{
				ECS* ecs = ProjectManager::GetProject()->application->GetECS();
				EntityID entity = ecs->CreateEntity();
				s_Data.selectedEntity = entity;
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Selectable("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		if (ImGui::BeginPopup("AddEntityContent"))
		{
			if (ImGui::Selectable("Add Child##AddEntityContentAddChild"))
			{
				ECS* ecs = ProjectManager::GetProject()->application->GetECS();
				EntityID entity = ecs->CreateEntity(s_Data.selectedEntity);
				s_Data.selectedEntity = entity;
			}
			if (ImGui::Selectable("Delete##AddEntityContentDelete"))
			{
				ECS* ecs = ProjectManager::GetProject()->application->GetECS();
				ecs->DestroyEntity(s_Data.selectedEntity);
			}
			ImGui::EndPopup();
		}
		if (ImGui::BeginPopup("Create Asset"))
		{
			if (ImGui::Selectable("+ New Folder"))
			{
				s_Data.currentDirectory->CreateNewDir("Folder" + String::S32ToString(s_Data.currentDirectory->directories.Size() + 1));
				s_Data.dirModify = ASSET_MODIFY_RENAME;
				s_Data.assetModifyIndex = s_Data.currentDirectory->directories.Size() - 1;
				ImGui::CloseCurrentPopup();
			} ImGui::Separator();
			if (ImGui::Selectable("+ New Scene"))
			{
				s_Data.currentDirectory->CreateNewFile("File" + String::S32ToString(s_Data.currentDirectory->files.Size() + 1) + ".euscene");
				s_Data.fileModify = ASSET_MODIFY_RENAME;
				s_Data.assetModifyIndex = s_Data.currentDirectory->files.Size() - 1;

				ECSLoadedScene loadedScene;
				loadedScene.name = "New Scene";

				ECSLoadedEntity rootEntity;
				rootEntity.name = "Root";
				rootEntity.enabled = true;

				loadedScene.entities.Push(rootEntity);
			}
			if (ImGui::Selectable("Load Resources"))
			{
				for (u32 i = 0; i < s_Data.currentDirectory->files.Size(); i++)
				{
					if (s_Data.currentDirectory->files[i].extension == "eutex")
						AssetManager::CreateTexture(s_Data.currentDirectory->files[i].path);
					else if (s_Data.currentDirectory->files[i].extension == "eumat")
					{
						LoadedMaterialFile loadedMaterialFile;
						MaterialLoader::LoadEumtlMaterial(s_Data.currentDirectory->files[i].path, &loadedMaterialFile);
						MaterialID mid;
						AssetManager::CreateMaterials(loadedMaterialFile, EU_SAMPLER_LINEAR_REPEAT_AF, &mid);
					}
					else if (s_Data.currentDirectory->files[i].extension == "eumdl")
					{
						ModelID mid = AssetManager::CreateModel(s_Data.currentDirectory->files[i].path);

					}
				}
			}
			ImGui::EndPopup();
		}
		if (ImGui::BeginPopup("Modify Directory"))
		{
			if (ImGui::Selectable("Rename"))
			{
				s_Data.dirModify = ASSET_MODIFY_RENAME;
				ImGui::CloseCurrentPopup();
			} ImGui::Separator();
			if (ImGui::Selectable("Delete"))
			{
				s_Data.dirModify = ASSET_MODIFY_DELETE;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		if (ImGui::BeginPopup("Modify File"))
		{
			if (ImGui::Selectable("Rename"))
			{
				s_Data.fileModify = ASSET_MODIFY_RENAME;
				ImGui::CloseCurrentPopup();
			} ImGui::Separator();
			if (ImGui::Selectable("Delete"))
			{
				s_Data.fileModify = ASSET_MODIFY_DELETE;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		if (ImGui::BeginPopup("EntityInspectorAddContent"))
		{
			ECS* ecs = ProjectManager::GetProject()->application->GetECS();
			if (ImGui::Selectable("Add Components"))
			{
				s_Data.openPopup = "Select Entity Components";
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Selectable("Add Child"))
			{
				ecs->CreateEntity(s_Data.selectedEntity);
			}
			ImGui::EndPopup();
		}
		if (ImGui::BeginPopupModal("Select Entity Components"))
		{
			EU_PERSISTENT b32 ShowEngineComponents = true;
			EU_PERSISTENT b32 ShowProjectComponents = true;

			ImGui::Text("Show Components"); ImGui::SameLine();
			ImGui::Checkbox("Show Engine Components", (bool*)&ShowEngineComponents); ImGui::SameLine();
			ImGui::Checkbox("Show Project Components", (bool*)&ShowProjectComponents);
			ImGui::InputText("##SearchComponents", s_Data.generalNameBuffer, 64);

			const List<MetadataInfo>& componentMetadatas = Metadata::GetComponentMetadataList();

			EU_PERSISTENT bool InitializeSelectedComponents = true;
			EU_PERSISTENT List<bool> SelectedComponents;

			if (InitializeSelectedComponents)
			{
				SelectedComponents.SetCapacityAndElementCount(componentMetadatas.Size());
				memset(&SelectedComponents[0], false, SelectedComponents.Size());
				InitializeSelectedComponents = false;
			}
			if (ShowEngineComponents)
			{
				ImGui::Text("Engine Components");
				ImGui::Columns(4);
				for (u32 i = 0; i < componentMetadatas.Size(); i++)
				{
					if (componentMetadatas[i].id <= Metadata::LastEngineTypeID)
					{
						ImGui::Checkbox((componentMetadatas[i].cls->name + "##AddComponentType_" + componentMetadatas[i].cls->name).C_Str(), &SelectedComponents[i]);
					}

					ImGui::NextColumn();
				}
				ImGui::EndColumns();
			}
			if (ShowProjectComponents)
			{
				ImGui::Text("ProjectComponents");
				ImGui::Columns(4);
				for (u32 i = 0; i < componentMetadatas.Size(); i++)
				{
					if (componentMetadatas[i].id > Metadata::LastEngineTypeID)
					{
						ImGui::Checkbox((componentMetadatas[i].cls->name + "##AddComponentType_" + componentMetadatas[i].cls->name).C_Str(), &SelectedComponents[i]);
					}

					ImGui::NextColumn();
				}
				ImGui::EndColumns();
			}

			if (ImGui::Button("Create Components##SEC_CreateComponents"))
			{
				ECS* ecs = ProjectManager::GetProject()->application->GetECS();
				for (u32 i = 0; i < SelectedComponents.Size(); i++)
				{
					if (SelectedComponents[i])
						ecs->CreateComponent(s_Data.selectedEntity, componentMetadatas[i].id);
				}

				InitializeSelectedComponents = true;
				ImGui::CloseCurrentPopup();
			} 

			ImGui::SameLine();

			if (ImGui::Button("Cancel##SEC_Cancel"))
			{
				InitializeSelectedComponents = true;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
		if (ImGui::BeginPopup("AddSystemsContent"))
		{
			if (ImGui::Selectable("Add Systems##AddSystemsContentAddSystems"))
			{
				s_Data.openPopup = "AddSystems";
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		if (ImGui::BeginPopupModal("AddSystems"))
		{
			const List<MetadataInfo>& systemMetadatas = Metadata::GetSystemMetadataList();
			
			EU_PERSISTENT b32 ShowEngineSystems = true;
			EU_PERSISTENT b32 ShowProjectSystems = true;

			ImGui::Text("Show Components"); ImGui::SameLine();
			ImGui::Checkbox("Show Engine Systems", (bool*)&ShowEngineSystems); ImGui::SameLine();
			ImGui::Checkbox("Show Project Systems", (bool*)&ShowProjectSystems);
			ImGui::InputText("##SearchSystems", s_Data.generalNameBuffer, 64);

			EU_PERSISTENT bool InitializeSelectedSystems = true;
			EU_PERSISTENT List<bool> SelectedSystems;

			if (InitializeSelectedSystems)
			{
				SelectedSystems.SetCapacityAndElementCount(systemMetadatas.Size());
				memset(&SelectedSystems[0], false, SelectedSystems.Size());
				InitializeSelectedSystems = false;
			}
			if (ShowEngineSystems)
			{
				ImGui::Text("Engine Components");
				ImGui::Columns(4);
				for (u32 i = 0; i < systemMetadatas.Size(); i++)
				{
					if (systemMetadatas[i].id <= Metadata::LastEngineTypeID)
					{
						ImGui::Checkbox((systemMetadatas[i].cls->name + "##AddComponentType_" + systemMetadatas[i].cls->name).C_Str(), &SelectedSystems[i]);
					}

					ImGui::NextColumn();
				}
				ImGui::EndColumns();
			}
			if (ShowProjectSystems)
			{
				ImGui::Text("ProjectComponents");
				ImGui::Columns(4);
				for (u32 i = 0; i < systemMetadatas.Size(); i++)
				{
					if (systemMetadatas[i].id > Metadata::LastEngineTypeID)
					{
						ImGui::Checkbox((systemMetadatas[i].cls->name + "##AddComponentType_" + systemMetadatas[i].cls->name).C_Str(), &SelectedSystems[i]);
					}

					ImGui::NextColumn();
				}
				ImGui::EndColumns();
			}

			if (ImGui::Button("Create Systems##SI_CreateSystems"))
			{
				ECS* ecs = ProjectManager::GetProject()->application->GetECS();
				for (u32 i = 0; i < SelectedSystems.Size(); i++)
				{
					if (SelectedSystems[i])
						ecs->CreateSystem(systemMetadatas[i].id, true);
				}

				InitializeSelectedSystems = true;
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel##SEC_Cancel"))
			{
				InitializeSelectedSystems = true;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	static void DrawMetadataMember(MetadataClass* cls, u32 memberIndex, const u8* data, const String& idString);

	static void DrawMetadataHelper(const MetadataInfo& metadata, const u8* data, const String& idString)
	{
		MetadataClass* cls = metadata.cls;
		for (u32 i = 0; i < cls->members.Size(); i++)
		{
			ImGui::Columns(2);
			DrawMetadataMember(cls, i, data, idString);
			ImGui::EndColumns();
		}
	}

	static void DrawMetadataClass(const MetadataInfo& metadata, const u8* data, const String& idString, String customName = "")
	{
		if (metadata.type == METADATA_CLASS)
		{
			MetadataClass* cls = metadata.cls;
			if (!customName.Empty())
			{
				if (ImGui::TreeNodeEx((customName + idString).C_Str()))
				{
					DrawMetadataHelper(metadata, data, idString + customName);
					ImGui::TreePop();
				}
			}
			else
			{
				if (ImGui::TreeNodeEx((cls->name + idString).C_Str()))
				{
					DrawMetadataHelper(metadata, data, idString);
					ImGui::TreePop();
				}
			}
		}
	}

	static void DrawMetadataMember(MetadataClass* cls, u32 memberIndex, const u8* data, const String& idString)
	{
		const MetadataMember& member = cls->members[memberIndex];
		String memberName = member.name;
		const MetadataInfo& metadata = Metadata::GetMetadata(member.typeID);
		const u8* offsetedData = data + member.offset;

		if (metadata.type == METADATA_PRIMITIVE)
		{
			String newIDString = idString + member.name;

			if (member.typeName == "TextureID")
			{

			}
			else if (member.typeName == "MaterialID")
			{

			}
			else if (member.typeName == "ModelID")
			{

			}
			else if (member.typeName == "MaterialModifierID")
			{

			}
			else
			{
				const MetadataPrimitive* primitive = metadata.primitive;
				switch (primitive->type)
				{
				case METADATA_PRIMITIVE_BOOL: {
					bool* value = (bool*)offsetedData;
					ImGui::Text(memberName.C_Str()); ImGui::SameLine();
					ImGui::NextColumn();
					ImGui::Checkbox(newIDString.C_Str(), value);
					ImGui::NextColumn();
				} break;
				case METADATA_PRIMITIVE_R32: {
					r32* value = (r32*)offsetedData;
					ImGui::Text(memberName.C_Str()); ImGui::SameLine();
					ImGui::NextColumn();
					ImGui::DragFloat(newIDString.C_Str(), value, cls->members[memberIndex].uiSliderSpeed, cls->members[memberIndex].uiSliderMin.x, cls->members[memberIndex].uiSliderMax.x);
					ImGui::NextColumn();
				} break;
				case METADATA_PRIMITIVE_R64: {
					r64* value = (r64*)offsetedData;
					ImGui::Text(memberName.C_Str()); ImGui::SameLine();
					ImGui::NextColumn();
					ImGui::DragFloat(newIDString.C_Str(), (r32*)value, cls->members[memberIndex].uiSliderSpeed, cls->members[memberIndex].uiSliderMin.x, cls->members[memberIndex].uiSliderMax.x);
					ImGui::NextColumn();
				} break;
				case METADATA_PRIMITIVE_U8: {
					u8* value = (u8*)offsetedData;
					ImGui::Text(memberName.C_Str()); ImGui::SameLine();
					ImGui::NextColumn();
					ImGui::DragInt(newIDString.C_Str(), (s32*)value, cls->members[memberIndex].uiSliderSpeed, EU_MAX(cls->members[memberIndex].uiSliderMin.x, 0), EU_MIN(cls->members[memberIndex].uiSliderMax.x, EU_U8_MAX));
					ImGui::NextColumn();
				} break;
				case METADATA_PRIMITIVE_U16: {
					u16* value = (u16*)offsetedData;
					ImGui::Text(memberName.C_Str()); ImGui::SameLine();
					ImGui::NextColumn();
					ImGui::DragInt(newIDString.C_Str(), (s32*)value, cls->members[memberIndex].uiSliderSpeed, EU_MAX(cls->members[memberIndex].uiSliderMin.x, 0), EU_MIN(cls->members[memberIndex].uiSliderMax.x, EU_U16_MAX));
					ImGui::NextColumn();
				} break;
				case METADATA_PRIMITIVE_U32: {
					u32* value = (u32*)offsetedData;
					ImGui::Text(memberName.C_Str()); ImGui::SameLine();
					ImGui::NextColumn();
					if (member.is32BitBool)
						ImGui::Checkbox(newIDString.C_Str(), (bool*)value);
					else
						ImGui::DragInt(newIDString.C_Str(), (s32*)value, cls->members[memberIndex].uiSliderSpeed, EU_MAX(cls->members[memberIndex].uiSliderMin.x, 0), EU_MIN(cls->members[memberIndex].uiSliderMax.x, EU_U32_MAX));

					ImGui::NextColumn();
				} break;
				case METADATA_PRIMITIVE_S8: {
					s8* value = (s8*)offsetedData;
					ImGui::Text(memberName.C_Str()); ImGui::SameLine();
					ImGui::NextColumn();
					ImGui::DragInt(newIDString.C_Str(), (s32*)value, cls->members[memberIndex].uiSliderSpeed, EU_MAX(cls->members[memberIndex].uiSliderMin.x, 0), EU_MIN(cls->members[memberIndex].uiSliderMax.x, EU_S8_MAX));
					ImGui::NextColumn();
				} break;
				case METADATA_PRIMITIVE_S16: {
					s16* value = (s16*)offsetedData;
					ImGui::Text(memberName.C_Str()); ImGui::SameLine();
					ImGui::NextColumn();
					ImGui::DragInt(newIDString.C_Str(), (s32*)value, cls->members[memberIndex].uiSliderSpeed, EU_MAX(cls->members[memberIndex].uiSliderMin.x, 0), EU_MIN(cls->members[memberIndex].uiSliderMax.x, EU_S16_MAX));
					ImGui::NextColumn();
				} break;
				case METADATA_PRIMITIVE_S32: {
					s32* value = (s32*)offsetedData;
					ImGui::Text(memberName.C_Str()); ImGui::SameLine();
					ImGui::NextColumn();
					ImGui::DragInt(newIDString.C_Str(), (s32*)value, cls->members[memberIndex].uiSliderSpeed, EU_MAX(cls->members[memberIndex].uiSliderMin.x, 0), EU_MIN(cls->members[memberIndex].uiSliderMax.x, EU_S32_MAX));
					ImGui::NextColumn();
				} break;
				}
			}
		}
		else if (metadata.type == METADATA_CLASS)
		{
			MetadataClass* memberClass = metadata.cls;
			String newIDString = idString + member.name;
			
			if (cls->members[memberIndex].typeID == Metadata::GetTypeID<v2>())
			{
				ImGui::EndColumns();
				ImGui::Columns(3);
				r32* xValue = (r32*)(offsetedData + memberClass->members[0].offset);
				r32* yValue = (r32*)(offsetedData + memberClass->members[1].offset);
				ImGui::Text((memberName + " ").C_Str()); ImGui::SameLine();
				ImGui::NextColumn();
				ImGui::Text("X"); ImGui::SameLine();
				ImGui::DragFloat((newIDString + "x").C_Str(), xValue, cls->members[memberIndex].uiSliderSpeed, cls->members[memberIndex].uiSliderMin.x, cls->members[memberIndex].uiSliderMax.x);
				ImGui::NextColumn();
				ImGui::Text("Y"); ImGui::SameLine();
				ImGui::DragFloat((newIDString + "y").C_Str(), yValue, cls->members[memberIndex].uiSliderSpeed, cls->members[memberIndex].uiSliderMin.y, cls->members[memberIndex].uiSliderMax.y);
				ImGui::EndColumns();
				ImGui::Columns(2);
			}
			else if (cls->members[memberIndex].typeID == Metadata::GetTypeID<v3>())
			{
				ImGui::EndColumns();
				ImGui::Columns(4);
				r32* xValue = (r32*)(offsetedData + memberClass->members[0].offset);
				r32* yValue = (r32*)(offsetedData + memberClass->members[1].offset);
				r32* zValue = (r32*)(offsetedData + memberClass->members[2].offset);
				ImGui::Text((memberName + " ").C_Str()); ImGui::SameLine();
				ImGui::NextColumn();
				ImGui::Text("X"); ImGui::SameLine();
				ImGui::DragFloat((newIDString + "x").C_Str(), xValue, cls->members[memberIndex].uiSliderSpeed, cls->members[memberIndex].uiSliderMin.x, cls->members[memberIndex].uiSliderMax.x);
				ImGui::NextColumn();
				ImGui::Text("Y"); ImGui::SameLine();
				ImGui::DragFloat((newIDString + "y").C_Str(), yValue, cls->members[memberIndex].uiSliderSpeed, cls->members[memberIndex].uiSliderMin.y, cls->members[memberIndex].uiSliderMax.y);
				ImGui::NextColumn();
				ImGui::Text("Z"); ImGui::SameLine();
				ImGui::DragFloat((newIDString + "z").C_Str(), zValue, cls->members[memberIndex].uiSliderSpeed, cls->members[memberIndex].uiSliderMin.y, cls->members[memberIndex].uiSliderMax.y);
				ImGui::EndColumns();
				ImGui::Columns(2);
			}
			else if (cls->members[memberIndex].typeID == Metadata::GetTypeID<v4>())
			{
				ImGui::EndColumns();
				ImGui::Columns(4);
				r32* xValue = (r32*)(offsetedData + memberClass->members[0].offset);
				r32* yValue = (r32*)(offsetedData + memberClass->members[1].offset);
				r32* zValue = (r32*)(offsetedData + memberClass->members[2].offset);
				r32* wValue = (r32*)(offsetedData + memberClass->members[3].offset);
				ImGui::Text((memberName + " ").C_Str()); ImGui::SameLine();
				ImGui::NextColumn();
				ImGui::Text("X"); ImGui::SameLine();
				ImGui::DragFloat((newIDString + "x").C_Str(), xValue, cls->members[memberIndex].uiSliderSpeed, cls->members[memberIndex].uiSliderMin.x, cls->members[memberIndex].uiSliderMax.x);
				ImGui::NextColumn();
				ImGui::Text("Y"); ImGui::SameLine();
				ImGui::DragFloat((newIDString + "y").C_Str(), yValue, cls->members[memberIndex].uiSliderSpeed, cls->members[memberIndex].uiSliderMin.y, cls->members[memberIndex].uiSliderMax.y);
				ImGui::NextColumn();
				ImGui::Text("Z"); ImGui::SameLine();
				ImGui::DragFloat((newIDString + "z").C_Str(), zValue, cls->members[memberIndex].uiSliderSpeed, cls->members[memberIndex].uiSliderMin.y, cls->members[memberIndex].uiSliderMax.y);
				ImGui::NextColumn();
				ImGui::Text("W"); ImGui::SameLine();
				ImGui::DragFloat((newIDString + "w").C_Str(), zValue, cls->members[memberIndex].uiSliderSpeed, cls->members[memberIndex].uiSliderMin.y, cls->members[memberIndex].uiSliderMax.y);
				ImGui::EndColumns();
				ImGui::Columns(2);
			}
			else
			{
				if (ImGui::TreeNodeEx((member.name + newIDString + "Dropdown").C_Str()))
				{
					DrawMetadataHelper(metadata, offsetedData, newIDString + "Dropdown");
					ImGui::TreePop();
					ImGui::Columns(2);
				}
			}
		}
	}

	static void DrawEntityInspector()
	{
		if (ImGui::Begin("Entity Inspector"))
		{
			
			if (!ProjectManager::GetProject()->loaded || s_Data.selectedEntity == EU_ECS_INVALID_ENTITY_ID)
			{
				ImGui::End();
				return;
			}

			if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(EU_BUTTON_RIGHT))
			{
				s_Data.openPopup = "EntityInspectorAddContent";
			}

			ECS* ecs = ProjectManager::GetProject()->application->GetECS();

			if (ImGui::InputText("##EntityName", s_Data.entityNameBuffer, EU_TEXT_BUFFER_SIZE_ENTITY_NAME))
			{
				ecs->SetEntityName(s_Data.selectedEntity, s_Data.entityNameBuffer);
			} ImGui::SameLine();
			if (ImGui::Button("CLEAR##EntityInspectorClearName"))
			{
				s_Data.entityNameBuffer[0] = 0;
				ecs->SetEntityName(s_Data.selectedEntity, "");
			}

			ECSEntityContainer* container = &ecs->GetAllEntities_()[s_Data.selectedEntity - 2];
			
			for (u32 i = 0; i < container->components.Size(); i++)
			{
				ECSComponentContainer* componentContainer = &container->components[i];
				const MetadataInfo& componentMetadata = Metadata::GetMetadata(componentContainer->typeID);
				DrawMetadataClass(componentMetadata, (u8*)componentContainer->actualComponent, "##EI");
			}
		}
		ImGui::End();
	}

	static void DrawSystemInspector()
	{
		if (ImGui::Begin("Systems"))
		{
			if (!ProjectManager::GetProject()->loaded)
			{
				ImGui::End();
				return;
			}

			ECS* ecs = ProjectManager::GetProject()->application->GetECS();

			if (ecs->GetActiveScene() == EU_ECS_INVALID_SCENE_ID)
			{
				ImGui::End();
				return;
			}

			List<ECSSystemContainer>& systems = ecs->GetSystems_();
			for (u32 i = 0; i < systems.Size(); i++)
			{
				ECSSystemContainer* system = &systems[i];
				const MetadataInfo& systemMetadata = Metadata::GetMetadata(system->typeID);
				String idString = "##SystemsWindow" + systemMetadata.cls->name;

				if (!system->actualSystem->enabled)
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.35f, 0.3f, 0.3f, 1.0));

				if (!system->actualSystem->enabled)
					ImGui::PopStyleColor();

				DrawMetadataClass(systemMetadata, (const u8*)system->actualSystem, "##SystemViewer");
				
			}

			if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(EU_BUTTON_RIGHT))
			{
				s_Data.openPopup = "AddSystemsContent";
			}

			if (ImGui::Button("Add Systems##SystemsWindowAddSystemsButton"))
			{
				s_Data.openPopup = "AddSystems";
			}
		}
		ImGui::End();
	}

	static void DrawRenderersWindow()
	{
		if (ImGui::Begin("Renderers"))
		{
			MasterRenderer* renderer = Engine::GetRenderer();
			if (ImGui::TreeNodeEx("Master Renderer"))
			{
				
				ImGui::TreePop();
			}
			if (ImGui::TreeNodeEx("Batch Renderer2D"))
			{
				Renderer2D* renderer2D = renderer->GetRenderer2D();
				ImGui::TreePop();
			}
			if (ImGui::TreeNodeEx("Renderer3D"))
			{
				Renderer3D* renderer3D = renderer->GetRenderer3D();
				ImGui::TreePop();
			}
		}

		ImGui::End();
	}

	void EditorGUI::RenderGUI()
	{
		RenderContext* rc = Engine::GetRenderContext();
		u32 width, height;
		rc->GetFramebufferSize(s_Data.renderPass, &width, &height);

		ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		bool open;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("EunoiaEditor", &open, window_flags);
		ImGui::PopStyleVar(3);

		ImGuiID dockspace_id = ImGui::GetID("eunoia");

		if (ImGui::DockBuilderGetNode(dockspace_id) == 0)
		{
			ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
			ImGuiID test = ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace); // Add empty node
			ImGui::DockBuilderSetNodeSize(dockspace_id, ImVec2(width, height));
			ImGuiID dock_main_id = dockspace_id;
			ImGuiID dock_up_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.12f, 0, &dock_main_id);
			ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, 0, &dock_main_id);
			ImGuiID dock_right_down_id = ImGui::DockBuilderSplitNode(dock_right_id, ImGuiDir_Right, 0.25f, 0, &dock_right_id);
			ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.2f, 0, &dock_main_id);
			ImGuiID dock_center_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 1.0f, 0, &dock_left_id);
			ImGuiID dock_down_id = ImGui::DockBuilderSplitNode(dock_center_id, ImGuiDir_Down, 0.2f, 0, &dock_center_id);

			
		}

		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

		DrawMainMenuBar();
		DrawGameWindow();
		DrawAssetBrowser();
		DrawEntityEntityHierarchyWindow();
		DrawEntityInspector();
		DrawSystemInspector();
		DrawRenderersWindow();

		DrawPopupWindows();

		ImGui::End();
	}
	

}