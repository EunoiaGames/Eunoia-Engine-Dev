#pragma once

#include <Eunoia\Core\Application.h>
#include <Eunoia\DataStructures\List.h>
#include <Eunoia\ECS\ECSLoader.h>
#include <Eunoia\Utils\FileUtils.h>
#include <Eunoia\Rendering\Renderer2D.h>
#include <Eunoia\Rendering\Renderer3D.h>

namespace Eunoia_Editor {

	struct EunoiaProject
	{
		Eunoia::String name;
		b32 loaded;
		HMODULE dllHandle;
		Eunoia::Application* application;
		Eunoia::ECSResetPoint resetPoint;
		Eunoia::EUDirectory* assetDirectory;

		b32 stepApplication;
		b32 stepPaused;
	};

	enum SystemFunctionFlags
	{
		SYSTEM_FUNCTION_PreUpdate = 1,
		SYSTEM_FUNCTION_Update = 2,
		SYSTEM_FUNCTION_PostUpdate = 4,
		SYSTEM_FUNCTION_PreRender = 8,
		SYSTEM_FUNCTION_Render = 16,
		SYSTEM_FUNCTION_PostRender = 32,
	};

	class ProjectManager
	{
	public:
		static void Init();
		static b32 LoadProject(const Eunoia::String& name, b32 compile);
		static b32 RecompileProject();
		static b32 SaveProject();
		static b32 CreateNewProject(const Eunoia::String& name);
		static b32 DeleteProject(const Eunoia::String& name);
		static b32 CreateFinalBuild();

		static void CreateNewComponentType(const Eunoia::String& name);
		static void CreateNewSystemType(const Eunoia::String& name, u32 functionFlag);
		static void CreateNewEventType(const Eunoia::String& name);

		static void WriteScenesToFiles();
		static void WriteAssetsToFiles();
		static void WriteAssetsInDirectory(Eunoia::EUDirectory* dir);
		static void WriteEunoiaConfigToFile();

		static EunoiaProject* GetProject();

		static void OpenVS2019();

		static const Eunoia::List<Eunoia::String>& GetProjectNamesInProjectFolder();
		static Eunoia::String GetSpritePosOriginString(Eunoia::SpritePosOrigin origin);
	private:
		static void WriteScenesToFilesHelper(Eunoia::EUDirectory* directory);
		static void OpenVS2019Helper();
	};

}