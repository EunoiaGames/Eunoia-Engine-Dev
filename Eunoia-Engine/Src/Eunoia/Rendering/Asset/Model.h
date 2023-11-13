#pragma once

#include "../RenderContext.h"
#include "AssetTypeIDs.h"

#include "LoadedModel.h"

namespace Eunoia {

	struct Model
	{
		BufferID vertexBuffer;
		BufferID indexBuffer;
		u32 totalIndexCount;
		List<LoadedMesh> meshes;
		List<MaterialID> materials;
		List<MaterialModifierID> modifiers;
		List<ModelBone> bones;
		List<ModelAnimation> animations;
		m4 globalInverseTransform;
		u32 rootBone;
	};

}