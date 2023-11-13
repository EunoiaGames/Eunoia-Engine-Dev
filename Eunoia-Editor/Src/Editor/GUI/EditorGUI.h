#pragma once

#include "../EditorSettings.h"
#include <Eunoia\ECS\ECSTypes.h>
#include <Eunoia\Metadata\MetadataInfo.h>
#include <Eunoia\Rendering\Asset\LoadedMaterial.h>

namespace Eunoia_Editor {

	enum EngineCamera
	{
		ENGINE_CAMERA_2D,
		ENGINE_CAMERA_3D,
		ENGINE_CAMERA_NONE
	};

	class EditorGUI
	{
	public:
		static void Init();
		static void Destroy();
		static void UpdateInput();
		static void BeginFrame();
		static void EndFrame();
		static void RenderGUI();
	private:
		static void InitImGui();
		static void InitDarkTheme();
		static void InitRenderPass();
		static void InitResources();
	private:
		

		static b32 CheckForShortcut(const EditorShortcut& shortcut);
	};

}