#include "EditorSettings.h"
#include <cstdio>

namespace Eunoia_Editor {

	struct EditorSettings_Data
	{
		EditorSettings settings;
	};

	static EditorSettings_Data s_Data;

	EditorShortcut::EditorShortcut(u32 numHoldKeys, Eunoia::Key pressKey, Eunoia::Key hold1, Eunoia::Key hold2, Eunoia::Key hold3) :
		numHoldKeys(numHoldKeys),
		pressKey(pressKey)
	{
		holdKeys[0] = hold1;
		holdKeys[1] = hold2;
		holdKeys[2] = hold3;
	}

	void EditorSettings::LoadEditorSettingsFromFile()
	{
		FILE* file = fopen("Res/Settings.eucfg", "rb");
		if (!file)
		{
			LoadDefaultSettings();
			return;
		}

		fread(&s_Data.settings, sizeof(EditorSettings), 1, file);
		fclose(file);
	}

	EditorSettings* EditorSettings::GetEditorSettings()
	{
		return &s_Data.settings;
	}

	void EditorSettings::SaveEditorSettingsToFile()
	{
		FILE* file = fopen("Res/Settings.eucfg", "wb");
		fwrite(&s_Data.settings, sizeof(EditorSettings), 1, file);
		fclose(file);
	}

	void EditorSettings::LoadDefaultSettings()
	{
		EditorSettings* settings = &s_Data.settings;
		settings->saveProjectShortcut = EditorShortcut(1, Eunoia::EU_KEY_S, Eunoia::EU_KEY_CTL);
		settings->switchModeShortcut = EditorShortcut(2, Eunoia::EU_KEY_T, Eunoia::EU_KEY_CTL, Eunoia::EU_KEY_LEFT_ALT);
		settings->recompileShortcut = EditorShortcut(2, Eunoia::EU_KEY_R, Eunoia::EU_KEY_CTL, Eunoia::EU_KEY_LEFT_ALT);
		settings->toggleStepApplicationShortcut = EditorShortcut(2, Eunoia::EU_KEY_P, Eunoia::EU_KEY_CTL, Eunoia::EU_KEY_LEFT_ALT);
		settings->pauseShortcut = EditorShortcut(2, Eunoia::EU_KEY_SPACE, Eunoia::EU_KEY_CTL, Eunoia::EU_KEY_LEFT_ALT);
	}

}
