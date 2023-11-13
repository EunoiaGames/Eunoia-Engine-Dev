#pragma once

#include "../ECS.h"
#include "../../Rendering/Asset/AssetTypeIDs.h"

namespace Eunoia {

	EU_REFLECT(Component)
	struct ModelComponent : public ECSComponent
	{
		ModelComponent(ModelID model = EU_DEFAULT_MODEL_ID, b32 wireframe = false) :
			model(model),
			wireframe(wireframe)
		{}

		EU_PROPERTY() ModelID model;
		EU_PROPERTY(Bool32) b32 wireframe;
	};

}