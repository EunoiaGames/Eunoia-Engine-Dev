#pragma once

#include <Eunoia\Core\Application.h>

namespace Eunoia_Editor {

	class EditorApp : public Eunoia::Application
	{
	public:
		virtual void Init() override;
		virtual void OnClose() override;
		virtual void Update(r32 dt) override;
		virtual void Render() override;
		virtual void EndFrame() override;
		virtual void PrePhysicsSimulation(r32 dt) override;
		virtual void PostPhysicsSimulation(r32 dt) override;
	};

}