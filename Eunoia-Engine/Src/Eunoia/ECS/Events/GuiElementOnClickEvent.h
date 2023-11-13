#pragma once

#include "../ECS.h"
#include "../../Core/InputDefs.h"

namespace Eunoia {

	EU_REFLECT(Event)
	struct GuiElementOnClickEvent : public ECSEvent
	{
		GuiElementOnClickEvent(EntityID element, MouseButton button, const v2& pos, b32 enabled = true) :
			elementClicked(element),
			button(button),
			pos(pos),
			enabled(enabled)
		{}

		GuiElementOnClickEvent()
		{}

		EU_PROPERTY() EntityID elementClicked;
		EU_PROPERTY() MouseButton button;
		EU_PROPERTY() v2 pos;
		EU_PROPERTY() b32 enabled;
	};

}