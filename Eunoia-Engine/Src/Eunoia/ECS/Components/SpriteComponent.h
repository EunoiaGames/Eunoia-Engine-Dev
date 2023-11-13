#pragma once

#include "../ECS.h"
#include "../../Rendering/Renderer2D.h"

namespace Eunoia {

	EU_REFLECT(Component)
	struct SpriteComponent : public ECSComponent
	{
		SpriteComponent(const v2& size, const v4& color, const SpriteSheet& spriteSheet, const v2& texturePos, const v2& offset = v2(0.0f, 0.0f)) :
			size(size),
			color(color),
			spriteSheet(spriteSheet),
			texturePos(texturePos),
			offset(offset)
		{}

		SpriteComponent(const v2& size, const v4& color, TextureID texture = EU_INVALID_TEXTURE_ID, const v2& offset = v2(0.0f, 0.0f)) :
			size(size),
			color(color),
			spriteSheet(texture, 1, 1),
			texturePos(0, 0),
			offset(offset)
		{}

		SpriteComponent() :
			size(0.0f, 0.0f),
			color(0.0f, 0.0f, 0.0f, 1.0f),
			spriteSheet(EU_INVALID_TEXTURE_ID, 0, 0),
			texturePos(0, 0),
			offset(0.0f, 0.0f)
		{}

		EU_PROPERTY() v2 size;
		EU_PROPERTY() v4 color;
		EU_PROPERTY() SpriteSheet spriteSheet;
		EU_PROPERTY() v2 texturePos;
		EU_PROPERTY() v2 offset;
	};

	EU_REFLECT(Component)
	struct SpriteGroupComponent : public ECSComponent
	{
		SpriteGroupComponent(const List<Sprite>& sprites) :
			sprites(sprites)
		{}

		SpriteGroupComponent()
		{}

		List<Sprite> sprites;
	};
}