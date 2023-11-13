#include "GuiManager.h"
#include "../ECS/Components/Transform2DComponent.h"
#include "../ECS/Components/GuiComponent.h"
#include "../ECS/Components/GuiClickResponseComponent.h"
#include "../ECS/Components/Text2DComponent.h"
#include "../DataStructures/Map.h"
#include "../Core/Engine.h"
#include "../Rendering/Renderer2D.h"

namespace Eunoia {

	struct PanelData
	{
		r32 currentLineX;
		r32 currentLineHeight;
		r32 currentY;
		r32 lineGap;
		r32 spacing;
		r32 padding;
		ScrollBarMode horizontal;
		ScrollBarMode vertical;
	};

	struct GuiManagerData
	{
		Map<EntityID, PanelData> panelData;
	};

	static GuiManagerData s_Data;

	void GuiManager::Init()
	{
		
	}

	EntityID GuiManager::CreatePanel(ECS* ecs, const String& name, const v2& pos, const v2& size, const v4& color, r32 lineGap, r32 spacing, r32 padding,
		b32 titleBar, const v4& titleBarColor, const String& title, ScrollBarMode horizontal, ScrollBarMode vertical, b32 dispatchEventOnClick)
	{
		EntityID panel = ecs->CreateEntity(name);
		ecs->CreateComponent<Transform2DComponent>(panel, Transform2D(pos, v2(1.0f, 1.0f), 0.0f));
		ecs->CreateComponent<GuiComponent>(panel, GuiComponent(size, color, EU_ECS_INVALID_ENTITY_ID, false));

		PanelData data;
		data.currentLineX = padding;
		data.currentLineHeight = 0.0f;
		data.currentY = 0.0f;
		data.lineGap = lineGap;
		data.spacing = spacing;
		data.padding = padding;
		data.horizontal = horizontal;
		data.vertical = vertical;

		const r32 titleBarHeight = 22.0f;

		if (titleBar)
		{
			Transform2D t;
			t.pos = v2(pos.x, pos.y - 0.1f);

			EntityID titleBar = ecs->CreateEntity(name + "_TitleBar", panel);
			ecs->CreateComponent<Transform2DComponent>(titleBar);
			ecs->CreateComponent<GuiComponent>(titleBar, GuiComponent(v2(size.x, titleBarHeight), titleBarColor, panel, false));
			ecs->CreateComponent<GuiClickResponseComponent>(titleBar, GUI_CLICK_RESPONSE_FLAG_INTERNAL_PANEL_TITLEBAR);

			EntityID titleBarCollapse = ecs->CreateEntity(titleBar + "_Collapse", titleBar);
			ecs->CreateComponent<Transform2DComponent>(titleBarCollapse);
			ecs->CreateComponent<GuiComponent>(titleBarCollapse, GuiComponent(v2(titleBarHeight + 10, titleBarHeight), v4(0.1f, 0.94f, 0.34f, 1.0f), panel, false));
			ecs->CreateComponent<GuiClickResponseComponent>(titleBarCollapse, GUI_CLICK_RESPONSE_FLAG_INTERNAL_PANEL_COLLAPSE);

			Transform2D titleBarCloseTransform(v2(size.x - (titleBarHeight + 10), 0.0f), v2(1.0f, 1.0f), 0.0f);

			EntityID titleBarClose = ecs->CreateEntity(titleBar + "_Close", titleBar);
			ecs->CreateComponent<Transform2DComponent>(titleBarClose, titleBarCloseTransform);
			ecs->CreateComponent<GuiComponent>(titleBarClose, GuiComponent(v2(titleBarHeight + 10, titleBarHeight), v4(1.0f, 0.0f, 0.0f, 1.0f), panel, false));
			ecs->CreateComponent<GuiClickResponseComponent>(titleBarClose, GUI_CLICK_RESPONSE_FLAG_INTERNAL_PANEL_CLOSE);

			data.currentY = titleBarHeight + lineGap;
		}

		r32 scrollBarStartY = titleBar ? titleBarHeight : 0.0f;

		if (vertical == SCROLL_BAR_MODE_ALWAYS)
		{
			const r32 scrollBarWidth = 15;
			Transform2D scrollTransform;
			scrollTransform.pos = v2(size.x - scrollBarWidth, scrollBarStartY);

			r32 scrollBarHeight = size.y - scrollBarStartY;

			EntityID scrollBar = ecs->CreateEntity(name + "_vScrollBar", panel);
			ecs->CreateComponent<Transform2DComponent>(scrollBar, scrollTransform);
			ecs->CreateComponent<GuiComponent>(scrollBar, GuiComponent(v2(scrollBarWidth, scrollBarHeight), v4(0.18f, 0.18f, 0.18f, 1.0f), panel, false));

			EntityID scrollBarSlider = ecs->CreateEntity(name + "_vScrollBar_Slider", scrollBar);
			
			r32 scrollBarButtonHeight = 10;

			ecs->CreateComponent<Transform2DComponent>(scrollBarSlider, Transform2D(v2(3, scrollBarButtonHeight), v2(1.0f, 1.0f), 0.0f));
			ecs->CreateComponent<GuiComponent>(scrollBarSlider, GuiComponent(v2(scrollBarWidth - 6, scrollBarHeight - scrollBarButtonHeight * 2), v4(0.8f, 0.8f, 0.8f, 1.0f), panel, false));
			ecs->CreateComponent<GuiClickResponseComponent>(scrollBarSlider, GUI_CLICK_RESPONSE_FLAG_INTERNAL_VSCROLLBAR_SLIDER);
		}

		if (horizontal == SCROLL_BAR_MODE_ALWAYS)
		{
			
		}

		s_Data.panelData[panel] = data;

		return panel;
	}

	EntityID GuiManager::CreateButton(ECS* ecs, const String& name, EntityID panel, const v2& size, const v4& color, const String& text)
	{
		PanelData* panelData = &s_Data.panelData[panel];

		Transform2D buttonTransform(v2(panelData->currentLineX, panelData->currentY), v2(1.0f, 1.0f), 0.0f);

		EntityID button = ecs->CreateEntity(name, panel);
		ecs->CreateComponent<Transform2DComponent>(button, buttonTransform);
		ecs->CreateComponent<GuiComponent>(button, GuiComponent(size, color, panel, true, text));
		ecs->CreateComponent<GuiClickResponseComponent>(button, GuiClickResponseComponent(GUI_CLICK_RESPONSE_FLAG_NONE));

		RecalcScrollBarSliderSizeAndAddSpacing(ecs, panel, size);

		return button;
	}

	EntityID GuiManager::CreateCheckbox(ECS* ecs, const String& name, EntityID panel, const v2& size, const v4& outlineColor, b32 checked)
	{
		PanelData* panelData = &s_Data.panelData[panel];

		Transform2D transform(v2(panelData->currentLineX, panelData->currentY), v2(1.0f, 1.0f), 0.0f);

		EntityID checkBox = ecs->CreateEntity(name, panel);
		ecs->CreateComponent<Transform2DComponent>(checkBox, transform);
		ecs->CreateComponent<GuiComponent>(checkBox, GuiComponent(size, outlineColor, panel));
		ecs->CreateComponent<GuiClickResponseComponent>(checkBox, GUI_CLICK_RESPONSE_FLAG_INTERNAL_CHECKBOX);
		
		r32 fillerAdjustment = 1;
		Transform2D fillerTransform(v2(fillerAdjustment, fillerAdjustment), v2(1.0f, 1.0f), 0.0f);

		const v4& fillerColor = ecs->GetComponent<GuiComponent>(panel)->color;

		EntityID checkBoxFiller = ecs->CreateEntity(name + "_Filler", checkBox);
		ecs->CreateComponent<Transform2DComponent>(checkBoxFiller, fillerTransform);
		ecs->CreateComponent<GuiComponent>(checkBoxFiller, GuiComponent(v2(size.x - fillerAdjustment * 2, size.y - fillerAdjustment * 2), fillerColor, panel));
		
		Transform2D checkTransform(v2(size.x / 4.0f, size.y / 4.0f), v2(1.0f, 1.0f), 0.0f);

		EntityID checkBoxCheck = ecs->CreateEntity(name + "_Check", checkBox);
		ecs->CreateComponent<Transform2DComponent>(checkBoxCheck, checkTransform);
		ecs->CreateComponent<GuiComponent>(checkBoxCheck, GuiComponent(v2(size.x / 2.0f, size.y / 2.0f), v4(0.0f, 1.0f, 0.0f, 1.0f), panel));

		ecs->SetEntityEnabled(checkBoxCheck, checked);

		RecalcScrollBarSliderSizeAndAddSpacing(ecs, panel, size);

		return checkBox;
	}

	EntityID GuiManager::CreateSlider(ECS* ecs, const String& name, EntityID panel, r32 length, const v4& barColor, const v4& sliderColor)
	{
		PanelData* panelData = &s_Data.panelData[panel];
		Transform2D transform(v2(panelData->currentLineX, panelData->currentY), v2(1.0f, 1.0f), 0.0f);

		const r32 barHeight = 6.0f;

		EntityID bar = ecs->CreateEntity(name, panel);
		ecs->CreateComponent<Transform2DComponent>(bar, transform);
		ecs->CreateComponent<GuiComponent>(bar, GuiComponent(v2(length, barHeight), barColor, panel));

		const v2 sliderTabSize(barHeight * 1.3f, barHeight * 2.1f);

		Transform2D sliderTransform(v2((length - sliderTabSize.x) / 2.0f, -(sliderTabSize.y - barHeight) / 2.0f), v2(1.0f, 1.0f), 0.0f);
		
		EntityID slider = ecs->CreateEntity(name + "_Slider", bar);
		ecs->CreateComponent<Transform2DComponent>(slider, sliderTransform);
		ecs->CreateComponent<GuiComponent>(slider, GuiComponent(sliderTabSize, sliderColor, panel));
		ecs->CreateComponent<GuiClickResponseComponent>(slider, GUI_CLICK_RESPONSE_FLAG_INTERNAL_SLIDER_KNOB);

		RecalcScrollBarSliderSizeAndAddSpacing(ecs, panel, v2(length, barHeight)); //TODO: Fix height

		return bar;
	}

	EntityID GuiManager::CreateLabel(ECS* ecs, const String& name, EntityID panel, const String& text, const v4& color)
	{
		PanelData* panelData = &s_Data.panelData[panel];

		r32 textScale = 0.5f;
		Transform2D labelTransform(v2(panelData->currentLineX, panelData->currentY), v2(textScale, textScale), 0.0f);

		EntityID label = ecs->CreateEntity(name, panel);
		ecs->CreateComponent<Transform2DComponent>(label, labelTransform);
		ecs->CreateComponent<GuiComponent>(label, GuiComponent(v2(0.0f, 0.0f), v4(0.0f, 0.0f, 0.0f, 1.0f), panel, true, text));

		Renderer2D* renderer = Engine::GetRenderer()->GetRenderer2D();
		RecalcScrollBarSliderSizeAndAddSpacing(ecs, panel, v2(renderer->GetLineWidth(text, textScale), renderer->GetLineHeight(text, textScale)));

		return label;
	}

	EntityID GuiManager::CreateTreePanel(ECS* ecs, const String& name, EntityID panel, const String& text, const v4& color)
	{
		PanelData* panelData = &s_Data.panelData[panel];
		Transform2D transform(v2(panelData->currentLineX, panelData->currentY), v2(1.0f, 1.0f), 0.0f);

		EntityID tree = ecs->CreateEntity(panel);
		ecs->CreateComponent<Transform2DComponent>(tree, transform);

		return tree;
	}

	b32 GuiManager::IsCheckboxChecked(ECS* ecs, EntityID checkBox)
	{
		const String& name = ecs->GetEntityName(checkBox);
		EntityID checkEntity = ecs->GetChildEntity(checkBox, name + "_Check");
		return ecs->IsEntityEnabled(checkEntity);
	}

	void GuiManager::AddSpace(EntityID panel, r32 space)
	{
		s_Data.panelData[panel].currentLineX += space;
	}

	void GuiManager::NextLine(EntityID panel)
	{
		PanelData* data = &s_Data.panelData[panel];
		data->currentLineX = data->padding;
		data->currentY += data->currentLineHeight + data->lineGap;
		data->currentLineHeight = data->lineGap;
	}

	void GuiManager::RecalcScrollBarSliderSizeAndAddSpacing(ECS* ecs, EntityID panel, const v2& space)
	{
		PanelData* panelData = &s_Data.panelData[panel];

		const v2& panelSize = ecs->GetComponent<GuiComponent>(panel)->size;
		EntityID vScrollBar = ecs->GetChildEntity(panel, ecs->GetEntityName(panel) + "_vScrollBar");
		EntityID hScrollBar = ecs->GetChildEntity(panel, ecs->GetEntityName(panel) + "_hScrollBar");

		if(vScrollBar != EU_ECS_INVALID_ENTITY_ID)
		{
			EntityID vScrollBarSlider = ecs->GetChildEntity(vScrollBar, ecs->GetEntityName(vScrollBar) + "_Slider");
			r32 contentHeight = panelData->currentY + panelData->currentLineHeight + panelData->lineGap;
			r32 panelHeight = panelSize.y;

			r32 scrollBarHeightModifier = contentHeight > panelHeight ? panelHeight / contentHeight : 1.0f;
			r32 scrollBarSliderHeight = ecs->GetComponent<GuiComponent>(vScrollBar)->size.y* scrollBarHeightModifier;
			ecs->GetComponent<GuiComponent>(vScrollBarSlider)->size.y = scrollBarSliderHeight - 20;
		}
		if(hScrollBar != EU_ECS_INVALID_ENTITY_ID)
		{
		
		}

		panelData->currentLineX += space.x + panelData->spacing;
		panelData->currentLineHeight = EU_MAX(panelData->currentLineHeight, space.y);
	}

	void GuiManager::AdjustTransformToScrollBar(ECS* ecs, EntityID panel, Transform2D* transform)
	{
		EntityID vScrollBar = ecs->GetChildEntity(panel, ecs->GetEntityName(panel) + "_vScrollBar");
		EntityID hScrollBar = ecs->GetChildEntity(panel, ecs->GetEntityName(panel) + "_hScrollBar");

		v2 offset;
		if (vScrollBar != EU_ECS_INVALID_ENTITY_ID)
		{
			EntityID vScrollBarSlider = ecs->GetChildEntity(vScrollBar, ecs->GetEntityName(vScrollBar) + "_Slider");

			r32 sliderPos = ecs->GetComponent<Transform2DComponent>(vScrollBarSlider)->localTransform.pos.y;
			offset.x = 0.0f;
			offset.y = sliderPos;

			r32 scrollBarHeight = ecs->GetComponent<GuiComponent>(vScrollBar)->size.y;
			r32 vScrollBarLocalPos = ecs->GetComponent<Transform2DComponent>(vScrollBarSlider)->localTransform.pos.y;



			r32 vScrollBarEndPos = vScrollBarLocalPos + ecs->GetComponent<GuiComponent>(vScrollBarSlider)->size.y;
		}
		if (hScrollBar != EU_ECS_INVALID_ENTITY_ID)
		{
			EntityID hScrollBarSlider = ecs->GetChildEntity(hScrollBar, ecs->GetEntityName(hScrollBar) + "_Slider");
		}

		transform->Translate(offset);
	}
}
