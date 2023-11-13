#include "GuiSystem.h"
#include "../Components/GuiComponent.h"
#include "../Components/GuiClickResponseComponent.h"
#include "../Components/Transform2DComponent.h"
#include "../../Core/Input.h"
#include "../Events/GuiElementOnClickEvent.h"
#include "../../Core/Engine.h"

namespace Eunoia
{
	void Eunoia::DisplayResizeCallback(const DisplayEvent& e, void* userPtr)
	{
		if (e.type == DISPLAY_EVENT_RESIZE)
		{
			GuiSystem* guiSystem = (GuiSystem*)userPtr;
			guiSystem->m_GuiOrtho = m4::CreateOrthographic(0, e.width, 0.0, e.height, 0.0f, 1.0f);
		}
	}

	GuiSystem::GuiSystem()
	{
		AddComponentType<GuiComponent>();
		AddComponentType<Transform2DComponent>();

		u32 width = Engine::GetDisplay()->GetWidth();
		u32 height = Engine::GetDisplay()->GetHeigth();

		m_GuiOrigin = SPRITE_POS_ORIGIN_TOP_LEFT;
		m_GuiOrtho = m4::CreateOrthographic(0, width, 0, height, 0.0f, 1.0f);

		m_PendingEventLeft = false;
		m_PendingEventRight = false;

		m_MovingElement = EU_ECS_INVALID_ENTITY_ID;
		m_MovingElementButton = (MouseButton)0;

		Engine::GetDisplay()->AddDisplayEventCallback(DisplayResizeCallback, this);
	}

	void GuiSystem::ProcessEntityOnUpdate(EntityID entity, r32 dt)
	{
		GuiComponent* guiComponent = m_ECS->GetComponent<GuiComponent>(entity);
		const Transform2D& transform = m_ECS->GetComponent<Transform2DComponent>(entity)->worldTransform;

		if (guiComponent->affectedByScroll && guiComponent->panel != EU_ECS_INVALID_ENTITY_ID)
		{
			v2 panelPos = m_ECS->GetComponent<Transform2DComponent>(guiComponent->panel)->worldTransform.pos;
			r32 panelHeight = m_ECS->GetComponent<GuiComponent>(guiComponent->panel)->size.y;
			EntityID vScrollBar = m_ECS->GetChildEntity(guiComponent->panel, m_ECS->GetEntityName(guiComponent->panel) + "_vScrollBar");
			EntityID hScrollBar = m_ECS->GetChildEntity(guiComponent->panel, m_ECS->GetEntityName(guiComponent->panel) + "_vScrollBar");

			if (vScrollBar != EU_ECS_INVALID_ENTITY_ID)
			{
				EntityID vScrollBarSlider = m_ECS->GetChildEntity(vScrollBar, m_ECS->GetEntityName(vScrollBar) + "_Slider");
				r32 localY = m_ECS->GetComponent<Transform2DComponent>(vScrollBarSlider)->localTransform.pos.y - 10;
				
				b32 isAbove = (transform.pos.y - localY) < (panelPos.y + 22);
				b32 isBelow = (transform.pos.y + guiComponent->size.y - localY) > (panelPos.y + panelHeight);

				if (!isAbove)
					guiComponent->offset.y = m_ECS->GetComponent<Transform2DComponent>(vScrollBarSlider)->localTransform.pos.y - 10;
			}
		}

		GuiClickResponseComponent* element = m_ECS->GetComponent<GuiClickResponseComponent>(entity);
		if (!element)
			return;
		b32 mouseClicked = false;
		MouseButton buttonClicked;
		if (EUInput::IsButtonPressed(EU_BUTTON_LEFT))
		{
			mouseClicked = true;
			buttonClicked = EU_BUTTON_LEFT;
		}
		if (EUInput::IsButtonPressed(EU_BUTTON_RIGHT))
		{
			mouseClicked = true;
			buttonClicked = EU_BUTTON_RIGHT;
		}

		if (m_MovingElement != EU_ECS_INVALID_ENTITY_ID)
		{
			if (EUInput::IsButtonRelease(m_MovingElementButton))
				m_MovingElement = EU_ECS_INVALID_ENTITY_ID;
		}

		if (mouseClicked)
		{
			v2 mousePos = Engine::GetDisplay()->GetMousePos();
			//mousePos.y = Engine::GetDisplay()->GetHeigth() - mousePos.y;
			//v2 coords = GuiManager::WindowCoordsToGuiCoords(mousePos);

			const v2 elementPos = m_ECS->GetComponent<Transform2DComponent>(entity)->worldTransform.pos;
			const v2 elementSize = m_ECS->GetComponent<GuiComponent>(entity)->size;

			if (mousePos.x > elementPos.x && mousePos.x < elementPos.x + elementSize.x &&
				mousePos.y > elementPos.y && mousePos.y < elementPos.y + elementSize.y)
			{
				if ((element->flags & GUI_CLICK_RESPONSE_FLAG_INTERNAL_PANEL_TITLEBAR) == GUI_CLICK_RESPONSE_FLAG_INTERNAL_PANEL_TITLEBAR && buttonClicked == EU_BUTTON_LEFT)
				{
					m_MovingElement = m_ECS->GetParentEntity(entity);
					const v2& panelSize = m_ECS->GetComponent<GuiComponent>(m_MovingElement)->size;
					m_MovingElementButton = EU_BUTTON_LEFT;
					m_MovingElementMoveX = m_MovingElementMoveY = true;
					m_MovingElementMin = v2(0.0f, 0.0f);
					m_MovingElementMax = v2(Engine::GetDisplay()->GetWidth() - panelSize.x, Engine::GetDisplay()->GetHeigth() - panelSize.y);
				}
				else if ((element->flags & GUI_CLICK_RESPONSE_FLAG_INTERNAL_PANEL_COLLAPSE) == GUI_CLICK_RESPONSE_FLAG_INTERNAL_PANEL_COLLAPSE && buttonClicked == EU_BUTTON_LEFT)
				{
					EntityID titleBarEntity = m_ECS->GetParentEntity(entity);
					EntityID panelEntity = m_ECS->GetParentEntity(titleBarEntity);
					m_ECS->SetComponentEnabledOpposite<GuiComponent>(panelEntity);
				}
				else if ((element->flags & GUI_CLICK_RESPONSE_FLAG_INTERNAL_PANEL_CLOSE) == GUI_CLICK_RESPONSE_FLAG_INTERNAL_PANEL_CLOSE)
				{
					EntityID titleBarEntity = m_ECS->GetParentEntity(entity);
					EntityID panelEntity = m_ECS->GetParentEntity(titleBarEntity);
					m_ECS->SetEntityEnabled(panelEntity, false);
				}
				else if ((element->flags & GUI_CLICK_RESPONSE_FLAG_INTERNAL_CHECKBOX) == GUI_CLICK_RESPONSE_FLAG_INTERNAL_CHECKBOX)
				{
					const String& name = m_ECS->GetEntityName(entity);
					EntityID checkEntity = m_ECS->GetChildEntity(entity, name + "_Check");
					m_ECS->SetEntityEnabledOpposite(checkEntity);
					m_ECS->DispatchEvent<GuiElementOnClickEvent>(entity, EU_BUTTON_LEFT, mousePos, m_ECS->IsEntityEnabled(checkEntity));
				}
				else if ((element->flags & GUI_CLICK_RESPONSE_FLAG_INTERNAL_SLIDER_KNOB) == GUI_CLICK_RESPONSE_FLAG_INTERNAL_SLIDER_KNOB)
				{
					EntityID barEntity = m_ECS->GetParentEntity(entity);
					r32 knobWidth = m_ECS->GetComponent<GuiComponent>(entity)->size.x;
					const Transform2D& barTransform = m_ECS->GetComponent<Transform2DComponent>(barEntity)->worldTransform;
					r32 length = m_ECS->GetComponent<GuiComponent>(barEntity)->size.x;

					m_MovingElement = entity;
					m_MovingElementButton = EU_BUTTON_LEFT;
					m_MovingElementMoveX = true;
					m_MovingElementMoveY = false;
					m_MovingElementMin = v2(0.0, 0.0f);
					m_MovingElementMax = v2(length - knobWidth, 0.0f);
				}
				else if ((element->flags & GUI_CLICK_RESPONSE_FLAG_INTERNAL_VSCROLLBAR_SLIDER) == GUI_CLICK_RESPONSE_FLAG_INTERNAL_VSCROLLBAR_SLIDER)
				{
					EntityID scrollBarSlider = entity;
					EntityID scrollBar = m_ECS->GetParentEntity(scrollBarSlider);

					r32 scrollBarSliderHeight = m_ECS->GetComponent<GuiComponent>(scrollBarSlider)->size.y;
					r32 scrollBarHeight = m_ECS->GetComponent<GuiComponent>(scrollBar)->size.y;

					m_MovingElement = scrollBarSlider;
					m_MovingElementButton = EU_BUTTON_LEFT;
					m_MovingElementMoveX = false;
					m_MovingElementMoveY = true;
					m_MovingElementMin = v2(0.0f, 10.0f);
					m_MovingElementMax = v2(0.0f, scrollBarHeight - scrollBarSliderHeight - 20);
				}
				else
				{
					m_ECS->DispatchEvent<GuiElementOnClickEvent>(GuiElementOnClickEvent(entity, buttonClicked, mousePos, m_ECS->IsEntityEnabled(entity)));
				}
			}
		}
	}

	void GuiSystem::PostUpdate(r32 dt)
	{
		if (m_MovingElement != EU_ECS_INVALID_ENTITY_ID)
		{
			Transform2D* transform = &m_ECS->GetComponent<Transform2DComponent>(m_MovingElement)->localTransform;
			v2 moveAmount = Engine::GetDisplay()->GetMouseDeltaPos();
			if (m_MovingElementMoveX && transform->pos.x >= m_MovingElementMin.x && transform->pos.x <= m_MovingElementMax.x)
			{
				transform->pos.x += moveAmount.x;
				transform->pos.x = EU_CLAMP(m_MovingElementMin.x, m_MovingElementMax.x, transform->pos.x);
			}
			if (m_MovingElementMoveY && transform->pos.y >= m_MovingElementMin.y && transform->pos.y <= m_MovingElementMax.y) 
			{
				transform->pos.y += moveAmount.y;
				transform->pos.y = EU_CLAMP(m_MovingElementMin.y, m_MovingElementMax.y, transform->pos.y);
			}
		}
	}

	void GuiSystem::PreRender()
	{
		Renderer2D* renderer = Engine::GetRenderer()->GetRenderer2D();

		m_PrevOrigin = renderer->GetSpritePosOrigin();
		m_PrevOrtho = renderer->GetOrthographic();

		renderer->SetSpritePosOrigin(m_GuiOrigin);
		renderer->SetProjection(m_GuiOrtho);
	}

	void GuiSystem::ProcessEntityOnRender(EntityID entity)
	{
		GuiComponent* guiComponent = m_ECS->GetComponent<GuiComponent>(entity);
		const Transform2D& transform = m_ECS->GetComponent<Transform2DComponent>(entity)->worldTransform;

		Renderer2D* renderer = Engine::GetRenderer()->GetRenderer2D();
		if (guiComponent->size.x != 0.0f && guiComponent->size.y != 0.0f)
		{
			Sprite sprite;
			sprite.size = guiComponent->size;

			if (guiComponent->panel != EU_ECS_INVALID_ENTITY_ID)
			{
				v2 panelPos = m_ECS->GetComponent<Transform2DComponent>(guiComponent->panel)->worldTransform.pos;
				v2 elementPos = (transform.pos - guiComponent->offset);
				v2 panelSize = m_ECS->GetComponent<GuiComponent>(guiComponent->panel)->size;
				v2 amountOffscreenPositive = (elementPos + guiComponent->size) - (panelPos + panelSize);

				if (amountOffscreenPositive.x > 0.0f)
				{
					sprite.size.x = guiComponent->size.x - amountOffscreenPositive.x;
				}
				

				if (amountOffscreenPositive.y > 0.0f)
				{
					sprite.size.y = guiComponent->size.y - amountOffscreenPositive.y;
				}
			}

			sprite.pos = v3(transform.pos - guiComponent->offset, 0.0f);
			sprite.occluder = false;
			sprite.color = guiComponent->color;
			sprite.rot = 0.0f;
			sprite.spriteSheet.texture = EU_INVALID_TEXTURE_ID;
			sprite.texturePos = v2(0, 0);

			renderer->SubmitSprite(sprite);
		}
		if (!guiComponent->text.Empty())
		{
			//renderer->SubmitText(guiComponent->text, v3(transform.pos, 0.0f), v4(1.0f, 0.0f, 0.0f, 1.0f), 0.5f);
		}
	}

	void GuiSystem::PostRender()
	{
		Renderer2D* renderer = Engine::GetRenderer()->GetRenderer2D();
		renderer->SetSpritePosOrigin(m_PrevOrigin);
		//renderer->SetProjection(m_PrevOrtho);
	}
}
