#pragma once

#include "../ECS.h"

namespace Eunoia {

	enum GuiClickResponseFlags
	{
		GUI_CLICK_RESPONSE_FLAG_NONE = 0,

		GUI_CLICK_RESPONSE_FLAG_INTERNAL_PANEL_TITLEBAR = 1,
		GUI_CLICK_RESPONSE_FLAG_INTERNAL_PANEL_COLLAPSE = 2,
		GUI_CLICK_RESPONSE_FLAG_INTERNAL_PANEL_CLOSE = 4,

		GUI_CLICK_RESPONSE_FLAG_INTERNAL_CHECKBOX = 8,
		GUI_CLICK_RESPONSE_FLAG_INTERNAL_SLIDER_KNOB = 16,
		GUI_CLICK_RESPONSE_FLAG_INTERNAL_VSCROLLBAR_SLIDER = 32,
	};

	EU_REFLECT(Component)
	struct GuiClickResponseComponent : public ECSComponent
	{
		GuiClickResponseComponent(u32 flags) :
			flags(flags)
		{}

		GuiClickResponseComponent()
		{}

		u32 flags;
	};

}