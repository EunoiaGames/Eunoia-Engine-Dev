#pragma once

#include "../Common.h"

#define EU_ECS_INVALID_ID			0
#define EU_ECS_ROOT_ENTITY			1
#define EU_ECS_INVALID_ENTITY_ID	EU_ECS_INVALID_ID
#define EU_ECS_INVALID_SCENE_ID		EU_ECS_INVALID_ID

#define EU_ECS_MAX_COMPONENTS_A_SYSTEM_CAN_PROCESS 16

namespace Eunoia
{
	typedef u32 ECSID;
	typedef ECSID EntityID;
	typedef ECSID SceneID;
}