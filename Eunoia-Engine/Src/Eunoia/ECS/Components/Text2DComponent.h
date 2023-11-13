#pragma once

#include "../ECS.h"

namespace Eunoia {

	EU_REFLECT(Component)
	struct Text2DComponent : public ECSComponent
	{
		Text2DComponent(const String& text, const v4& color) :
			text(text),
			color(color)
		{}

		Text2DComponent()
		{}

		String text;
		v4 color;
	};

}