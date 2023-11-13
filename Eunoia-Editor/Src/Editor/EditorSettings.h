#pragma once

#include <Eunoia\DataStructures\List.h>
#include <Eunoia\Core\InputDefs.h>

#define EU_EDITOR_MAX_SHORTCUT_HOLD_KEYS 3

namespace Eunoia_Editor
{
	struct EditorShortcut
	{
		EditorShortcut(u32 numHoldKeys, Eunoia::Key pressKey, Eunoia::Key hold1, Eunoia::Key hold2 = Eunoia::EU_KEY_ERROR, Eunoia::Key hold3 = Eunoia::EU_KEY_ERROR);
		EditorShortcut() {}

		Eunoia::Key holdKeys[EU_EDITOR_MAX_SHORTCUT_HOLD_KEYS];
		u32 numHoldKeys;
		Eunoia::Key pressKey;
	};

	struct EditorKeybinds
	{
		
	};

	struct EditorSettings
	{
		EditorSettings() {}

		EditorShortcut saveProjectShortcut;
		EditorShortcut switchModeShortcut;
		EditorShortcut recompileShortcut;
		EditorShortcut toggleStepApplicationShortcut;
		EditorShortcut pauseShortcut;

		static void LoadEditorSettingsFromFile();
		static EditorSettings* GetEditorSettings();
		static void SaveEditorSettingsToFile();
		
		static void LoadDefaultSettings();
	};
}