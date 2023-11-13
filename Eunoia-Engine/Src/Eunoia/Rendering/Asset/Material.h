#pragma once

#include "../RenderContext.h"
#include "LoadedMaterial.h"

namespace Eunoia {

	struct Material
	{
		String name;
		String path;
		TextureID textures[NUM_MATERIAL_TEXTURE_TYPES];
		SamplerID sampler;
	};

}