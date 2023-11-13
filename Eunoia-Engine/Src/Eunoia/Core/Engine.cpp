#include "Engine.h"
#include "../Utils/Log.h"
#include "Application.h"
#include "../Rendering/MasterRenderer.h"
#include "../Rendering/Asset/AssetManager.h"
#include "Input.h"
#include "../Metadata/Metadata.h"
#include "../ECS/Systems/TransformHierarchy3DSystem.h"
#include "../ECS/Systems/TransformHierarchy2DSystem.h"
#include "../Rendering/GuiManager.h"
#include "../Physics/PhysicsEngine3D.h"
namespace Eunoia {

	struct Engine_Data
	{
		List<Application*> applications;
		Application* activeApp;
		b32 running;
		r32 timeInSeconds;
		b32 editorAttached;

		Display* display;
		RenderContext* renderContext;
		MasterRenderer* renderer;
		PhysicsEngine3D* physicsEngine;
	};

	static Engine_Data s_Data;

	void Engine::Init(Application* app, const String& title, u32 width, u32 height, RenderAPI api, b32 editorAttached)
	{
		Logger::Init();
		Metadata::Init();
		ECSLoader::Init();
		
		if (!app->GetECS())
		{
			app->InitECS();
		}

		s_Data.editorAttached = editorAttached;
		s_Data.applications.Push(app);
		s_Data.activeApp = app;
		
		DisplayInfo displayInfo;
		displayInfo.title = title;
		displayInfo.width = width;
		displayInfo.height = height;

		s_Data.display = Display::CreateDisplay();
		s_Data.display->Create(title, width, height);
		s_Data.renderContext = RenderContext::CreateRenderContext(api);
		s_Data.renderContext->Init(s_Data.display);
		s_Data.renderer = new MasterRenderer(s_Data.renderContext, s_Data.display);
		s_Data.renderer->Init();
		s_Data.physicsEngine = new PhysicsEngine3D();
		s_Data.physicsEngine->Init();

		AssetManager::Init();

		EUInput::InitInput();
		GuiManager::Init();
	}

	void Engine::Start()
	{
		s_Data.running = true;
		s_Data.activeApp->Init();

		r32 dt = 1.0f / 60.0f;
		while (s_Data.running)
		{	
			if (s_Data.display->CheckForEvent(DISPLAY_EVENT_CLOSE))
			{
				Stop();
			}

			Update(dt);
			Render();

			s_Data.timeInSeconds += dt;
		}
	}

	void Engine::Stop()
	{
		s_Data.running = false;
	}

	EngineApplicationHandle Engine::AddApplication(Application* app, b32 setActive)
	{
		EngineApplicationHandle handle = s_Data.applications.Size();
		s_Data.applications.Push(app);

		if (setActive)
			s_Data.activeApp = app;

		return handle;
	}

	void Engine::SetActiveApplication(EngineApplicationHandle handle)
	{
		s_Data.activeApp = s_Data.applications[handle];
	}

	RenderContext* Engine::GetRenderContext()
	{
		return s_Data.renderContext;
	}

	MasterRenderer* Engine::GetRenderer()
	{
		return s_Data.renderer;
	}

	Display* Engine::GetDisplay()
	{
		return s_Data.display;
	}

	PhysicsEngine3D* Engine::GetPhysicsEngine3D()
	{
		return s_Data.physicsEngine;
	}

	r32 Engine::GetTime()
	{
		return s_Data.timeInSeconds;
	}

	b32 Engine::IsEditorAttached()
	{
		return s_Data.editorAttached;
	}

	Application* Engine::GetActiveApplication()
	{
		return s_Data.activeApp;
	}

	Application* Engine::GetApplication(EngineApplicationHandle handle)
	{
		return s_Data.applications[handle];
	}

	void Engine::Update(r32 dt)
	{
		EUInput::BeginInput();
		s_Data.activeApp->BeginECS();
		s_Data.activeApp->UpdateECS(dt);
		s_Data.activeApp->Update(dt);

		for (u32 i = 0; i < s_Data.applications.Size(); i++)
			if (s_Data.applications[i] != s_Data.activeApp)
				s_Data.applications[i]->GetECS()->OnlyUpdateRequiredSystems(dt);

		//s_Data.activeApp->GetECS()->PrePhysicsSystems(dt);
		//s_Data.activeApp->PrePhysicsSimulation(dt);

		//s_Data.physicsEngine->StepSimulation(dt);

		//s_Data.activeApp->GetECS()->PostPhysicsSystems(dt);
		//s_Data.activeApp->PostPhysicsSimulation(dt);
		
		EUInput::UpdateInput();
	}

	void Engine::Render()
	{	
		s_Data.renderContext->BeginFrame();
		s_Data.renderer->BeginFrame();
		
		s_Data.activeApp->RenderECS();
		s_Data.activeApp->Render();

		s_Data.renderer->EndFrame();
		s_Data.renderer->RenderFrame();
		s_Data.activeApp->EndFrame();

		s_Data.renderContext->Present();
		s_Data.display->Update();
	}

}
