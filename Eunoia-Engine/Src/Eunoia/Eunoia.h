#pragma once

#include "Common.h"

#include "Core\Engine.h"
#include "Core\Application.h"
#include "Core\InputDefs.h"
#include "Core\Input.h"

#include "ECS\ECS.h"
#include "ECS\ECSLoader.h"
#include "ECS\Components\Components.h"
#include "ECS\Systems\Systems.h"
#include "ECS\Events\Events.h"

#include "Memory\Allocators.h"

#include "Metadata\MetadataInfo.h"
#include "Metadata\Metadata.h"

#include "DataStructures\String.h"
#include "DataStructures\List.h"
#include "DataStructures\Map.h"

#include "Math\GeneralMath.h"
#include "Math\Math.h"

#include "Rendering\Display.h"
#include "Rendering\RenderContext.h"
#include "Rendering\Renderer2D.h"
#include "Rendering\Renderer3D.h"
#include "Rendering\MasterRenderer.h"
#include "Rendering\GuiManager.h"
#include "Rendering\Light3D.h"

#include "Rendering\Asset\LoadedModel.h"
#include "Rendering\Asset\LoadedMaterial.h"
#include "Rendering\Asset\Model.h"
#include "Rendering\Asset\Material.h"
#include "Rendering\Asset\ModelLoader.h"
#include "Rendering\Asset\MaterialLoader.h"
#include "Rendering\Asset\AssetTypeIDs.h"
#include "Rendering\Asset\AssetManager.h"
#include "Rendering\Asset\TextureLoader.h"

#include "Physics\PhysicsEngine3D.h"
#include "Physics\RigidBody.h"

#include "Utils\Log.h"
#include "Utils\FileUtils.h"
#include "Utils\SettingsLoader.h"
