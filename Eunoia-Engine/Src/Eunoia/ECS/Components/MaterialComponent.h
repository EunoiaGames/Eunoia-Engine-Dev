#pragma once

#include "../ECS.h"
#include "../../Rendering/Asset/AssetTypeIDs.h"

namespace Eunoia {

	EU_REFLECT(Component)
	struct MaterialComponent : public ECSComponent
	{
		MaterialComponent(MaterialID material = EU_DEFAULT_MATERIAL_ID, MaterialModifierID modifier = EU_DEFAULT_MATERIAL_MODIFIER_ID) :
			material(material),
			modifier(modifier)
		{}

		EU_PROPERTY() MaterialID material;
		EU_PROPERTY() MaterialModifierID modifier;
	};

}
