#include "ProjectManager.h"
#include <filesystem>
#include <Eunoia\ECS\Systems\TransformHierarchy2DSystem.h>
#include <Eunoia\ECS\Systems\TransformHierarchy3DSystem.h>
#include <Eunoia\Rendering\Asset\AssetManager.h>
#include <Eunoia\Rendering\Asset\MaterialLoader.h>
#include <Eunoia\Utils\SettingsLoader.h>
#include <Eunoia\Core\Engine.h>

typedef Eunoia::Application* (*CreateApplicationFunction) (Eunoia::ApplicationInfo* appInfo);

namespace Eunoia_Editor {

	using namespace Eunoia;

	struct ProjectManager_Data
	{
		EunoiaProject project;
		std::thread thread;
		List<String> projectNames;
	};

	static ProjectManager_Data s_Data;

	void ProjectManager::Init()
	{
		s_Data.project.loaded = false;
		s_Data.project.stepApplication = false;
		s_Data.project.stepPaused = false;

		std::filesystem::directory_iterator it("../EunoiaProjects");
		for (const auto& entry : it)
		{
			if (!entry.is_directory())
				continue;
			Eunoia::String projPath = Eunoia::String(entry.path().generic_string().c_str());
			s_Data.projectNames.Push(projPath.SubString(projPath.FindLastOf("/") + 1));
		}
	}

	b32 ProjectManager::LoadProject(const Eunoia::String& name, b32 compile)
	{
		EunoiaProject* project = &s_Data.project;

		b32 found = false;
		for (u32 i = 0; i < s_Data.projectNames.Size(); i++)
		{
			if (name == s_Data.projectNames[i])
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			EU_LOG_ERROR("Tried to load a project that doesn't exist");
			return false;
		}

		String configuration = "Debug";
		String buildFolder = "Debug-windows-x86_64";

		if (compile)
		{
			String genProjectFilesCommand = "call GenProjectFiles-VS2019.bat \"" + project->name + "\"";
			std::system(genProjectFilesCommand.C_Str());

			String CompileProjectCommand = "call CompileProj.bat ../EunoiaProjects/" + name + "/" + name + ".vcxproj " + configuration;
			std::system(CompileProjectCommand.C_Str());
		}
		
		if (project->loaded)
		{
			FreeLibrary(project->dllHandle);
			project->dllHandle = 0;
		}

		String dllPath = "../EunoiaProjects/" + name + "/Bin/" + buildFolder + "/" + name + ".dll";
		project->dllHandle = LoadLibraryA(dllPath.C_Str());
		if (!project->dllHandle)
		{
			EU_LOG_ERROR("Could not load project DLL");
			return false;
		}

		CreateApplicationFunction createApplication = (CreateApplicationFunction)GetProcAddress(project->dllHandle, "CreateApplication");
		if (!createApplication)
		{
			EU_LOG_ERROR("Could not load CreateApplication function from project dll");
			return false;
		}

		Eunoia::ApplicationInfo appInfo;
		project->application = createApplication(&appInfo);
		project->application->InitECS();
		project->application->Init();
		Eunoia::Metadata::UnregisterProjectMetadata();
		project->application->RegisterMetadata();

		project->name = name;
		project->loaded = true;
		project->stepApplication = false;
		project->stepPaused = false;
		project->assetDirectory = new EUDirectory("../EunoiaProjects/" + name + "/Assets");

		SettingsLoader::LoadSettingsFromFile("../EunoiaProjects/" + name + "/Settings.eucfg", true);

		EU_LOG_INFO("Loaded project");

		return true;
	}

	b32 ProjectManager::RecompileProject()
	{
		EunoiaProject* project = &s_Data.project;
		if (!project->loaded)
		{
			EU_LOG_WARN("Tried to recompile project before loading one first");
			return false;
		}
		
		String configuration = "Debug";
		String buildFolder = "Debug-windows-x86_64";

		String genProjectFilesCommand = "call GenProjectFiles-VS2019.bat \"" + project->name + "\"";
		std::system(genProjectFilesCommand.C_Str());

		FreeLibrary(project->dllHandle);
		Eunoia::String compileProjectCommand = "call CompileProj.bat ../EunoiaProjects/" + project->name + "/" + project->name + ".vcxproj " + configuration;
		std::system(compileProjectCommand.C_Str());

		Eunoia::String dllPath = "../EunoiaProjects/" + project->name + "/Bin/" + buildFolder + "/" + project->name + ".dll";
		project->dllHandle = LoadLibraryA(dllPath.C_Str());
		if (!project->dllHandle)
		{
			EU_LOG_ERROR("Could not load project DLL");
			return false;
		}

		CreateApplicationFunction createApplication = (CreateApplicationFunction)GetProcAddress(project->dllHandle, "CreateApplication");
		if (!createApplication)
		{
			EU_LOG_ERROR("Could not load CreateApplication function from project dll");
			return false;
		}



		// s_Project.application->GetECS();
		//delete s_Project.application;

		project->application->GetECS()->CreateResetPoint(&project->resetPoint);

		Eunoia::ApplicationInfo appInfo;
		project->application = createApplication(&appInfo);
		project->application->InitECS();
		Eunoia::Metadata::UnregisterProjectMetadata();
		project->application->RegisterMetadata();
		project->application->GetECS()->RestoreResetPoint(project->resetPoint);
		project->application->OnRecompile();

		EU_LOG_INFO("Recompiled project");

		return true;
	}

	b32 ProjectManager::SaveProject()
	{
		EunoiaProject* project = &s_Data.project;
		if (!project->loaded)
			return false;

		WriteScenesToFiles();
		WriteAssetsToFiles();
		WriteEunoiaConfigToFile();

		return true;
	}

	b32 ProjectManager::CreateNewProject(const Eunoia::String& name)
	{
		EunoiaProject* project = &s_Data.project;

		const String rootProjectDir = "../EunoiaProjects/";

		String projectSrcDir = rootProjectDir + name + "/Src";
		String mkdirCmd = "mkdir \"" + projectSrcDir + "\"";
		std::system(mkdirCmd.C_Str());

		mkdirCmd = "mkdir \"" + projectSrcDir + "/Components\"";
		std::system(mkdirCmd.C_Str());
		mkdirCmd = "mkdir \"" + projectSrcDir + "/Systems\"";
		std::system(mkdirCmd.C_Str());
		mkdirCmd = "mkdir \"" + projectSrcDir + "/Events\"";
		std::system(mkdirCmd.C_Str());

		String headerFileText = "#pragma once\n#include <Eunoia\\Eunoia.h>\n\nusing namespace Eunoia;\n\nclass EU_PROJ " + name + 
			" : public Application\n{\npublic:\n\tvoid Init() override;\n\tvoid OnClose() override;\n\tvoid RegisterMetadata() override;"
			"\n\n};\n\nextern \"C\" EU_PROJ Application* CreateApplication(ApplicationInfo* appInfo);";

		String sourceFileText = "#include \"" + name + ".h\"\n\nvoid " + name + "::Init()\n{\n\n}\n\nvoid " + name +
			"::OnClose()\n{\n\n}\n\nApplication* CreateApplication(ApplicationInfo* appInfo)\n{\n\tappInfo->renderAPI = RENDER_API_VULKAN;\n\tappInfo->versionString = \"1.0.0\";\n\tappInfo->displayInfo.title = \"" 
			+ name + "\";\n\tappInfo->displayInfo.width = 1920;\n\tappInfo->displayInfo.height = 1080;\n\n\treturn new " + name + "();\n}";

		String genFileText = "#include \"" + name + ".h\"";

		Eunoia::String location = projectSrcDir + "/" + name + ".h";
		FILE* file = fopen(location.C_Str(), "w");
		fwrite(headerFileText.C_Str(), 1, headerFileText.Length(), file);
		fclose(file);

		location = projectSrcDir + "/" + name + ".cpp";
		file = fopen(location.C_Str(), "w");
		fwrite(sourceFileText.C_Str(), 1, sourceFileText.Length(), file);
		fclose(file);

		location = projectSrcDir + "/" + name + "_Generated.cpp";
		file = fopen(location.C_Str(), "w");
		fwrite(genFileText.C_Str(), 1, genFileText.Length(), file);
		fclose(file);

		Eunoia::String genProjectFilesCommand = "call GenProjectFiles-VS2019.bat \"" + name + "\"";
		std::system(genProjectFilesCommand.C_Str());

		Eunoia::String configuration = "Debug";
		Eunoia::String buildFolder = "Debug-windows-x86_64";

		Eunoia::String compileProjectCommand = "call CompileProj.bat ../EunoiaProjects/" + name + "/" + name + ".vcxproj " + configuration;
		std::system(compileProjectCommand.C_Str());

		String dllPath = "../EunoiaProjects/" + name + "/Bin/" + buildFolder + "/" + name + ".dll";
		project->dllHandle = LoadLibraryA(dllPath.C_Str());
		if (!project->dllHandle)
		{
			EU_LOG_ERROR("Could not load project DLL");
			return false;
		}

		CreateApplicationFunction createApplication = (CreateApplicationFunction)GetProcAddress(project->dllHandle, "CreateApplication");
		if (!createApplication)
		{
			EU_LOG_ERROR("Could not load CreateApplication function from project dll");
			return false;
		}

		ApplicationInfo appInfo;
		project->application = createApplication(&appInfo);
		project->application->InitECS();
		project->application->Init();
		Eunoia::Metadata::UnregisterProjectMetadata();
		project->application->RegisterMetadata();

		project->name = name;
		project->loaded = true;
		project->stepApplication = false;
		project->stepPaused = false;
		project->assetDirectory = new EUDirectory("../EunoiaProjects/" + name + "/Assets");

		s_Data.projectNames.Push(name);

		EU_LOG_INFO("Created new project");

		return true;
	}

	b32 ProjectManager::DeleteProject(const Eunoia::String& name)
	{
		return b32();
	}

	b32 ProjectManager::CreateFinalBuild()
	{
		EunoiaProject* project = &s_Data.project;
		if (!project->loaded)
			return false;

		EUDirectory finalBuildDir("../EunoiaProjects/" + project->name + "/FinalBuild");
		finalBuildDir.CreateNewFile("TempMain.cpp");

		FILE* tempMainFile = fopen(("../EunoiaProjects/" + project->name + "/FinalBuild/TempMain.cpp").C_Str(), "w");

		SaveProject();
		mem_size fileSize = 0;
		u8* data = FileUtils::LoadBinaryFile("../EunoiaProjects/" + project->name + "/" + project->name + "ECS.euecs", &fileSize);

		String dataText = "Eunoia::List<u8> ecsData = {";
		for (u32 i = 0; i < fileSize; i++)
		{
			dataText += String::S32ToString(data[i]);
			if (i != (fileSize - 1))
				dataText += ",";
		}
		free(data);
		dataText += "};Eunoia::ECSLoadedData l;Eunoia::ECSLoader::LoadECSDataFromMemory(&l,ecsData);";

		EunoiaSettings settings;
		SettingsLoader::LoadSettingsFromFile("../EunoiaProjects/" + project->name + "/Settings.eucfg", false);
		u8* settingsData = (u8*)&settings;
		String settingsDataText = "Eunoia::List<u8> settingsData = {";
		for (u32 i = 0; i < sizeof(EunoiaSettings); i++)
		{
			settingsDataText += String::S32ToString(settingsData[i]);
			if (i != (sizeof(EunoiaSettings) - 1))
				settingsDataText += ",";
		}
		settingsDataText += "};Eunoia::EunoiaSettings* settings = (Eunoia::EunoiaSettings*)&settingsData[0];Eunoia::SettingsLoader::ApplySettings(*settings);";

		String loadDataText = dataText + settingsDataText;

		String text = "#include \"../Src/" + project->name + ".h\"\nvoid main(){Eunoia::ApplicationInfo i;Eunoia::Application* a = CreateApplication(&i);Eunoia::Engine::Init(a,i.displayInfo.title,i.displayInfo.width,i.displayInfo.height,i.renderAPI);" 
			+ loadDataText + "a->GetECS()->InitFromLoadedDataFormat(l);Eunoia::Engine::Start();delete a;}";

		fwrite(text.C_Str(), 1, text.Length(), tempMainFile);
		fclose(tempMainFile);

		String createFinalBuildCommand = "call CreateFinalProjBuild.bat " + project->name;
		std::system(createFinalBuildCommand.C_Str());

		FileUtils::EUDeleteFile("../EunoiaProjects/" + project->name + "/FinalBuild/TempMain.cpp");

		std::string dst = std::filesystem::absolute("../Bin/Dist-windows-x86_64/Eunoia-Engine/Eunoia-Engine.dll").generic_string();
		std::string src = std::filesystem::absolute(Eunoia::String("../EunoiaProjects/" + project->name + "/FinalBuild").C_Str()).generic_string();
		std::string copyDLLCommand = "xcopy /c /i /r /f \"" + dst + "\" \"" + src + "\"";
		std::system(copyDLLCommand.c_str());

		EU_LOG_INFO("Created final project build");
	}

	void ProjectManager::CreateNewComponentType(const Eunoia::String& name)
	{
		EunoiaProject* project = &s_Data.project;

		if (!s_Data.project.loaded)
		{
			EU_LOG_WARN("Tried to new component type before loading a project first");
			return;
		}

		String fileLocation = "../EunoiaProjects/" + project->name + "/Src/Components/" + name + ".h";
		FILE* file = fopen(fileLocation.C_Str(), "w");
		String text = "#include <Eunoia\\Eunoia.h>\n\nEU_REFLECT(Component)\nstruct " + name + " : public Eunoia::ECSComponent\n{\n\n};";
		fwrite(text.C_Str(), 1, text.Length(), file);
		fclose(file);
	}

	void ProjectManager::CreateNewSystemType(const Eunoia::String& name, u32 functionFlags)
	{
		EunoiaProject* project = &s_Data.project;

		Eunoia::String fileLocation = "../EunoiaProjects/" + project->name + "/Src/Systems/" + name + ".h";
		FILE* file = fopen(fileLocation.C_Str(), "w");
		Eunoia::String text = "#include <Eunoia\\Eunoia.h>\n\nEU_REFLECT(System)\nclass EU_PROJ " + name + " : public Eunoia::ECSSystem\n{\npublic:\n\t" + name + "();\n";

		if (EU_HAS_FLAG(functionFlags, SYSTEM_FUNCTION_PreUpdate))
			text += "\n\tvirtual void PreUpdate(r32 dt) override;";
		if (EU_HAS_FLAG(functionFlags, SYSTEM_FUNCTION_Update))
			text += "\n\tvirtual void ProcessEntityOnUpdate(Eunoia::EntityID entity, r32 dt) override;";
		if (EU_HAS_FLAG(functionFlags, SYSTEM_FUNCTION_PostUpdate))
			text += "\n\tvirtual void PostUpdate(r32 dt) override;";
		if (EU_HAS_FLAG(functionFlags, SYSTEM_FUNCTION_PreRender))
			text += "\n\tvirtual void PreRender() override;";
		if (EU_HAS_FLAG(functionFlags, SYSTEM_FUNCTION_Render))
			text += "\n\tvirtual void ProcessEntityOnRender(Eunoia::EntityID entity) override;";
		if (EU_HAS_FLAG(functionFlags, SYSTEM_FUNCTION_PostRender))
			text += "\n\tvirtual void PostRender() override;";

		text += "\n};";

		fwrite(text.C_Str(), 1, text.Length(), file);
		fclose(file);

		fileLocation = "../EunoiaProjects/" + project->name + "/Src/Systems/" + name + ".cpp";
		file = fopen(fileLocation.C_Str(), "w");
		text = "#include \"" + name + ".h\"\n\n" + name + "::" + name + "()\n{\n\n}";;

		if (EU_HAS_FLAG(functionFlags, SYSTEM_FUNCTION_PreUpdate))
			text += "\n\nvoid " + name + "::PreUpdate(r32 dt)\n{\n\n}";
		if (EU_HAS_FLAG(functionFlags, SYSTEM_FUNCTION_Update))
			text += "\n\nvoid " + name + "::ProcessEntityOnUpdate(Eunoia::EntityID entity, r32 dt)\n{\n\n}";
		if (EU_HAS_FLAG(functionFlags, SYSTEM_FUNCTION_PostUpdate))
			text += "\n\nvoid " + name + "::PostUpdate(r32 dt)\n{\n\n}";
		if (EU_HAS_FLAG(functionFlags, SYSTEM_FUNCTION_PreRender))
			text += "\n\nvoid " + name + "::PreRender()\n{\n\n}";
		if (EU_HAS_FLAG(functionFlags, SYSTEM_FUNCTION_Render))
			text += "\n\nvoid " + name + "::ProcessEntityOnRender(Eunoia::EntityID entity)\n{\n\n}";
		if (EU_HAS_FLAG(functionFlags, SYSTEM_FUNCTION_PostRender))
			text += "\n\nvoid " + name + "::PostRender()\n{\n\n}";

		fwrite(text.C_Str(), 1, text.Length(), file);
		fclose(file);
	}

	void ProjectManager::CreateNewEventType(const Eunoia::String& name)
	{
		EunoiaProject* project = &s_Data.project;

		Eunoia::String fileLocation = "../EunoiaProjects/" + project->name + "/Src/Events/" + name + ".h";
		FILE* file = fopen(fileLocation.C_Str(), "w");

		String text = "#include <Eunoia\\Eunoia.h>\n\nEU_REFLECT(Event)\nstruct " + name + " : public Eunoia::ECSEvent\n{\n};";
		fwrite(text.C_Str(), 1, text.Length(), file);
		fclose(file);
	}

	void ProjectManager::WriteScenesToFiles()
	{
		EunoiaProject* project = &s_Data.project;
		if (!project->loaded)
			return;

		WriteScenesToFilesHelper(project->assetDirectory);
	}

	void ProjectManager::WriteScenesToFilesHelper(Eunoia::EUDirectory* directory)
	{
		for (u32 i = 0; i < directory->files.Size(); i++)
		{
			const EUFile& file = directory->files[i];
			if (file.extension == "euscene" && file.userData)
			{
				SceneID scene = *(SceneID*)file.userData;
				ECSLoadedScene loadedScene;
				s_Data.project.application->GetECS()->ConvertSceneToLoadedDataFormat(&loadedScene, scene);
				ECSLoader::WriteLoadedSceneToFile(loadedScene, file.path);
			}
		}

		for (u32 i = 0; i < directory->directories.Size(); i++)
			WriteScenesToFilesHelper(directory->directories[i]);
	}

	void ProjectManager::WriteAssetsToFiles()
	{
		EunoiaProject* project = &s_Data.project;
		if (!project->loaded)
			return;

		WriteAssetsInDirectory(project->assetDirectory);
	}

	void ProjectManager::WriteAssetsInDirectory(EUDirectory* dir)
	{
		for (u32 i = 0; i < dir->files.Size(); i++)
		{
			if (dir->files[i].extension == "eumat")
			{
				MaterialID mid = AssetManager::GetMaterialID(dir->files[i].name);

				if (mid == EU_INVALID_MATERIAL_ID)
					continue;

				const Material& material = AssetManager::GetMaterial(mid);
				LoadedMaterial loadedMaterial;
				loadedMaterial.name = material.name;
				for (u32 j = 0; j < NUM_MATERIAL_TEXTURE_TYPES; j++)
				{
					loadedMaterial.texturePaths[(MaterialTextureType)j] = AssetManager::GetTexturePath(material.textures[j]);
				}
				LoadedMaterialFile writeData;
				writeData.materials.Push(loadedMaterial);
				MaterialLoader::WriteEumtlMaterial(dir->files[i].path, writeData);
			}
		}

		for (u32 i = 0; i < dir->directories.Size(); i++)
			WriteAssetsInDirectory(dir->directories[i]);
	}

	void ProjectManager::WriteEunoiaConfigToFile()
	{
		EunoiaProject* project = &s_Data.project;
		if (!project->loaded)
			return;

		Renderer2D* r2D = Engine::GetRenderer()->GetRenderer2D();
		Renderer3D* r3D = Engine::GetRenderer()->GetRenderer3D();

		EunoiaSettings settings;
		settings.settings2D.origin = r2D->GetSpritePosOrigin();
		settings.settings2D.projection = r2D->GetOrthographic();
		settings.settings3D.ambient = r3D->GetAmbient();
		settings.settings3D.lightingModel = r3D->GetLightingModel();
		settings.settings3D.bloomThreshold = r3D->GetBloomThreshold();
		settings.settings3D.bloomBlurIterCount = r3D->GetBloomBlurIterCount();

		SettingsLoader::WriteSettingsToFile("../EunoiaProjects/" + project->name + "/Settings.eucfg", settings);
	}

	EunoiaProject* ProjectManager::GetProject()
	{
		return &s_Data.project;
	}

	void ProjectManager::OpenVS2019()
	{
		s_Data.thread = std::thread(OpenVS2019Helper);
		s_Data.thread.detach();
	}

	void ProjectManager::OpenVS2019Helper()
	{
		if (!s_Data.project.loaded)
		{
			EU_LOG_WARN("Tried to open VS2019 before loading project");
			return;
		}

		Eunoia::String openVisualStudioCommand = "call \"../EunoiaProjects/" + s_Data.project.name + "/" + s_Data.project.name + ".sln\"";
		std::system(openVisualStudioCommand.C_Str());
	}

	String ProjectManager::GetSpritePosOriginString(SpritePosOrigin origin)
	{
		switch (origin)
		{
		case SPRITE_POS_ORIGIN_BOTTOM_LEFT: return "Bottom Left";
		case SPRITE_POS_ORIGIN_BOTTOM_RIGHT: return "Bottom Right";
		case SPRITE_POS_ORIGIN_CENTER: return "Center";
		case SPRITE_POS_ORIGIN_TOP_LEFT: return "Top Left";
		case SPRITE_POS_ORIGIN_TOP_RIGHT: return "Top Right";
		}
	}

	const List<String>& ProjectManager::GetProjectNamesInProjectFolder()
	{
		return s_Data.projectNames;
	}
	
}
