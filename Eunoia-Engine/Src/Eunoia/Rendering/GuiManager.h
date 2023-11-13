#pragma once

#include "../Math/Math.h"
#include "../DataStructures/List.h"
#include "../ECS/ECS.h"

namespace Eunoia {

	struct GuiLayout
	{
		r32 spacing;
		r32 padding;
	};

	enum ScrollBarMode
	{
		SCROLL_BAR_MODE_AS_NEEDED,
		SCROLL_BAR_MODE_ALWAYS,
		SCROLL_BAR_MODE_NEVER,

		NUM_SCROLL_BAR_MODES
	};

	class EU_API GuiManager
	{
	public:
		static void Init();
		static EntityID CreatePanel(ECS* ecs, const String& name, const v2& pos, const v2& size, const v4& color, r32 lineGap = 0.0f,
			r32 spacing = 0.0f, r32 padding = 0.0f, b32 titleBar = false, const v4& titleBarColor = v4(0.0f, 0.0f, 0.0f, 0.0f),
			const String& title = "", ScrollBarMode horizontal = SCROLL_BAR_MODE_NEVER, ScrollBarMode vertical = SCROLL_BAR_MODE_AS_NEEDED, b32 dispatchEventOnClick = false);

		static EntityID CreateButton(ECS* ecs, const String& name, EntityID panel, const v2& size, const v4& color, const String& text = "");
		static EntityID CreateCheckbox(ECS* ecs, const String& name, EntityID panel, const v2& size, const v4& outlineColor, b32 checked = false);
		static EntityID CreateSlider(ECS* ecs, const String& name, EntityID panel, r32 length, const v4& barColor, const v4& sliderColor); 
		static EntityID CreateLabel(ECS* ecs, const String& name, EntityID panel, const String& text, const v4& color);
		static EntityID CreateTreePanel(ECS* ecs, const String& name, EntityID panel, const String& text, const v4& color);

		static b32 IsCheckboxChecked(ECS* ecs, EntityID checkBox);

		static void AddSpace(EntityID panel, r32 space);
		static void NextLine(EntityID panel);
	private:
		static void RecalcScrollBarSliderSizeAndAddSpacing(ECS* ecs, EntityID panel, const v2& space);
		static void AdjustTransformToScrollBar(ECS* ecs, EntityID panel, Transform2D* transform);
	};

}
