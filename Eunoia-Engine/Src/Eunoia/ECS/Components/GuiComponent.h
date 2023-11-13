#pragma once

#include "../ECS.h"
#include "../../Math/Math.h"

namespace Eunoia {

	EU_REFLECT(Component)
	struct GuiComponent : public ECSComponent
	{
		GuiComponent(const v2& size, const v4& color, EntityID panel, b32 affectedByScroll = true, const String& text = "", const v2& offset = v2(0.0f, 0.0f)) :
			size(size),
			color(color),
			panel(panel),
			text(text),
			offset(offset),
			affectedByScroll(affectedByScroll)
		{}

		GuiComponent() :
			affectedByScroll(true)
		{}

		EU_PROPERTY() v2 size;
		EU_PROPERTY() v4 color;
		EU_PROPERTY() String text;
		EU_PROPERTY() EntityID panel;
		EU_PROPERTY() v2 offset;
		EU_PROPERTY(Bool32) b32 affectedByScroll;
	};

}