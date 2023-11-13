#pragma once

#include "../Rendering/Renderer2D.h"
#include "../Rendering/Renderer3D.h"

namespace Eunoia {

	struct EunoiaSettings2D
	{
		m4 projection;
		SpritePosOrigin origin;
	};

	struct EunoiaSettings3D
	{
		v3 ambient;
		r32 bloomThreshold;
		u32 bloomBlurIterCount;
		LightingModel lightingModel;
	};

	struct EunoiaSettings
	{
		EunoiaSettings2D settings2D;
		EunoiaSettings3D settings3D;
	};

	class EU_API SettingsLoader
	{
	public:
		static void WriteSettingsToFile(const String& path, const EunoiaSettings& settings);
		static void LoadSettingsFromFile(const String& path, b32 applySettings = false);
		static void ApplySettings(const EunoiaSettings& settings);
	};

}