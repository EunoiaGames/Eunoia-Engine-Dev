#include "ECS.h"
#include "Systems\TransformHierarchy2DSystem.h"
#include "Systems\TransformHierarchy3DSystem.h"

namespace Eunoia {

	void ECS::AddRequiredSystems()
	{
		CreateSystem<TransformHierarchy3DSystem>();
		CreateSystem<TransformHierarchy2DSystem>();
	}

}
