#pragma once

#include "../Common.h"
#include "../ECS/ECS.h"
#include "../Rendering/RenderContext.h"
#include "../Rendering/DisplayInfo.h"

namespace Eunoia {

	struct ApplicationInfo
	{
		DisplayInfo displayInfo;
		String versionString;
		RenderAPI renderAPI;
	};

	class EU_API Application
	{
	public:
		virtual void Init() = 0;
		virtual void RegisterMetadata() {};

		virtual void OnRecompile() {}
		virtual void OnClose() {}

		virtual void Update(r32 dt) {}
		virtual void Render() {}
		virtual void EndFrame() {}
		virtual void PrePhysicsSimulation(r32 dt) {}
		virtual void PostPhysicsSimulation(r32 dt) {}

		inline ECS* GetECS() { return m_ECS; }
		inline void InitECS() { m_ECS = new ECS(); }
		inline void BeginECS() { m_ECS->Begin(); }
		inline void UpdateECS(r32 dt) { m_ECS->UpdateSystems(dt); }
		inline void RenderECS() { m_ECS->RenderSystems(); }
	protected:
		ECS* m_ECS;
	};
}