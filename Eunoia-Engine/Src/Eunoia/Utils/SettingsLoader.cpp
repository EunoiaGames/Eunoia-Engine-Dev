#include "SettingsLoader.h"
#include "../Core/Engine.h"

namespace Eunoia {

	void SettingsLoader::WriteSettingsToFile(const String& path, const EunoiaSettings& settings)
	{
		FILE* file = fopen(path.C_Str(), "wb");
		if (!file)
			return;

		fwrite(&settings, sizeof(EunoiaSettings), 1, file);
		fclose(file);
	}

	void SettingsLoader::LoadSettingsFromFile(const String& path, b32 applySettings)
	{
		FILE* file = fopen(path.C_Str(), "rb");
		if (!file)
			return;

		EunoiaSettings settings;
		fread(&settings, sizeof(EunoiaSettings), 1, file);
		fclose(file);

		if (applySettings)
			ApplySettings(settings);
	}

	void SettingsLoader::ApplySettings(const EunoiaSettings& settings)
	{
		Renderer2D* r2D = Engine::GetRenderer()->GetRenderer2D();
		Renderer3D* r3D = Engine::GetRenderer()->GetRenderer3D();

		r2D->SetProjection(settings.settings2D.projection);
		r2D->SetSpritePosOrigin(settings.settings2D.origin);
		r3D->SetAmbient(settings.settings3D.ambient);
		r3D->SetBloomThreshold(settings.settings3D.bloomThreshold);
		r3D->SetBloomBlurIterationCount(settings.settings3D.bloomBlurIterCount);
	}

}
