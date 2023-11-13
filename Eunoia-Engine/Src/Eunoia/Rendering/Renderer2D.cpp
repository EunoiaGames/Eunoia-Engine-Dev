#include "Renderer2D.h"
#include "../Core/Engine.h"
#include "../Utils/Log.h"
#include "../DataStructures/List.h"
#include "Asset/AssetTypeIDs.h"

namespace Eunoia {

	static void DisplayResizeCallback(const DisplayEvent& e, void* userPtr)
	{
		Renderer2D* renderer = (Renderer2D*)userPtr;

		if (e.type == DISPLAY_EVENT_RESIZE)
		{
			renderer->m_RenderContext->ResizeFramebuffer(renderer->m_RenderPass, e.width, e.height);
		}
	}

	Renderer2D::Renderer2D(RenderContext* renderContext, Display* display) :
		m_RenderContext(renderContext),
		m_Display(display)
	{}

	Renderer2D::~Renderer2D()
	{
	}

	TextureID Renderer2D::Init(u32 maxSprites)
	{
		if(!Engine::IsEditorAttached())
			m_Display->AddDisplayEventCallback(DisplayResizeCallback, this);

		
		m_Origin = SPRITE_POS_ORIGIN_DEFAULT;
		m_View = m4::CreateIdentity();
		m_Projection = m4::CreateIdentity();
		m_TransformStackSize = 0;

		ShaderID spriteMapShader = m_RenderContext->LoadShader("Batch2D");
		ShaderID occlusionMapShader = m_RenderContext->LoadShader("OcclusionMap");

		RenderPass renderPass;
		Framebuffer* framebuffer = &renderPass.framebuffer;

		framebuffer->useSwapchainSize = true;
		framebuffer->numAttachments = 2;
		framebuffer->attachments[0].format = TEXTURE_FORMAT_RGBA16_FLOAT;
		framebuffer->attachments[0].isClearAttachment = true;
		framebuffer->attachments[0].isSamplerAttachment = true;
		framebuffer->attachments[0].isStoreAttachment = true;
		framebuffer->attachments[0].isSubpassInputAttachment = false;
		framebuffer->attachments[0].isSwapchainAttachment = false;
		framebuffer->attachments[0].nonClearAttachmentPreserve = false;
		framebuffer->attachments[0].memoryTransferSrc = false;
		framebuffer->attachments[1].format = TEXTURE_FORMAT_R8_UNORM;
		framebuffer->attachments[1].isClearAttachment = true;
		framebuffer->attachments[1].isSamplerAttachment = false;
		framebuffer->attachments[1].isStoreAttachment = false;
		framebuffer->attachments[1].isSubpassInputAttachment = true;
		framebuffer->attachments[1].isSwapchainAttachment = false;
		framebuffer->attachments[1].nonClearAttachmentPreserve = false;
		framebuffer->attachments[1].memoryTransferSrc = false;

		Subpass spriteMapSubpass;
		spriteMapSubpass.depthStencilAttachment = 0;
		spriteMapSubpass.useDepthStencilAttachment = false;
		spriteMapSubpass.numReadAttachments = 0;
		spriteMapSubpass.numWriteAttachments = 1;
		spriteMapSubpass.writeAttachments[0] = 0;
		
		GraphicsPipeline spriteMapPipeline {};
		spriteMapPipeline.shader = spriteMapShader;
		spriteMapPipeline.topology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		spriteMapPipeline.viewportState.useFramebufferSizeForScissor = true;
		spriteMapPipeline.viewportState.useFramebufferSizeForViewport = true;
		spriteMapPipeline.viewportState.scissor.x =
		spriteMapPipeline.viewportState.scissor.y =
		spriteMapPipeline.viewportState.viewport.x =
		spriteMapPipeline.viewportState.viewport.y = 0;
		spriteMapPipeline.rasterizationState.cullMode = CULL_MODE_BACK;
		spriteMapPipeline.rasterizationState.depthClampEnabled = false;
		spriteMapPipeline.rasterizationState.discard = false;
		spriteMapPipeline.rasterizationState.frontFace = FRONT_FACE_CCW;
		spriteMapPipeline.rasterizationState.polygonMode = POLYGON_MODE_FILL;
		spriteMapPipeline.vertexInputState.numAttributes = 4;
		spriteMapPipeline.vertexInputState.vertexSize = EU_VERTEX_SIZE_AUTO;
		spriteMapPipeline.vertexInputState.attributes[0].name = "POSITION";
		spriteMapPipeline.vertexInputState.attributes[0].type = VERTEX_ATTRIBUTE_FLOAT2;
		spriteMapPipeline.vertexInputState.attributes[0].location = 0;
		spriteMapPipeline.vertexInputState.attributes[1].name = "TEXCOORD";
		spriteMapPipeline.vertexInputState.attributes[1].type = VERTEX_ATTRIBUTE_FLOAT2;
		spriteMapPipeline.vertexInputState.attributes[1].location = 1;
		spriteMapPipeline.vertexInputState.attributes[2].name = "COLOR";
		spriteMapPipeline.vertexInputState.attributes[2].type = VERTEX_ATTRIBUTE_FLOAT4;
		spriteMapPipeline.vertexInputState.attributes[2].location = 2;
		spriteMapPipeline.vertexInputState.attributes[3].name = "INDEX";
		spriteMapPipeline.vertexInputState.attributes[3].type = VERTEX_ATTRIBUTE_FLOAT;
		spriteMapPipeline.vertexInputState.attributes[3].location = 3;
		spriteMapPipeline.numBlendStates = 1;
		spriteMapPipeline.blendStates[0].blendEnabled = true;
		spriteMapPipeline.blendStates[0].alpha.dstFactor = BLEND_FACTOR_ONE;
		spriteMapPipeline.blendStates[0].alpha.srcFactor = BLEND_FACTOR_ONE;
		spriteMapPipeline.blendStates[0].alpha.operation = BLEND_OPERATION_MAX;
		spriteMapPipeline.blendStates[0].color.dstFactor = BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		spriteMapPipeline.blendStates[0].color.srcFactor = BLEND_FACTOR_SRC_ALPHA;
		spriteMapPipeline.blendStates[0].color.operation = BLEND_OPERATION_ADD;
		spriteMapPipeline.depthStencilState.depthCompare = COMPARE_OPERATION_LESS;
		spriteMapPipeline.depthStencilState.depthTestEnabled = false;
		spriteMapPipeline.depthStencilState.depthWriteEnabled = false;
		spriteMapPipeline.depthStencilState.stencilTestEnabled = false;

		MaxTextureGroupBinds maxBinds;
		maxBinds.set = 1;
		maxBinds.maxBinds = EU_RENDERER2D_MAX_SPRITE_GROUPS + 1;

		spriteMapPipeline.maxTextureGroupBinds.Push(maxBinds);

		spriteMapSubpass.pipelines.Push(spriteMapPipeline);

		/*Subpass occlusionMapSubpass;
		occlusionMapSubpass.depthStencilAttachment = 0;
		occlusionMapSubpass.useDepthStencilAttachment = false;
		occlusionMapSubpass.numReadAttachments = 0;
		occlusionMapSubpass.numWriteAttachments = 1;
		occlusionMapSubpass.writeAttachments[0] = 1;
		
		GraphicsPipeline occlusionMapPipeline {};
		occlusionMapPipeline.shader = occlusionMapShader;
		occlusionMapPipeline.topology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		occlusionMapPipeline.viewportState.useFramebufferSizeForScissor = true;
		occlusionMapPipeline.viewportState.useFramebufferSizeForViewport = true;
		occlusionMapPipeline.viewportState.scissor.x =
		occlusionMapPipeline.viewportState.scissor.y =
		occlusionMapPipeline.viewportState.viewport.x =
		occlusionMapPipeline.viewportState.viewport.y = 0;
		occlusionMapPipeline.rasterizationState.cullMode = CULL_MODE_BACK;
		occlusionMapPipeline.rasterizationState.depthClampEnabled = false;
		occlusionMapPipeline.rasterizationState.discard = false;
		occlusionMapPipeline.rasterizationState.frontFace = FRONT_FACE_CCW;
		occlusionMapPipeline.rasterizationState.polygonMode = POLYGON_MODE_FILL;
		occlusionMapPipeline.vertexInputState.numAttributes = 1;
		occlusionMapPipeline.vertexInputState.vertexSize = EU_VERTEX_SIZE_AUTO;
		occlusionMapPipeline.vertexInputState.attributes[0].name = "POSITION";
		occlusionMapPipeline.vertexInputState.attributes[0].type = VERTEX_ATTRIBUTE_FLOAT2;
		occlusionMapPipeline.vertexInputState.attributes[0].location = 0;
		occlusionMapPipeline.numBlendStates = 1;
		occlusionMapPipeline.blendStates[0].blendEnabled = false;
		occlusionMapPipeline.blendStates[0].alpha.dstFactor = BLEND_FACTOR_ZERO;
		occlusionMapPipeline.blendStates[0].alpha.srcFactor = BLEND_FACTOR_ONE;
		occlusionMapPipeline.blendStates[0].alpha.operation = BLEND_OPERATION_ADD;
		occlusionMapPipeline.blendStates[0].color = occlusionMapPipeline.blendStates[0].alpha;
		occlusionMapPipeline.depthStencilState.depthCompare = COMPARE_OPERATION_LESS;
		occlusionMapPipeline.depthStencilState.depthTestEnabled = false;
		occlusionMapPipeline.depthStencilState.depthWriteEnabled = false;
		occlusionMapPipeline.depthStencilState.stencilTestEnabled = false;

		occlusionMapSubpass.pipelines.Push(occlusionMapPipeline);*/

		renderPass.subpasses.Push(spriteMapSubpass);

		m_RenderPass = m_RenderContext->CreateRenderPass(renderPass);

		RenderPass occlusionMapPas;


		m_PerFrameUBO = m_RenderContext->CreateShaderBuffer(SHADER_BUFFER_UNIFORM_BUFFER, sizeof(m4), 1);
		m_RenderContext->AttachShaderBufferToRenderPass(m_RenderPass, m_PerFrameUBO, 0, 0, 0, 0);

		m_VertexBuffer = m_RenderContext->CreateBuffer(BUFFER_TYPE_VERTEX, BUFFER_USAGE_DYNAMIC, 0, sizeof(SubmitedSprite) * maxSprites);
		m_OcclusionVertexBuffer = m_RenderContext->CreateBuffer(BUFFER_TYPE_VERTEX, BUFFER_USAGE_DYNAMIC, 0, sizeof(SubmitedOccluder) * maxSprites);
		
		u32 numIndices = maxSprites * 6;

		if (numIndices < EU_U16_MAX)
		{
			m_IndexType = INDEX_TYPE_U16;
			u16* indices = (u16*)malloc(numIndices * sizeof(u16));

			u32 offset = 0;
			for (u32 i = 0; i < numIndices; i += 6)
			{
				indices[i + 0] = offset + 0;
				indices[i + 1] = offset + 2;
				indices[i + 2] = offset + 3;

				indices[i + 3] = offset + 0;
				indices[i + 4] = offset + 1;
				indices[i + 5] = offset + 2;

				offset += 4;
			}

			m_IndexBuffer = m_RenderContext->CreateBuffer(BUFFER_TYPE_INDEX, BUFFER_USAGE_STATIC, indices, sizeof(u16) * numIndices);
			free(indices);
		}
		else
		{
			m_IndexType = INDEX_TYPE_U32;
			u32* indices = (u32*)malloc(numIndices * sizeof(u32));

			u32 offset = 0;
			for (u32 i = 0; i < numIndices; i += 6)
			{
				indices[i + 0] = offset + 0;
				indices[i + 1] = offset + 1;
				indices[i + 2] = offset + 2;

				indices[i + 3] = offset + 0;
				indices[i + 4] = offset + 2;
				indices[i + 5] = offset + 3;

				offset += 4;
			}

			m_IndexBuffer = m_RenderContext->CreateBuffer(BUFFER_TYPE_INDEX, BUFFER_USAGE_STATIC, indices, sizeof(u32) * numIndices);
			free(indices);
		}

		u8 pixels[4] = { 255, 255, 255, 255 };
		m_WhiteTexture = m_RenderContext->CreateTexture2D(pixels, 1, 1, TEXTURE_FORMAT_RGBA8_UNORM);

		m_OutputTexture = m_RenderContext->CreateTextureHandleForFramebufferAttachment(m_RenderPass, 0);

		if (FT_Init_FreeType(&m_FtLib))
		{
			EU_LOG_WARN("Could not initialize FreeType library. Text is not supported");
			return m_OutputTexture;
		}

		return m_OutputTexture;
	}

	FontID Renderer2D::LoadFont(const String& file, b32 setActive, u32 pixelFontSize)
	{
		String f = "../Eunoia-Engine/" + file;

		Font font;
		if (FT_New_Face(m_FtLib, f.C_Str(), 0, &font.face))
		{
			String errMsg = "Unable to load ttf font \"" + file + "\"";
			EU_LOG_WARN(errMsg.C_Str());
			return EU_INVALID_FONT_ID;
		}

		FT_Set_Pixel_Sizes(font.face, 0, pixelFontSize);

		font.ascii.SetCapacityAndElementCount(128);
		
		u32 fontAtlasWidth = 0;
		u32 fontAtlasHeight = 0;

		List<u8> pixels;
		font.lineHeight = 32;
		for (u32 c = 0; c < 128; c++)
		{
			if (FT_Load_Char(font.face, c, FT_LOAD_RENDER))
			{
				String errMsg = "Unable to load character '" + String(c) + "' from ttf font \"" + file + "\"";
				EU_LOG_WARN(errMsg.C_Str());
				continue;
			}

			if (!font.face->glyph->bitmap.buffer)
				continue;
			
			FontCharacter character;
			character.size = v2(font.face->glyph->bitmap.width, font.face->glyph->bitmap.rows);
			character.bearing = v2(font.face->glyph->bitmap_left, font.face->glyph->bitmap_top);
			character.advance = font.face->glyph->advance.x;

			pixels.Clear();
			u32 numPixels = character.size.x * character.size.y;
			if (pixels.GetCapacity() < numPixels * 4)
				pixels.SetCapacityAndElementCount(numPixels * 4);
			
			for (u32 i = 0; i < character.size.x * character.size.y; i++)
			{
				for(u32 j = 0; j < 4; j++)
					pixels[i * 4 + j] = font.face->glyph->bitmap.buffer[i];
			}

			character.texture = m_RenderContext->CreateTexture2D(&pixels[0], character.size.x, character.size.y, TEXTURE_FORMAT_RGBA8_UNORM);
			
			font.ascii[c] = character;
		}

		m_Fonts.Push(font);

		if (setActive)
			m_ActiveFont = m_Fonts.Size();

		return m_Fonts.Size();
	}

	void Renderer2D::SetActiveFont(FontID font)
	{
		m_ActiveFont = font;
	}

	u32 Renderer2D::GetLineHeight(const String& text, r32 scale)
	{
		const Font& font = m_Fonts[m_ActiveFont - 1];

		r32 maxHeight = 0.0f;
		for (u32 i = 0; i < text.Length(); i++)
			maxHeight = EU_MAX(maxHeight, font.ascii[text[i]].size.y);

		return maxHeight * scale;
	}

	u32 Renderer2D::GetLineWidth(const String& text, r32 scale)
	{
		const Font& font = m_Fonts[m_ActiveFont - 1];

		r32 width = 0.0f;
		for (u32 i = 0; i < text.Length(); i++)
		{
			char c = text[i];
			const FontCharacter& character = font.ascii[c];
			if (i == text.Length() - 1)
				width += character.size.x * scale;
			else
				width += (character.advance >> 6)* scale;
		}

		return width;
	}

	void Renderer2D::SetSpritePosOrigin(SpritePosOrigin origin)
	{
		m_Origin = origin;
	}

	void Renderer2D::SetCamera(const v2& pos, r32 rot)
	{
		m_View = m4::CreateTranslation(v3(pos.x * -1.0f, pos.y, 0.0f));
		if (rot != 0)
			m_View = m_View * m4::CreateRotationZ(rot * -1.0f);
		m_ViewProjection = m_Projection * m_View;
	}

	void Renderer2D::SetProjection(const m4& projection)
	{
		m_Projection = projection;
		m_ViewProjection = m_Projection * m_View;
	}

	void Renderer2D::SetOrthographic(r32 left, r32 right, r32 bottom, r32 top, r32 znear, r32 zfar)
	{
		m_Projection = m4::CreateOrthographic(left, right, bottom, top, znear, zfar);
		m_ViewProjection = m_Projection * m_View;
	}

	SpritePosOrigin Renderer2D::GetSpritePosOrigin()
	{
		return m_Origin;
	}

	const m4& Renderer2D::GetOrthographic()
	{
		return m_Projection;
	}

	void Renderer2D::PushTransformStack(const m3& transform)
	{
		if (m_TransformStackSize == 0)
		{
			m_TransformStack[0] = transform;
		}
		else if (m_TransformStackSize >= EU_RENDERER2D_TRANSFORM_STACK_LIMIT)
		{
			EU_LOG_WARN("Could not push transform stack: size limit reached");
			return;
		}
		else
		{
			m_TransformStack[m_TransformStackSize] = m_TransformStack[m_TransformStackSize - 1] * transform;
		}

		m_TransformStackSize++;
	}

	void Renderer2D::PopTransformStack()
	{
		m_TransformStackSize--;
	}

	TextureID Renderer2D::GetOutputTexture()
	{
		return m_OutputTexture;
	}

	void Renderer2D::ResizeOutput(u32 width, u32 height)
	{
		m_RenderContext->ResizeFramebuffer(m_RenderPass, width, height);
	}

	void Renderer2D::BeginFrame()
	{
		m_NumSpriteGroups = 0;
		m_ColoredSprites.Clear();
		m_Lights.Clear();
		m_Occluders.Clear();
	}

	void Renderer2D::SubmitText(const String& text, v3 pos, v4 color, r32 scale)
	{
		const Font& font = m_Fonts[m_ActiveFont - 1];

		r32 maxHeight = 0.0f;
		for (u32 i = 0; i < text.Length(); i++)
			maxHeight = EU_MAX(maxHeight, font.ascii[text[i]].size.y);

		for (u32 i = 0; i < text.Length(); i++)
		{
			char c = text[i];
			const FontCharacter& character = font.ascii[c];
			Sprite sprite;
			sprite.size.x = character.size.x * scale;
			sprite.size.y = character.size.y * scale;
			sprite.pos.x = pos.x + character.bearing.x * scale;
			sprite.pos.y = pos.y + (character.size.y - character.bearing.y) * scale + maxHeight * scale;
			sprite.color = color;
			sprite.rot = 0.0f;
			sprite.occluder = false;
			sprite.spriteSheet.texture = character.texture;
			sprite.spriteSheet.numRows = 1;
			sprite.spriteSheet.numCols = 1;

			SubmitSprite(sprite);

			pos.x += (character.advance >> 6)* scale;
		}
	}

	void Renderer2D::SubmitLight(const Light& light)
	{
		m_Lights.Push(light);
	}

	void Renderer2D::SubmitSprite(const Sprite& sprite)
	{
		SubmitedSprite s;

		v2 pos = sprite.pos.xy();
		//pos.y = -pos.y;

		switch (m_Origin)
		{
		case SPRITE_POS_ORIGIN_BOTTOM_LEFT: pos = v2(pos.x, pos.y - sprite.size.y); break;
		case SPRITE_POS_ORIGIN_TOP_LEFT: break;
		case SPRITE_POS_ORIGIN_TOP_RIGHT: pos = v2(pos.x - sprite.size.x, pos.y); break;
		case SPRITE_POS_ORIGIN_BOTTOM_RIGHT: pos = pos - sprite.size; break;
		case SPRITE_POS_ORIGIN_CENTER: pos = pos - sprite.size * 0.5f; break;
		}

		m3 transform = m3::CreateRotation(sprite.rot);
		if (m_TransformStackSize > 0)
			transform = m_TransformStack[m_TransformStackSize - 1] * transform;

		v2 texCoord_bl, texCoord_tl, texCoord_tr, texCoord_br;
		texCoord_bl = v2(0.0f, 0.0f);
		texCoord_tl = v2(0.0f, 1.0f);
		texCoord_tr = v2(1.0f, 1.0f);
		texCoord_br = v2(1.0f, 0.0f);

		s.vertices[0].pos = transform * v2(0, 0) + pos;
		s.vertices[0].color = sprite.color;
		s.vertices[0].texCoord = texCoord_bl;

		s.vertices[1].pos = transform * v2(0.0f, sprite.size.y) + pos;
		s.vertices[1].color = sprite.color;
		s.vertices[1].texCoord = texCoord_tl;

		s.vertices[2].pos = transform * sprite.size + pos;
		s.vertices[2].color = sprite.color;
		s.vertices[2].texCoord = texCoord_tr;

		s.vertices[3].pos = transform * v2(sprite.size.x, 0.0f) + pos;
		s.vertices[3].color = sprite.color;
		s.vertices[3].texCoord = texCoord_br;

		if (sprite.occluder)
		{
			SubmitedOccluder occluder;
			occluder.vertices[0].pos = s.vertices[0].pos;
			occluder.vertices[1].pos = s.vertices[1].pos;
			occluder.vertices[2].pos = s.vertices[2].pos;
			occluder.vertices[3].pos = s.vertices[3].pos;

			m_Occluders.Push(occluder);
		}

		/*s.vertices[0].pos = v3(transform * v2(0, 0) + pos, sprite.pos.z);
		s.vertices[0].color = sprite.color;
		s.vertices[0].texCoord = texCoord_bl;

		s.vertices[1].pos = v3(transform * v2(0.0f, sprite.size.y) + pos, sprite.pos.z);
		s.vertices[1].color = sprite.color;
		s.vertices[2].texCoord = texCoord_tl;

		s.vertices[2].pos = v3(transform * sprite.size + pos, sprite.pos.z);
		s.vertices[2].color = sprite.color;
		s.vertices[2].texCoord = texCoord_tr;

		s.vertices[3].pos = v3(transform * v2(sprite.size.x, 0.0f) + pos, sprite.pos.z);
		s.vertices[3].color = sprite.color;
		s.vertices[3].texCoord = texCoord_br;*/

		if (sprite.spriteSheet.texture == EU_INVALID_TEXTURE_ID)
		{
			s.vertices[0].textureIndex = 0.0f;
			s.vertices[1].textureIndex = 0.0f;
			s.vertices[2].textureIndex = 0.0f;
			s.vertices[3].textureIndex = 0.0f;
			m_ColoredSprites.Push(s);
			return;
		}
		else
		{
			if (sprite.spriteSheet.numCols > 1 &&
				sprite.spriteSheet.numRows > 1)
			{
				GetTexCoords(sprite.spriteSheet, sprite.texturePos, &texCoord_bl, &texCoord_tl, &texCoord_tr, &texCoord_br);

				s.vertices[0].texCoord = texCoord_bl;
				s.vertices[2].texCoord = texCoord_tl;
				s.vertices[2].texCoord = texCoord_tr;
				s.vertices[3].texCoord = texCoord_br;
			}
		}

		for (u32 i = 0; i < m_NumSpriteGroups; i++)
		{
			for (u32 j = 0; j < m_SpriteGroups[i].numTextures; j++)
			{
				if (sprite.spriteSheet.texture == m_SpriteGroups[i].textures[j])
				{
					s.vertices[0].textureIndex = j;
					s.vertices[1].textureIndex = j;
					s.vertices[2].textureIndex = j;
					s.vertices[3].textureIndex = j;
					m_SpriteGroups[i].sprites.Push(s);
					return;
				}
			}

			if (m_SpriteGroups[i].numTextures != EU_MAX_ARRAY_OF_TEXTURES_SIZE)
			{
				u32 textureIndex = m_SpriteGroups[i].numTextures;
				s.vertices[0].textureIndex = textureIndex;
				s.vertices[1].textureIndex = textureIndex;
				s.vertices[2].textureIndex = textureIndex;
				s.vertices[3].textureIndex = textureIndex;

				m_SpriteGroups[i].numTextures++;
				m_SpriteGroups[i].textures[textureIndex] = sprite.spriteSheet.texture;
				m_SpriteGroups[i].sprites.Push(s);
				return;
			}
		}

		if (m_NumSpriteGroups == EU_RENDERER2D_MAX_SPRITE_GROUPS)
		{
			EU_LOG_ERROR("Cannot submit this sprite. Sprite group limit has been reached");
			return;
		}

		SpriteGroup* group = &m_SpriteGroups[m_NumSpriteGroups++];
		group->numTextures = 1;
		group->textures[0] = sprite.spriteSheet.texture;

		s.vertices[0].textureIndex = 0.0f;
		s.vertices[1].textureIndex = 0.0f;
		s.vertices[2].textureIndex = 0.0f;
		s.vertices[3].textureIndex = 0.0f;
		group->sprites.Push(s);
	}

	void Renderer2D::SubmitSprite(v3 pos, v2 size, v4 color, r32 rot, b32 occluder)
	{
		Sprite s;
		s.pos = pos;
		s.size = size;
		s.color = color;
		s.rot = rot;
		s.occluder = occluder;
		s.spriteSheet.texture = EU_INVALID_TEXTURE_ID;
		s.spriteSheet.numRows = 0;
		s.spriteSheet.numCols = 0;
		s.texturePos = v2(0, 0);

		SubmitSprite(s);
	}

	void Renderer2D::SubmitSprite(v3 pos, v2 size, SpriteSheet spriteSheet, v2 spritePos, v4 color, r32 rot, b32 occluder)
	{
		Sprite s;
		s.pos = pos;
		s.size = size;
		s.color = color;
		s.rot = rot;
		s.occluder = occluder;
		s.spriteSheet = spriteSheet;
		s.texturePos = spritePos;

		SubmitSprite(s);
	}

	void Renderer2D::SubmitSprite(v3 pos, v2 size, TextureID texture, v4 color, r32 rot, b32 occluder)
	{
		Sprite s;
		s.pos = pos;
		s.size = size;
		s.color = color;
		s.rot = rot;
		s.occluder = occluder;
		s.spriteSheet.texture = texture;
		s.spriteSheet.numRows = 1;
		s.spriteSheet.numCols = 1;
		s.texturePos = v2(0, 0);

		SubmitSprite(s);
	}

	void Renderer2D::EndFrame()
	{
		Renderer2DVertex* vertex = (Renderer2DVertex*)m_RenderContext->MapBuffer(m_VertexBuffer);
		
		if (!m_Occluders.Empty())
		{
			Renderer2DVertex* occluderVertex = (Renderer2DVertex*)m_RenderContext->MapBuffer(m_OcclusionVertexBuffer);
			memcpy(occluderVertex, &m_Occluders[0], sizeof(SubmitedOccluder) * m_Occluders.Size());
			m_RenderContext->UnmapBuffer(m_OcclusionVertexBuffer);
		}

		memcpy(vertex, &m_ColoredSprites[0], sizeof(SubmitedSprite) * m_ColoredSprites.Size());

		u32 currentVertexOffset = m_ColoredSprites.Size() * 4;
		vertex += currentVertexOffset;

		for (u32 i = 0; i < m_NumSpriteGroups; i++)
		{
			m_SpriteGroups[i].vertexOffset = currentVertexOffset;
			m_SpriteGroups[i].indexCount = m_SpriteGroups[i].sprites.Size() * 6;

			memcpy(vertex, &m_SpriteGroups[i].sprites[0], sizeof(SubmitedSprite) * m_SpriteGroups[i].sprites.Size());
			vertex += 4 * m_SpriteGroups[i].sprites.Size();

			currentVertexOffset += m_SpriteGroups[i].sprites.Size() * 4;

			m_SpriteGroups[i].sprites.Clear();
		}

		m_RenderContext->UnmapBuffer(m_VertexBuffer);
	}

	void Renderer2D::RenderFrame()
	{
		RenderPassBeginInfo beginInfo;
		beginInfo.initialPipeline = 0;
		beginInfo.numClearValues = 2;
		beginInfo.clearValues[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		beginInfo.clearValues[0].clearDepthStencil = false;
		beginInfo.clearValues[1].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		beginInfo.clearValues[1].clearDepthStencil = false;
		beginInfo.renderPass = m_RenderPass;

		m_RenderContext->BeginRenderPass(beginInfo);

		m4 viewProjection = m_Projection * m_View;

		if(!m_ColoredSprites.Empty() || m_NumSpriteGroups > 0)
			m_RenderContext->UpdateShaderBuffer(m_PerFrameUBO, &viewProjection, sizeof(m4));

		if (!m_ColoredSprites.Empty())
		{
			TextureGroupBind bind;
			bind.set = 1;
			bind.numTextureBinds = 1;
			bind.binds[0].binding = 0;
			bind.binds[0].sampler = EU_SAMPLER_NEAREST_CLAMP_TO_EDGE;
			bind.binds[0].texture[0] = m_WhiteTexture;
			bind.binds[0].textureArrayLength = 1;

			m_RenderContext->BindTextureGroup(bind);

			RenderCommand command;
			command.vertexBuffer = m_VertexBuffer;
			command.indexBuffer = m_IndexBuffer;
			command.indexOffset = 0;
			command.count = m_ColoredSprites.Size() * 6;
			command.indexType = m_IndexType;
			command.vertexOffset = 0;

			m_RenderContext->SubmitRenderCommand(command);
		}

		for (u32 i = 0; i < m_NumSpriteGroups; i++)
		{
			RenderCommand command;
			command.vertexBuffer = m_VertexBuffer;
			command.indexBuffer = m_IndexBuffer;
			command.indexOffset = 0;
			command.indexType = m_IndexType;
			command.count = m_SpriteGroups[i].indexCount;
			command.vertexOffset = m_SpriteGroups[i].vertexOffset;

			TextureGroupBind bind;
			bind.set = 1;
			bind.numTextureBinds = 1;
			bind.binds[0].binding = 0;
			bind.binds[0].sampler = EU_SAMPLER_NEAREST_CLAMP_TO_EDGE;
			bind.binds[0].textureArrayLength = m_SpriteGroups[i].numTextures;
			memcpy(bind.binds[0].texture, m_SpriteGroups[i].textures, sizeof(TextureID) * m_SpriteGroups[i].numTextures);

			m_RenderContext->BindTextureGroup(bind);
			m_RenderContext->SubmitRenderCommand(command);
		}

		m_RenderContext->EndRenderPass();
	}

	void Renderer2D::GetTexCoords(const SpriteSheet& spriteSheet, const v2& spritePos, v2* bl, v2* tl, v2* tr, v2* br)
	{
		u32 texWidth, texHeight;
		m_RenderContext->GetTextureSize(spriteSheet.texture, &texWidth, &texHeight);

		r32 spriteWidth = texWidth / spriteSheet.numCols;
		r32 spriteHeight = texHeight / spriteSheet.numRows;

		bl->x = spritePos.x * spriteWidth;
		bl->y = (spritePos.y + 1.0f) * spriteHeight;

		tl->x = spritePos.x * spriteWidth;
		tl->y = spritePos.y * spriteHeight;

		tr->x = (spritePos.x + 1.0f) * spriteWidth;
		tr->y = spritePos.y * spriteHeight;

		br->x = (spritePos.x + 1.0f) * spriteWidth;
		br->y = (spritePos.y + 1.0f) * spriteHeight;
	}
}
