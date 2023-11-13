#pragma once

#include "../Rendering/RenderContext.h"
#include "../Rendering/MasterRenderer.h"
#include "../Math/Math.h"
#include "../Rendering/DisplayInfo.h"
#include "../Physics/PhysicsEngine3D.h"

#define EU_MAIN_APPLICATION 0

namespace Eunoia {

	typedef u32 EngineApplicationHandle;

	class Application;
	class EU_API Engine
	{
	public:
		static void Init(Application* app, const String& title, u32 width, u32 height, RenderAPI api, b32 editorAttached = true);
		static void Start();
		static void Stop();

		static EngineApplicationHandle AddApplication(Application* app, b32 setActive = false);
		static void SetActiveApplication(EngineApplicationHandle handle);

		static r32 GetTime();
		static b32 IsEditorAttached();

		static Application* GetActiveApplication();
		static Application* GetApplication(EngineApplicationHandle handle);

		static RenderContext* GetRenderContext();
		static Display* GetDisplay();
		static MasterRenderer* GetRenderer();
		static PhysicsEngine3D* GetPhysicsEngine3D();
	private:
		static void Update(r32 dt);
		static void Render();
	};

}
