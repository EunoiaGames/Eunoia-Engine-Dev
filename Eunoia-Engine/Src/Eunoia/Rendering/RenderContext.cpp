#include "RenderContext.h"
#include "../Platform/Vulkan/RenderContextVK.h"

namespace Eunoia {

	b32 RenderContext::IsStencilFormat(TextureFormat format)
	{
		return format == TEXTURE_FORMAT_DEPTH32_FLOAT_STENCIL8_UINT;
	}

	b32 RenderContext::IsDepthFormat(TextureFormat format)
	{
		return format == TEXTURE_FORMAT_DEPTH32_FLOAT_STENCIL8_UINT ||
			format == TEXTURE_FORMAT_DEPTH32_FLOAT;
	}

	b32 RenderContext::IsDepthStencilFormat(TextureFormat format)
	{
		return format == TEXTURE_FORMAT_DEPTH32_FLOAT_STENCIL8_UINT;
	}

	RenderContext* RenderContext::CreateRenderContext(RenderAPI api)
	{
		switch (api)
		{
		case RENDER_API_VULKAN: return new RenderContextVK();
		}

		return 0;
	}

}
