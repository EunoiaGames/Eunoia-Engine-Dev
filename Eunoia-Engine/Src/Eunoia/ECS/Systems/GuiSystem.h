#pragma once

#include "../ECS.h"
#include "../../Rendering/Renderer2D.h"

namespace Eunoia {

	EU_REFLECT(System)
	class EU_API GuiSystem : public ECSSystem
	{
	public:
		GuiSystem();
		virtual void ProcessEntityOnUpdate(EntityID entity, r32 dt) override;
		virtual void PostUpdate(r32 dt) override;
		virtual void PreRender() override;
		virtual void ProcessEntityOnRender(EntityID entity) override;
		virtual void PostRender() override;
	private:
		friend void DisplayResizeCallback(const DisplayEvent& e, void* userPtr);
	private:
		SpritePosOrigin m_PrevOrigin;
		m4 m_PrevOrtho;

		SpritePosOrigin m_GuiOrigin;
		m4 m_GuiOrtho;

		b32 m_PendingEventLeft;
		b32 m_PendingEventRight;

		EntityID m_MovingElement;
		MouseButton m_MovingElementButton;
		b32 m_MovingElementMoveX;
		b32 m_MovingElementMoveY;
		v2 m_MovingElementMin;
		v2 m_MovingElementMax;
	};

}