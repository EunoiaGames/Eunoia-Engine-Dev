#pragma once

#include "RenderContext.h"
#include "../Math/Math.h"
#include <freetype\freetype.h>

#define EU_RENDERER2D_DEFAULT_MAX_SPRITE_COUNT 10000
#define EU_RENDERER2D_MAX_LIGHT_COUNT 64
#define EU_RENDERER2D_ABSOLUTE_MAX_SPRITE_COUNT 50000
#define EU_RENDERER2D_TRANSFORM_STACK_LIMIT 64

#define EU_INVALID_RENDERER2D_OBJECT_ID 0
#define EU_INVALID_FONT_ID EU_INVALID_RENDERER2D_OBJECT_ID

#define EU_RENDERER2D_MAX_SPRITE_GROUPS 32

namespace Eunoia {

	typedef u32 Renderer2DObjectID;
	typedef Renderer2DObjectID FontID;

	EU_REFLECT()
	struct SpriteSheet
	{
		SpriteSheet(TextureID texture, u32 numRows, u32 numCols) :
			texture(texture),
			numRows(numRows),
			numCols(numCols)
		{}

		SpriteSheet() :
			texture(EU_INVALID_TEXTURE_ID),
			numRows(0),
			numCols(0)
		{}

		EU_PROPERTY() TextureID texture;
		EU_PROPERTY() u32 numRows;
		EU_PROPERTY() u32 numCols;
	};

	struct Sprite
	{
		v3 pos;
		v2 size;
		v4 color;
		SpriteSheet spriteSheet;
		v2 texturePos;
		r32 rot;
		b32 occluder;
	};

	struct Light
	{
		b32 castsShadows;
		v2 pos;
		r32 intensity;
		v3 color;
		v3 attenuation;
	};

	enum SpritePosOrigin
	{
		SPRITE_POS_ORIGIN_BOTTOM_LEFT,
		SPRITE_POS_ORIGIN_TOP_LEFT,
		SPRITE_POS_ORIGIN_TOP_RIGHT,
		SPRITE_POS_ORIGIN_BOTTOM_RIGHT,
		SPRITE_POS_ORIGIN_CENTER,

		NUM_SPRITE_POS_ORIGIN_TYPES,
		SPRITE_POS_ORIGIN_DEFAULT = SPRITE_POS_ORIGIN_BOTTOM_LEFT,
		SPRITE_POS_ORIGIN_FIRST = 0
	};

	struct Renderer2DVertex
	{
		v2 pos;
		v2 texCoord;
		v4 color;
		r32 textureIndex;
	};

	struct Renderer2DOccluderVertex
	{
		v2 pos;
	};

	struct SubmitedSprite
	{
		Renderer2DVertex vertices[4];
	};

	struct SubmitedOccluder
	{
		Renderer2DOccluderVertex vertices[4];
	};

	struct SpriteGroup
	{
		SpriteGroup() :
			numTextures(0),
			vertexOffset(0),
			indexCount(0)
		{}

		TextureID textures[EU_MAX_ARRAY_OF_TEXTURES_SIZE];
		List<SubmitedSprite> sprites;

		u32 indexCount;
		u32 numTextures;
		u32 vertexOffset;
	};

	struct FontCharacter
	{
		TextureID texture;
		v2 size;
		v2 bearing;
		u32 advance;
		r32 actualHeight;
	};

	struct Font
	{
		FT_Face face;
		List<FontCharacter> ascii;
		u32 lineHeight;
	};

	struct LightData
	{
		v2 pos;
		r32 intensity;
		r32 p0;
		v3 color;
		r32 p1;
		v3 atten;
		r32 p2;
	};

	struct LightBufferData
	{
		u32 numLights;
		v3 p0;
		LightData lights[EU_RENDERER2D_MAX_LIGHT_COUNT];
	};

	class EU_API Renderer2D
	{
	public:
		Renderer2D(RenderContext* renderContext, Display* display);
		~Renderer2D();

		TextureID Init(u32 maxSprites = EU_RENDERER2D_DEFAULT_MAX_SPRITE_COUNT);

		FontID LoadFont(const String& file, b32 setActive = true, u32 pixelFontSize = 48);
		void SetActiveFont(FontID font);
		u32 GetLineHeight(const String& text, r32 scale);
		u32 GetLineWidth(const String& text, r32 scale);

		void SetSpritePosOrigin(SpritePosOrigin origin);
		void SetCamera(const v2& pos, r32 rot);
		void SetProjection(const m4& projection);
		void SetOrthographic(r32 left, r32 right, r32 bottom, r32 top, r32 znear = 0.0f, r32 zfar = 1000.0f);

		SpritePosOrigin GetSpritePosOrigin();
		const m4& GetOrthographic();

		void PushTransformStack(const m3& transform);
		void PopTransformStack();

		TextureID GetOutputTexture();
		void ResizeOutput(u32 width, u32 height);

		void BeginFrame();
		void SubmitText(const String& text, v3 pos, v4 color, r32 scale);
		void SubmitLight(const Light& light);
		void SubmitSprite(const Sprite& sprite);
		void SubmitSprite(v3 pos, v2 size, v4 color, r32 rot = 0.0f, b32 occluder = false);
		void SubmitSprite(v3 pos, v2 size, SpriteSheet spriteSheet, v2 spritePos, v4 color = v4(1.0f, 1.0f, 1.0f, 1.0f), r32 rot = 0.0f, b32 occluder = false);
		void SubmitSprite(v3 pos, v2 size, TextureID texture, v4 color = v4(1.0f, 1.0f, 1.0f, 1.0f), r32 rot = 0.0f, b32 occluder = false);
		void EndFrame();
		void RenderFrame();
	private:
		friend void DisplayResizeCallback(const DisplayEvent& e, void* userPtr);
		void GetTexCoords(const SpriteSheet& spriteSheet, const v2& spritePos, v2* bl, v2* tl, v2* tr, v2* br);
	private:
		RenderContext* m_RenderContext;
		Display* m_Display;

		ShaderBufferID m_PerFrameUBO;
		ShaderBufferID m_LightsUBO;

		RenderPassID m_RenderPass;
		TextureID m_OutputTexture;

		BufferID m_VertexBuffer;
		BufferID m_OcclusionVertexBuffer;
		BufferID m_IndexBuffer;
		IndexType m_IndexType;

		SpriteGroup m_SpriteGroups[EU_RENDERER2D_MAX_SPRITE_GROUPS];
		u32 m_NumSpriteGroups;
		List<SubmitedSprite> m_ColoredSprites;
		List<SubmitedOccluder> m_Occluders;
		List<Light>	m_Lights;
		LightBufferData m_LightBufferData;

		SpritePosOrigin m_Origin;

		TextureID m_WhiteTexture;

		FT_Library m_FtLib;
		List<Font> m_Fonts;
		FontID m_ActiveFont;

		m4 m_View;
		m4 m_Projection;
		m4 m_ViewProjection;
		m3 m_TransformStack[EU_RENDERER2D_TRANSFORM_STACK_LIMIT];
		u32 m_TransformStackSize;
	};

}
