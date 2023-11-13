#include "../Eunoia.h"

namespace Eunoia {


	template<>
	EU_API metadata_typeid Metadata::GetTypeID < char > () { return 0; }

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < u8 > () { return 1; }

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < u16 > () { return 2; }

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < u32 > () { return 3; }

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < s8 > () { return 4; }

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < s16 > () { return 5; }

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < s32 > () { return 6; }

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < r32 > () { return 7; }

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < r64 > () { return 8; }

	void Metadata::InitMetadataPrimitives()
	{
		MetadataInfo info;
		info.id = 0;
		info.type = METADATA_PRIMITIVE;
		info.primitive = new MetadataPrimitive();
		info.primitive->name = "char";
		info.primitive->size = sizeof( char );
		info.primitive->type = METADATA_PRIMITIVE_CHAR;
		Metadata::RegisterMetadataInfo(info);

		info.id = 1;
		info.type = METADATA_PRIMITIVE;
		info.primitive = new MetadataPrimitive();
		info.primitive->name = "u8";
		info.primitive->size = sizeof( u8 );
		info.primitive->type = METADATA_PRIMITIVE_U8;
		Metadata::RegisterMetadataInfo(info);

		info.id = 2;
		info.type = METADATA_PRIMITIVE;
		info.primitive = new MetadataPrimitive();
		info.primitive->name = "u16";
		info.primitive->size = sizeof( u16 );
		info.primitive->type = METADATA_PRIMITIVE_U16;
		Metadata::RegisterMetadataInfo(info);

		info.id = 3;
		info.type = METADATA_PRIMITIVE;
		info.primitive = new MetadataPrimitive();
		info.primitive->name = "u32";
		info.primitive->size = sizeof( u32 );
		info.primitive->type = METADATA_PRIMITIVE_U32;
		Metadata::RegisterMetadataInfo(info);

		info.id = 4;
		info.type = METADATA_PRIMITIVE;
		info.primitive = new MetadataPrimitive();
		info.primitive->name = "s8";
		info.primitive->size = sizeof( s8 );
		info.primitive->type = METADATA_PRIMITIVE_S8;
		Metadata::RegisterMetadataInfo(info);

		info.id = 5;
		info.type = METADATA_PRIMITIVE;
		info.primitive = new MetadataPrimitive();
		info.primitive->name = "s16";
		info.primitive->size = sizeof( s16 );
		info.primitive->type = METADATA_PRIMITIVE_S16;
		Metadata::RegisterMetadataInfo(info);

		info.id = 6;
		info.type = METADATA_PRIMITIVE;
		info.primitive = new MetadataPrimitive();
		info.primitive->name = "s32";
		info.primitive->size = sizeof( s32 );
		info.primitive->type = METADATA_PRIMITIVE_S32;
		Metadata::RegisterMetadataInfo(info);

		info.id = 7;
		info.type = METADATA_PRIMITIVE;
		info.primitive = new MetadataPrimitive();
		info.primitive->name = "r32";
		info.primitive->size = sizeof( r32 );
		info.primitive->type = METADATA_PRIMITIVE_R32;
		Metadata::RegisterMetadataInfo(info);

		info.id = 8;
		info.type = METADATA_PRIMITIVE;
		info.primitive = new MetadataPrimitive();
		info.primitive->name = "r64";
		info.primitive->size = sizeof( r64 );
		info.primitive->type = METADATA_PRIMITIVE_R64;
		Metadata::RegisterMetadataInfo(info);
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < String > () { return 9; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<String>()
	{
		MetadataInfo info;
		info.id = 9;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "String";
		info.cls->baseClassName = "";
		info.cls->baseClassSize = 0;
		info.cls->size = sizeof( String );
		info.cls->isComponent = false;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< String >;
		info.cls->members.SetCapacityAndElementCount( 2 );

		info.cls->members[ 0 ].name = "m_Chars";
		info.cls->members[ 0 ].typeName = "char";
		info.cls->members[ 0 ].typeID = GetTypeID<char>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PRIVATE;
		info.cls->members[ 0 ].offset = EU_U32_MAX;
		info.cls->members[ 0 ].size = sizeof( char );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = true;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		info.cls->members[ 1 ].name = "m_Length";
		info.cls->members[ 1 ].typeName = "u32";
		info.cls->members[ 1 ].typeID = GetTypeID<u32>();
		info.cls->members[ 1 ].accessModifier = METADATA_ACCESS_MODIFIER_PRIVATE;
		info.cls->members[ 1 ].offset = EU_U32_MAX;
		info.cls->members[ 1 ].size = sizeof( u32 );
		info.cls->members[ 1 ].isStatic = false;
		info.cls->members[ 1 ].isConst = false;
		info.cls->members[ 1 ].isPointer = false;
		info.cls->members[ 1 ].arrayLength = 1;
		info.cls->members[ 1 ].uiSliderMin = v4(0.0);
		info.cls->members[ 1 ].uiSliderMax = v4(0.0);
		info.cls->members[ 1 ].uiSliderSpeed = 0.1;
		info.cls->members[ 1 ].is32BitBool = false;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < Gamepad > () { return 10; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo< Gamepad >()
	{
		MetadataInfo info;
		info.id = 10;
		info.type = METADATA_ENUM;
		info.enm = Eunoia::Metadata::AllocateEnum( true );
		info.enm->name = "Gamepad";
		info.enm->values.SetCapacityAndElementCount( 7 );
		info.enm->values[ 0 ].name = "EU_GAMEPAD_0";
		info.enm->values[ 0 ].value = EU_GAMEPAD_0;
		info.enm->values[ 1 ].name = "EU_GAMEPAD_1";
		info.enm->values[ 1 ].value = EU_GAMEPAD_1;
		info.enm->values[ 2 ].name = "EU_GAMEPAD_2";
		info.enm->values[ 2 ].value = EU_GAMEPAD_2;
		info.enm->values[ 3 ].name = "EU_GAMEPAD_3";
		info.enm->values[ 3 ].value = EU_GAMEPAD_3;
		info.enm->values[ 4 ].name = "EU_NUM_GAMEPADS";
		info.enm->values[ 4 ].value = EU_NUM_GAMEPADS;
		info.enm->values[ 5 ].name = "EU_FIRST_GAMEPAD";
		info.enm->values[ 5 ].value = EU_FIRST_GAMEPAD;
		info.enm->values[ 6 ].name = "EU_LAST_GAMEPAD";
		info.enm->values[ 6 ].value = EU_LAST_GAMEPAD;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < Key > () { return 11; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo< Key >()
	{
		MetadataInfo info;
		info.id = 11;
		info.type = METADATA_ENUM;
		info.enm = Eunoia::Metadata::AllocateEnum( true );
		info.enm->name = "Key";
		info.enm->values.SetCapacityAndElementCount( 93 );
		info.enm->values[ 0 ].name = "EU_KEY_ERROR";
		info.enm->values[ 0 ].value = EU_KEY_ERROR;
		info.enm->values[ 1 ].name = "EU_KEY_ESC";
		info.enm->values[ 1 ].value = EU_KEY_ESC;
		info.enm->values[ 2 ].name = "EU_KEY_1";
		info.enm->values[ 2 ].value = EU_KEY_1;
		info.enm->values[ 3 ].name = "EU_KEY_2";
		info.enm->values[ 3 ].value = EU_KEY_2;
		info.enm->values[ 4 ].name = "EU_KEY_3";
		info.enm->values[ 4 ].value = EU_KEY_3;
		info.enm->values[ 5 ].name = "EU_KEY_4";
		info.enm->values[ 5 ].value = EU_KEY_4;
		info.enm->values[ 6 ].name = "EU_KEY_5";
		info.enm->values[ 6 ].value = EU_KEY_5;
		info.enm->values[ 7 ].name = "EU_KEY_6";
		info.enm->values[ 7 ].value = EU_KEY_6;
		info.enm->values[ 8 ].name = "EU_KEY_7";
		info.enm->values[ 8 ].value = EU_KEY_7;
		info.enm->values[ 9 ].name = "EU_KEY_8";
		info.enm->values[ 9 ].value = EU_KEY_8;
		info.enm->values[ 10 ].name = "EU_KEY_9";
		info.enm->values[ 10 ].value = EU_KEY_9;
		info.enm->values[ 11 ].name = "EU_KEY_0";
		info.enm->values[ 11 ].value = EU_KEY_0;
		info.enm->values[ 12 ].name = "EU_KEY_DASH";
		info.enm->values[ 12 ].value = EU_KEY_DASH;
		info.enm->values[ 13 ].name = "EU_KEY_EQUALS";
		info.enm->values[ 13 ].value = EU_KEY_EQUALS;
		info.enm->values[ 14 ].name = "EU_KEY_BACKSPACE";
		info.enm->values[ 14 ].value = EU_KEY_BACKSPACE;
		info.enm->values[ 15 ].name = "EU_KEY_TAB";
		info.enm->values[ 15 ].value = EU_KEY_TAB;
		info.enm->values[ 16 ].name = "EU_KEY_Q";
		info.enm->values[ 16 ].value = EU_KEY_Q;
		info.enm->values[ 17 ].name = "EU_KEY_W";
		info.enm->values[ 17 ].value = EU_KEY_W;
		info.enm->values[ 18 ].name = "EU_KEY_E";
		info.enm->values[ 18 ].value = EU_KEY_E;
		info.enm->values[ 19 ].name = "EU_KEY_R";
		info.enm->values[ 19 ].value = EU_KEY_R;
		info.enm->values[ 20 ].name = "EU_KEY_T";
		info.enm->values[ 20 ].value = EU_KEY_T;
		info.enm->values[ 21 ].name = "EU_KEY_Y";
		info.enm->values[ 21 ].value = EU_KEY_Y;
		info.enm->values[ 22 ].name = "EU_KEY_U";
		info.enm->values[ 22 ].value = EU_KEY_U;
		info.enm->values[ 23 ].name = "EU_KEY_I";
		info.enm->values[ 23 ].value = EU_KEY_I;
		info.enm->values[ 24 ].name = "EU_KEY_O";
		info.enm->values[ 24 ].value = EU_KEY_O;
		info.enm->values[ 25 ].name = "EU_KEY_P";
		info.enm->values[ 25 ].value = EU_KEY_P;
		info.enm->values[ 26 ].name = "EU_KEY_LEFT_BRACKET";
		info.enm->values[ 26 ].value = EU_KEY_LEFT_BRACKET;
		info.enm->values[ 27 ].name = "EU_KEY_RIGHT_BRACKET";
		info.enm->values[ 27 ].value = EU_KEY_RIGHT_BRACKET;
		info.enm->values[ 28 ].name = "EU_KEY_ENTER";
		info.enm->values[ 28 ].value = EU_KEY_ENTER;
		info.enm->values[ 29 ].name = "EU_KEY_CTL";
		info.enm->values[ 29 ].value = EU_KEY_CTL;
		info.enm->values[ 30 ].name = "EU_KEY_A";
		info.enm->values[ 30 ].value = EU_KEY_A;
		info.enm->values[ 31 ].name = "EU_KEY_S";
		info.enm->values[ 31 ].value = EU_KEY_S;
		info.enm->values[ 32 ].name = "EU_KEY_D";
		info.enm->values[ 32 ].value = EU_KEY_D;
		info.enm->values[ 33 ].name = "EU_KEY_F";
		info.enm->values[ 33 ].value = EU_KEY_F;
		info.enm->values[ 34 ].name = "EU_KEY_G";
		info.enm->values[ 34 ].value = EU_KEY_G;
		info.enm->values[ 35 ].name = "EU_KEY_H";
		info.enm->values[ 35 ].value = EU_KEY_H;
		info.enm->values[ 36 ].name = "EU_KEY_J";
		info.enm->values[ 36 ].value = EU_KEY_J;
		info.enm->values[ 37 ].name = "EU_KEY_K";
		info.enm->values[ 37 ].value = EU_KEY_K;
		info.enm->values[ 38 ].name = "EU_KEY_L";
		info.enm->values[ 38 ].value = EU_KEY_L;
		info.enm->values[ 39 ].name = "EU_KEY_SEMI_COLON";
		info.enm->values[ 39 ].value = EU_KEY_SEMI_COLON;
		info.enm->values[ 40 ].name = "EU_KEY_QUOTE";
		info.enm->values[ 40 ].value = EU_KEY_QUOTE;
		info.enm->values[ 41 ].name = "EU_KEY_TILDE";
		info.enm->values[ 41 ].value = EU_KEY_TILDE;
		info.enm->values[ 42 ].name = "EU_KEY_LEFT_SHIFT";
		info.enm->values[ 42 ].value = EU_KEY_LEFT_SHIFT;
		info.enm->values[ 43 ].name = "EU_KEY_BACK_SLASH";
		info.enm->values[ 43 ].value = EU_KEY_BACK_SLASH;
		info.enm->values[ 44 ].name = "EU_KEY_Z";
		info.enm->values[ 44 ].value = EU_KEY_Z;
		info.enm->values[ 45 ].name = "EU_KEY_X";
		info.enm->values[ 45 ].value = EU_KEY_X;
		info.enm->values[ 46 ].name = "EU_KEY_C";
		info.enm->values[ 46 ].value = EU_KEY_C;
		info.enm->values[ 47 ].name = "EU_KEY_V";
		info.enm->values[ 47 ].value = EU_KEY_V;
		info.enm->values[ 48 ].name = "EU_KEY_B";
		info.enm->values[ 48 ].value = EU_KEY_B;
		info.enm->values[ 49 ].name = "EU_KEY_N";
		info.enm->values[ 49 ].value = EU_KEY_N;
		info.enm->values[ 50 ].name = "EU_KEY_M";
		info.enm->values[ 50 ].value = EU_KEY_M;
		info.enm->values[ 51 ].name = "EU_KEY_COMMA";
		info.enm->values[ 51 ].value = EU_KEY_COMMA;
		info.enm->values[ 52 ].name = "EU_KEY_PERIOD";
		info.enm->values[ 52 ].value = EU_KEY_PERIOD;
		info.enm->values[ 53 ].name = "EU_KEY_SLASH";
		info.enm->values[ 53 ].value = EU_KEY_SLASH;
		info.enm->values[ 54 ].name = "EU_KEY_RIGHT_SHIFT";
		info.enm->values[ 54 ].value = EU_KEY_RIGHT_SHIFT;
		info.enm->values[ 55 ].name = "EU_KEY_KP_ASTERISK";
		info.enm->values[ 55 ].value = EU_KEY_KP_ASTERISK;
		info.enm->values[ 56 ].name = "EU_KEY_LEFT_ALT";
		info.enm->values[ 56 ].value = EU_KEY_LEFT_ALT;
		info.enm->values[ 57 ].name = "EU_KEY_SPACE";
		info.enm->values[ 57 ].value = EU_KEY_SPACE;
		info.enm->values[ 58 ].name = "EU_KEY_CAPS_LOCK";
		info.enm->values[ 58 ].value = EU_KEY_CAPS_LOCK;
		info.enm->values[ 59 ].name = "EU_KEY_F1";
		info.enm->values[ 59 ].value = EU_KEY_F1;
		info.enm->values[ 60 ].name = "EU_KEY_F2";
		info.enm->values[ 60 ].value = EU_KEY_F2;
		info.enm->values[ 61 ].name = "EU_KEY_F3";
		info.enm->values[ 61 ].value = EU_KEY_F3;
		info.enm->values[ 62 ].name = "EU_KEY_F4";
		info.enm->values[ 62 ].value = EU_KEY_F4;
		info.enm->values[ 63 ].name = "EU_KEY_F5";
		info.enm->values[ 63 ].value = EU_KEY_F5;
		info.enm->values[ 64 ].name = "EU_KEY_F6";
		info.enm->values[ 64 ].value = EU_KEY_F6;
		info.enm->values[ 65 ].name = "EU_KEY_F7";
		info.enm->values[ 65 ].value = EU_KEY_F7;
		info.enm->values[ 66 ].name = "EU_KEY_F8";
		info.enm->values[ 66 ].value = EU_KEY_F8;
		info.enm->values[ 67 ].name = "EU_KEY_F9";
		info.enm->values[ 67 ].value = EU_KEY_F9;
		info.enm->values[ 68 ].name = "EU_KEY_F10";
		info.enm->values[ 68 ].value = EU_KEY_F10;
		info.enm->values[ 69 ].name = "EU_KEY_NUM_LOCK";
		info.enm->values[ 69 ].value = EU_KEY_NUM_LOCK;
		info.enm->values[ 70 ].name = "EU_KEY_SCROLL_LOCK";
		info.enm->values[ 70 ].value = EU_KEY_SCROLL_LOCK;
		info.enm->values[ 71 ].name = "EU_KEY_KP_7";
		info.enm->values[ 71 ].value = EU_KEY_KP_7;
		info.enm->values[ 72 ].name = "EU_KEY_KP_8";
		info.enm->values[ 72 ].value = EU_KEY_KP_8;
		info.enm->values[ 73 ].name = "EU_KEY_KP_9";
		info.enm->values[ 73 ].value = EU_KEY_KP_9;
		info.enm->values[ 74 ].name = "EU_KEY_KP_MINUS";
		info.enm->values[ 74 ].value = EU_KEY_KP_MINUS;
		info.enm->values[ 75 ].name = "EU_KEY_KP_4";
		info.enm->values[ 75 ].value = EU_KEY_KP_4;
		info.enm->values[ 76 ].name = "EU_KEY_KP_5";
		info.enm->values[ 76 ].value = EU_KEY_KP_5;
		info.enm->values[ 77 ].name = "EU_KEY_KP_6";
		info.enm->values[ 77 ].value = EU_KEY_KP_6;
		info.enm->values[ 78 ].name = "EU_KEY_KP_PLUS";
		info.enm->values[ 78 ].value = EU_KEY_KP_PLUS;
		info.enm->values[ 79 ].name = "EU_KEY_KP_1";
		info.enm->values[ 79 ].value = EU_KEY_KP_1;
		info.enm->values[ 80 ].name = "EU_KEY_KP_2";
		info.enm->values[ 80 ].value = EU_KEY_KP_2;
		info.enm->values[ 81 ].name = "EU_KEY_KP_3";
		info.enm->values[ 81 ].value = EU_KEY_KP_3;
		info.enm->values[ 82 ].name = "EU_KEY_KP_0";
		info.enm->values[ 82 ].value = EU_KEY_KP_0;
		info.enm->values[ 83 ].name = "EU_KEY_KP_DEL";
		info.enm->values[ 83 ].value = EU_KEY_KP_DEL;
		info.enm->values[ 84 ].name = "EU_KEY_SYS_RQ";
		info.enm->values[ 84 ].value = EU_KEY_SYS_RQ;
		info.enm->values[ 85 ].name = "EU_KEY_BLANK";
		info.enm->values[ 85 ].value = EU_KEY_BLANK;
		info.enm->values[ 86 ].name = "EU_KEY_F11";
		info.enm->values[ 86 ].value = EU_KEY_F11;
		info.enm->values[ 87 ].name = "EU_KEY_F12";
		info.enm->values[ 87 ].value = EU_KEY_F12;
		info.enm->values[ 88 ].name = "EU_KEY_UP";
		info.enm->values[ 88 ].value = EU_KEY_UP;
		info.enm->values[ 89 ].name = "EU_KEY_DOWN";
		info.enm->values[ 89 ].value = EU_KEY_DOWN;
		info.enm->values[ 90 ].name = "EU_KEY_LEFT";
		info.enm->values[ 90 ].value = EU_KEY_LEFT;
		info.enm->values[ 91 ].name = "EU_KEY_RIGHT";
		info.enm->values[ 91 ].value = EU_KEY_RIGHT;
		info.enm->values[ 92 ].name = "EU_NUM_KEYS";
		info.enm->values[ 92 ].value = EU_NUM_KEYS;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < MouseButton > () { return 12; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo< MouseButton >()
	{
		MetadataInfo info;
		info.id = 12;
		info.type = METADATA_ENUM;
		info.enm = Eunoia::Metadata::AllocateEnum( true );
		info.enm->name = "MouseButton";
		info.enm->values.SetCapacityAndElementCount( 7 );
		info.enm->values[ 0 ].name = "EU_BUTTON_LEFT";
		info.enm->values[ 0 ].value = EU_BUTTON_LEFT;
		info.enm->values[ 1 ].name = "EU_BUTTON_RIGHT";
		info.enm->values[ 1 ].value = EU_BUTTON_RIGHT;
		info.enm->values[ 2 ].name = "EU_BUTTON_EXTRA_1";
		info.enm->values[ 2 ].value = EU_BUTTON_EXTRA_1;
		info.enm->values[ 3 ].name = "EU_BUTTON_EXTRA_2";
		info.enm->values[ 3 ].value = EU_BUTTON_EXTRA_2;
		info.enm->values[ 4 ].name = "EU_BUTTON_EXTRA_3";
		info.enm->values[ 4 ].value = EU_BUTTON_EXTRA_3;
		info.enm->values[ 5 ].name = "EU_MOUSE_WHEEL";
		info.enm->values[ 5 ].value = EU_MOUSE_WHEEL;
		info.enm->values[ 6 ].name = "EU_NUM_MOUSE_BUTTONS";
		info.enm->values[ 6 ].value = EU_NUM_MOUSE_BUTTONS;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < GamepadButton > () { return 13; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo< GamepadButton >()
	{
		MetadataInfo info;
		info.id = 13;
		info.type = METADATA_ENUM;
		info.enm = Eunoia::Metadata::AllocateEnum( true );
		info.enm->name = "GamepadButton";
		info.enm->values.SetCapacityAndElementCount( 19 );
		info.enm->values[ 0 ].name = "EU_GAMEPAD_XBOX360_BUTTON_DPAD_UP";
		info.enm->values[ 0 ].value = EU_GAMEPAD_XBOX360_BUTTON_DPAD_UP;
		info.enm->values[ 1 ].name = "EU_GAMEPAD_XBOX360_BUTTON_DPAD_DOWN";
		info.enm->values[ 1 ].value = EU_GAMEPAD_XBOX360_BUTTON_DPAD_DOWN;
		info.enm->values[ 2 ].name = "EU_GAMEPAD_XBOX360_BUTTON_DPAD_LEFT";
		info.enm->values[ 2 ].value = EU_GAMEPAD_XBOX360_BUTTON_DPAD_LEFT;
		info.enm->values[ 3 ].name = "EU_GAMEPAD_XBOX360_BUTTON_DPAD_RIGHT";
		info.enm->values[ 3 ].value = EU_GAMEPAD_XBOX360_BUTTON_DPAD_RIGHT;
		info.enm->values[ 4 ].name = "EU_GAMEPAD_XBOX360_BUTTON_START";
		info.enm->values[ 4 ].value = EU_GAMEPAD_XBOX360_BUTTON_START;
		info.enm->values[ 5 ].name = "EU_GAMEPAD_XBOX360_BUTTON_BACK";
		info.enm->values[ 5 ].value = EU_GAMEPAD_XBOX360_BUTTON_BACK;
		info.enm->values[ 6 ].name = "EU_GAMEPAD_XBOX360_BUTTON_LEFT_THUMB";
		info.enm->values[ 6 ].value = EU_GAMEPAD_XBOX360_BUTTON_LEFT_THUMB;
		info.enm->values[ 7 ].name = "EU_GAMEPAD_XBOX360_BUTTON_RIGHT_THUMB";
		info.enm->values[ 7 ].value = EU_GAMEPAD_XBOX360_BUTTON_RIGHT_THUMB;
		info.enm->values[ 8 ].name = "EU_GAMEPAD_XBOX360_BUTTON_LEFT_SHOULDER";
		info.enm->values[ 8 ].value = EU_GAMEPAD_XBOX360_BUTTON_LEFT_SHOULDER;
		info.enm->values[ 9 ].name = "EU_GAMEPAD_XBOX360_BUTTON_RIGHT_SHOULDER";
		info.enm->values[ 9 ].value = EU_GAMEPAD_XBOX360_BUTTON_RIGHT_SHOULDER;
		info.enm->values[ 10 ].name = "EU_GAMEPAD_XBOX360_BUTTON_A";
		info.enm->values[ 10 ].value = EU_GAMEPAD_XBOX360_BUTTON_A;
		info.enm->values[ 11 ].name = "EU_GAMEPAD_XBOX360_BUTTON_B";
		info.enm->values[ 11 ].value = EU_GAMEPAD_XBOX360_BUTTON_B;
		info.enm->values[ 12 ].name = "EU_GAMEPAD_XBOX360_BUTTON_X";
		info.enm->values[ 12 ].value = EU_GAMEPAD_XBOX360_BUTTON_X;
		info.enm->values[ 13 ].name = "EU_GAMEPAD_XBOX360_BUTTON_Y";
		info.enm->values[ 13 ].value = EU_GAMEPAD_XBOX360_BUTTON_Y;
		info.enm->values[ 14 ].name = "EU_GAMEPAD_XBOX360_BUTTON_FIRST_BUTTON";
		info.enm->values[ 14 ].value = EU_GAMEPAD_XBOX360_BUTTON_FIRST_BUTTON;
		info.enm->values[ 15 ].name = "EU_GAMEPAD_XBOX360_BUTTON_LAST_BUTTON";
		info.enm->values[ 15 ].value = EU_GAMEPAD_XBOX360_BUTTON_LAST_BUTTON;
		info.enm->values[ 16 ].name = "EU_GAMEPAD_XBOX360_BUTTON_LT";
		info.enm->values[ 16 ].value = EU_GAMEPAD_XBOX360_BUTTON_LT;
		info.enm->values[ 17 ].name = "EU_GAMEPAD_XBOX360_BUTTON_RT";
		info.enm->values[ 17 ].value = EU_GAMEPAD_XBOX360_BUTTON_RT;
		info.enm->values[ 18 ].name = "EU_NUM_GAMEPAD_BUTTONS";
		info.enm->values[ 18 ].value = EU_NUM_GAMEPAD_BUTTONS;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < GamepadTrigger > () { return 14; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo< GamepadTrigger >()
	{
		MetadataInfo info;
		info.id = 14;
		info.type = METADATA_ENUM;
		info.enm = Eunoia::Metadata::AllocateEnum( true );
		info.enm->name = "GamepadTrigger";
		info.enm->values.SetCapacityAndElementCount( 3 );
		info.enm->values[ 0 ].name = "EU_GAMEPAD_XBOX360_TRIGGER_LT";
		info.enm->values[ 0 ].value = EU_GAMEPAD_XBOX360_TRIGGER_LT;
		info.enm->values[ 1 ].name = "EU_GAMEPAD_XBOX360_TRIGGER_RT";
		info.enm->values[ 1 ].value = EU_GAMEPAD_XBOX360_TRIGGER_RT;
		info.enm->values[ 2 ].name = "EU_NUM_GAMEPAD_TRIGGERS";
		info.enm->values[ 2 ].value = EU_NUM_GAMEPAD_TRIGGERS;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < GamepadThumbstick > () { return 15; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo< GamepadThumbstick >()
	{
		MetadataInfo info;
		info.id = 15;
		info.type = METADATA_ENUM;
		info.enm = Eunoia::Metadata::AllocateEnum( true );
		info.enm->name = "GamepadThumbstick";
		info.enm->values.SetCapacityAndElementCount( 3 );
		info.enm->values[ 0 ].name = "EU_GAMEPAD_XBOX360_THUMBSTICK_LEFT";
		info.enm->values[ 0 ].value = EU_GAMEPAD_XBOX360_THUMBSTICK_LEFT;
		info.enm->values[ 1 ].name = "EU_GAMEPAD_XBOX360_THUMBSTICK_RIGHT";
		info.enm->values[ 1 ].value = EU_GAMEPAD_XBOX360_THUMBSTICK_RIGHT;
		info.enm->values[ 2 ].name = "EU_NUM_GAMEPAD_THUMBSTICKS";
		info.enm->values[ 2 ].value = EU_NUM_GAMEPAD_THUMBSTICKS;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < v2 > () { return 16; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<v2>()
	{
		MetadataInfo info;
		info.id = 16;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "v2";
		info.cls->baseClassName = "";
		info.cls->baseClassSize = 0;
		info.cls->size = sizeof( v2 );
		info.cls->isComponent = false;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< v2 >;
		info.cls->members.SetCapacityAndElementCount( 2 );

		info.cls->members[ 0 ].name = "x";
		info.cls->members[ 0 ].typeName = "r32";
		info.cls->members[ 0 ].typeID = GetTypeID<r32>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( v2, v2::x );
		info.cls->members[ 0 ].size = sizeof( r32 );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		info.cls->members[ 1 ].name = "y";
		info.cls->members[ 1 ].typeName = "r32";
		info.cls->members[ 1 ].typeID = GetTypeID<r32>();
		info.cls->members[ 1 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 1 ].offset = offsetof( v2, v2::y );
		info.cls->members[ 1 ].size = sizeof( r32 );
		info.cls->members[ 1 ].isStatic = false;
		info.cls->members[ 1 ].isConst = false;
		info.cls->members[ 1 ].isPointer = false;
		info.cls->members[ 1 ].arrayLength = 1;
		info.cls->members[ 1 ].uiSliderMin = v4(0.0);
		info.cls->members[ 1 ].uiSliderMax = v4(0.0);
		info.cls->members[ 1 ].uiSliderSpeed = 0.1;
		info.cls->members[ 1 ].is32BitBool = false;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < v3 > () { return 17; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<v3>()
	{
		MetadataInfo info;
		info.id = 17;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "v3";
		info.cls->baseClassName = "";
		info.cls->baseClassSize = 0;
		info.cls->size = sizeof( v3 );
		info.cls->isComponent = false;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< v3 >;
		info.cls->members.SetCapacityAndElementCount( 3 );

		info.cls->members[ 0 ].name = "x";
		info.cls->members[ 0 ].typeName = "r32";
		info.cls->members[ 0 ].typeID = GetTypeID<r32>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( v3, v3::x );
		info.cls->members[ 0 ].size = sizeof( r32 );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		info.cls->members[ 1 ].name = "y";
		info.cls->members[ 1 ].typeName = "r32";
		info.cls->members[ 1 ].typeID = GetTypeID<r32>();
		info.cls->members[ 1 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 1 ].offset = offsetof( v3, v3::y );
		info.cls->members[ 1 ].size = sizeof( r32 );
		info.cls->members[ 1 ].isStatic = false;
		info.cls->members[ 1 ].isConst = false;
		info.cls->members[ 1 ].isPointer = false;
		info.cls->members[ 1 ].arrayLength = 1;
		info.cls->members[ 1 ].uiSliderMin = v4(0.0);
		info.cls->members[ 1 ].uiSliderMax = v4(0.0);
		info.cls->members[ 1 ].uiSliderSpeed = 0.1;
		info.cls->members[ 1 ].is32BitBool = false;

		info.cls->members[ 2 ].name = "z";
		info.cls->members[ 2 ].typeName = "r32";
		info.cls->members[ 2 ].typeID = GetTypeID<r32>();
		info.cls->members[ 2 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 2 ].offset = offsetof( v3, v3::z );
		info.cls->members[ 2 ].size = sizeof( r32 );
		info.cls->members[ 2 ].isStatic = false;
		info.cls->members[ 2 ].isConst = false;
		info.cls->members[ 2 ].isPointer = false;
		info.cls->members[ 2 ].arrayLength = 1;
		info.cls->members[ 2 ].uiSliderMin = v4(0.0);
		info.cls->members[ 2 ].uiSliderMax = v4(0.0);
		info.cls->members[ 2 ].uiSliderSpeed = 0.1;
		info.cls->members[ 2 ].is32BitBool = false;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < v4 > () { return 18; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<v4>()
	{
		MetadataInfo info;
		info.id = 18;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "v4";
		info.cls->baseClassName = "";
		info.cls->baseClassSize = 0;
		info.cls->size = sizeof( v4 );
		info.cls->isComponent = false;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< v4 >;
		info.cls->members.SetCapacityAndElementCount( 4 );

		info.cls->members[ 0 ].name = "x";
		info.cls->members[ 0 ].typeName = "r32";
		info.cls->members[ 0 ].typeID = GetTypeID<r32>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( v4, v4::x );
		info.cls->members[ 0 ].size = sizeof( r32 );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		info.cls->members[ 1 ].name = "y";
		info.cls->members[ 1 ].typeName = "r32";
		info.cls->members[ 1 ].typeID = GetTypeID<r32>();
		info.cls->members[ 1 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 1 ].offset = offsetof( v4, v4::y );
		info.cls->members[ 1 ].size = sizeof( r32 );
		info.cls->members[ 1 ].isStatic = false;
		info.cls->members[ 1 ].isConst = false;
		info.cls->members[ 1 ].isPointer = false;
		info.cls->members[ 1 ].arrayLength = 1;
		info.cls->members[ 1 ].uiSliderMin = v4(0.0);
		info.cls->members[ 1 ].uiSliderMax = v4(0.0);
		info.cls->members[ 1 ].uiSliderSpeed = 0.1;
		info.cls->members[ 1 ].is32BitBool = false;

		info.cls->members[ 2 ].name = "z";
		info.cls->members[ 2 ].typeName = "r32";
		info.cls->members[ 2 ].typeID = GetTypeID<r32>();
		info.cls->members[ 2 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 2 ].offset = offsetof( v4, v4::z );
		info.cls->members[ 2 ].size = sizeof( r32 );
		info.cls->members[ 2 ].isStatic = false;
		info.cls->members[ 2 ].isConst = false;
		info.cls->members[ 2 ].isPointer = false;
		info.cls->members[ 2 ].arrayLength = 1;
		info.cls->members[ 2 ].uiSliderMin = v4(0.0);
		info.cls->members[ 2 ].uiSliderMax = v4(0.0);
		info.cls->members[ 2 ].uiSliderSpeed = 0.1;
		info.cls->members[ 2 ].is32BitBool = false;

		info.cls->members[ 3 ].name = "w";
		info.cls->members[ 3 ].typeName = "r32";
		info.cls->members[ 3 ].typeID = GetTypeID<r32>();
		info.cls->members[ 3 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 3 ].offset = offsetof( v4, v4::w );
		info.cls->members[ 3 ].size = sizeof( r32 );
		info.cls->members[ 3 ].isStatic = false;
		info.cls->members[ 3 ].isConst = false;
		info.cls->members[ 3 ].isPointer = false;
		info.cls->members[ 3 ].arrayLength = 1;
		info.cls->members[ 3 ].uiSliderMin = v4(0.0);
		info.cls->members[ 3 ].uiSliderMax = v4(0.0);
		info.cls->members[ 3 ].uiSliderSpeed = 0.1;
		info.cls->members[ 3 ].is32BitBool = false;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < m3 > () { return 19; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<m3>()
	{
		MetadataInfo info;
		info.id = 19;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "m3";
		info.cls->baseClassName = "";
		info.cls->baseClassSize = 0;
		info.cls->size = sizeof( m3 );
		info.cls->isComponent = false;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< m3 >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < m4 > () { return 20; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<m4>()
	{
		MetadataInfo info;
		info.id = 20;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "m4";
		info.cls->baseClassName = "";
		info.cls->baseClassSize = 0;
		info.cls->size = sizeof( m4 );
		info.cls->isComponent = false;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< m4 >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < quat > () { return 21; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<quat>()
	{
		MetadataInfo info;
		info.id = 21;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "quat";
		info.cls->baseClassName = "";
		info.cls->baseClassSize = 0;
		info.cls->size = sizeof( quat );
		info.cls->isComponent = false;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< quat >;
		info.cls->members.SetCapacityAndElementCount( 4 );

		info.cls->members[ 0 ].name = "x";
		info.cls->members[ 0 ].typeName = "r32";
		info.cls->members[ 0 ].typeID = GetTypeID<r32>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( quat, quat::x );
		info.cls->members[ 0 ].size = sizeof( r32 );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		info.cls->members[ 1 ].name = "y";
		info.cls->members[ 1 ].typeName = "r32";
		info.cls->members[ 1 ].typeID = GetTypeID<r32>();
		info.cls->members[ 1 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 1 ].offset = offsetof( quat, quat::y );
		info.cls->members[ 1 ].size = sizeof( r32 );
		info.cls->members[ 1 ].isStatic = false;
		info.cls->members[ 1 ].isConst = false;
		info.cls->members[ 1 ].isPointer = false;
		info.cls->members[ 1 ].arrayLength = 1;
		info.cls->members[ 1 ].uiSliderMin = v4(0.0);
		info.cls->members[ 1 ].uiSliderMax = v4(0.0);
		info.cls->members[ 1 ].uiSliderSpeed = 0.1;
		info.cls->members[ 1 ].is32BitBool = false;

		info.cls->members[ 2 ].name = "z";
		info.cls->members[ 2 ].typeName = "r32";
		info.cls->members[ 2 ].typeID = GetTypeID<r32>();
		info.cls->members[ 2 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 2 ].offset = offsetof( quat, quat::z );
		info.cls->members[ 2 ].size = sizeof( r32 );
		info.cls->members[ 2 ].isStatic = false;
		info.cls->members[ 2 ].isConst = false;
		info.cls->members[ 2 ].isPointer = false;
		info.cls->members[ 2 ].arrayLength = 1;
		info.cls->members[ 2 ].uiSliderMin = v4(0.0);
		info.cls->members[ 2 ].uiSliderMax = v4(0.0);
		info.cls->members[ 2 ].uiSliderSpeed = 0.1;
		info.cls->members[ 2 ].is32BitBool = false;

		info.cls->members[ 3 ].name = "w";
		info.cls->members[ 3 ].typeName = "r32";
		info.cls->members[ 3 ].typeID = GetTypeID<r32>();
		info.cls->members[ 3 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 3 ].offset = offsetof( quat, quat::w );
		info.cls->members[ 3 ].size = sizeof( r32 );
		info.cls->members[ 3 ].isStatic = false;
		info.cls->members[ 3 ].isConst = false;
		info.cls->members[ 3 ].isPointer = false;
		info.cls->members[ 3 ].arrayLength = 1;
		info.cls->members[ 3 ].uiSliderMin = v4(0.0);
		info.cls->members[ 3 ].uiSliderMax = v4(0.0);
		info.cls->members[ 3 ].uiSliderSpeed = 0.1;
		info.cls->members[ 3 ].is32BitBool = false;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < Transform2D > () { return 22; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<Transform2D>()
	{
		MetadataInfo info;
		info.id = 22;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "Transform2D";
		info.cls->baseClassName = "";
		info.cls->baseClassSize = 0;
		info.cls->size = sizeof( Transform2D );
		info.cls->isComponent = false;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< Transform2D >;
		info.cls->members.SetCapacityAndElementCount( 3 );

		info.cls->members[ 0 ].name = "pos";
		info.cls->members[ 0 ].typeName = "v2";
		info.cls->members[ 0 ].typeID = GetTypeID<v2>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( Transform2D, Transform2D::pos );
		info.cls->members[ 0 ].size = sizeof( v2 );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		info.cls->members[ 1 ].name = "scale";
		info.cls->members[ 1 ].typeName = "v2";
		info.cls->members[ 1 ].typeID = GetTypeID<v2>();
		info.cls->members[ 1 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 1 ].offset = offsetof( Transform2D, Transform2D::scale );
		info.cls->members[ 1 ].size = sizeof( v2 );
		info.cls->members[ 1 ].isStatic = false;
		info.cls->members[ 1 ].isConst = false;
		info.cls->members[ 1 ].isPointer = false;
		info.cls->members[ 1 ].arrayLength = 1;
		info.cls->members[ 1 ].uiSliderMin = v4(0.0);
		info.cls->members[ 1 ].uiSliderMax = v4(0.0);
		info.cls->members[ 1 ].uiSliderSpeed = 0.1;
		info.cls->members[ 1 ].is32BitBool = false;

		info.cls->members[ 2 ].name = "rot";
		info.cls->members[ 2 ].typeName = "r32";
		info.cls->members[ 2 ].typeID = GetTypeID<r32>();
		info.cls->members[ 2 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 2 ].offset = offsetof( Transform2D, Transform2D::rot );
		info.cls->members[ 2 ].size = sizeof( r32 );
		info.cls->members[ 2 ].isStatic = false;
		info.cls->members[ 2 ].isConst = false;
		info.cls->members[ 2 ].isPointer = false;
		info.cls->members[ 2 ].arrayLength = 1;
		info.cls->members[ 2 ].uiSliderMin = v4(0.0);
		info.cls->members[ 2 ].uiSliderMax = v4(0.0);
		info.cls->members[ 2 ].uiSliderSpeed = 0.1;
		info.cls->members[ 2 ].is32BitBool = false;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < Transform3D > () { return 23; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<Transform3D>()
	{
		MetadataInfo info;
		info.id = 23;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "Transform3D";
		info.cls->baseClassName = "";
		info.cls->baseClassSize = 0;
		info.cls->size = sizeof( Transform3D );
		info.cls->isComponent = false;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< Transform3D >;
		info.cls->members.SetCapacityAndElementCount( 3 );

		info.cls->members[ 0 ].name = "pos";
		info.cls->members[ 0 ].typeName = "v3";
		info.cls->members[ 0 ].typeID = GetTypeID<v3>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( Transform3D, Transform3D::pos );
		info.cls->members[ 0 ].size = sizeof( v3 );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		info.cls->members[ 1 ].name = "scale";
		info.cls->members[ 1 ].typeName = "v3";
		info.cls->members[ 1 ].typeID = GetTypeID<v3>();
		info.cls->members[ 1 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 1 ].offset = offsetof( Transform3D, Transform3D::scale );
		info.cls->members[ 1 ].size = sizeof( v3 );
		info.cls->members[ 1 ].isStatic = false;
		info.cls->members[ 1 ].isConst = false;
		info.cls->members[ 1 ].isPointer = false;
		info.cls->members[ 1 ].arrayLength = 1;
		info.cls->members[ 1 ].uiSliderMin = v4(0.0);
		info.cls->members[ 1 ].uiSliderMax = v4(0.0);
		info.cls->members[ 1 ].uiSliderSpeed = 0.1;
		info.cls->members[ 1 ].is32BitBool = false;

		info.cls->members[ 2 ].name = "rot";
		info.cls->members[ 2 ].typeName = "quat";
		info.cls->members[ 2 ].typeID = GetTypeID<quat>();
		info.cls->members[ 2 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 2 ].offset = offsetof( Transform3D, Transform3D::rot );
		info.cls->members[ 2 ].size = sizeof( quat );
		info.cls->members[ 2 ].isStatic = false;
		info.cls->members[ 2 ].isConst = false;
		info.cls->members[ 2 ].isPointer = false;
		info.cls->members[ 2 ].arrayLength = 1;
		info.cls->members[ 2 ].uiSliderMin = v4(0.0);
		info.cls->members[ 2 ].uiSliderMax = v4(0.0);
		info.cls->members[ 2 ].uiSliderSpeed = 0.1;
		info.cls->members[ 2 ].is32BitBool = false;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < SpriteSheet > () { return 24; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<SpriteSheet>()
	{
		MetadataInfo info;
		info.id = 24;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "SpriteSheet";
		info.cls->baseClassName = "";
		info.cls->baseClassSize = 0;
		info.cls->size = sizeof( SpriteSheet );
		info.cls->isComponent = false;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< SpriteSheet >;
		info.cls->members.SetCapacityAndElementCount( 3 );

		info.cls->members[ 0 ].name = "texture";
		info.cls->members[ 0 ].typeName = "TextureID";
		info.cls->members[ 0 ].typeID = GetTypeID<TextureID>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( SpriteSheet, SpriteSheet::texture );
		info.cls->members[ 0 ].size = sizeof( TextureID );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		info.cls->members[ 1 ].name = "numRows";
		info.cls->members[ 1 ].typeName = "u32";
		info.cls->members[ 1 ].typeID = GetTypeID<u32>();
		info.cls->members[ 1 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 1 ].offset = offsetof( SpriteSheet, SpriteSheet::numRows );
		info.cls->members[ 1 ].size = sizeof( u32 );
		info.cls->members[ 1 ].isStatic = false;
		info.cls->members[ 1 ].isConst = false;
		info.cls->members[ 1 ].isPointer = false;
		info.cls->members[ 1 ].arrayLength = 1;
		info.cls->members[ 1 ].uiSliderMin = v4(0.0);
		info.cls->members[ 1 ].uiSliderMax = v4(0.0);
		info.cls->members[ 1 ].uiSliderSpeed = 0.1;
		info.cls->members[ 1 ].is32BitBool = false;

		info.cls->members[ 2 ].name = "numCols";
		info.cls->members[ 2 ].typeName = "u32";
		info.cls->members[ 2 ].typeID = GetTypeID<u32>();
		info.cls->members[ 2 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 2 ].offset = offsetof( SpriteSheet, SpriteSheet::numCols );
		info.cls->members[ 2 ].size = sizeof( u32 );
		info.cls->members[ 2 ].isStatic = false;
		info.cls->members[ 2 ].isConst = false;
		info.cls->members[ 2 ].isPointer = false;
		info.cls->members[ 2 ].arrayLength = 1;
		info.cls->members[ 2 ].uiSliderMin = v4(0.0);
		info.cls->members[ 2 ].uiSliderMax = v4(0.0);
		info.cls->members[ 2 ].uiSliderSpeed = 0.1;
		info.cls->members[ 2 ].is32BitBool = false;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < Light3DType > () { return 25; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo< Light3DType >()
	{
		MetadataInfo info;
		info.id = 25;
		info.type = METADATA_ENUM;
		info.enm = Eunoia::Metadata::AllocateEnum( true );
		info.enm->name = "Light3DType";
		info.enm->values.SetCapacityAndElementCount( 4 );
		info.enm->values[ 0 ].name = "LIGHT3D_DIRECTIONAL";
		info.enm->values[ 0 ].value = LIGHT3D_DIRECTIONAL;
		info.enm->values[ 1 ].name = "LIGHT3D_POINT";
		info.enm->values[ 1 ].value = LIGHT3D_POINT;
		info.enm->values[ 2 ].name = "LIGHT3D_SPOTLIGHT";
		info.enm->values[ 2 ].value = LIGHT3D_SPOTLIGHT;
		info.enm->values[ 3 ].name = "NUM_LIGHT3D_TYPES";
		info.enm->values[ 3 ].value = NUM_LIGHT3D_TYPES;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < Attenuation > () { return 26; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<Attenuation>()
	{
		MetadataInfo info;
		info.id = 26;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "Attenuation";
		info.cls->baseClassName = "";
		info.cls->baseClassSize = 0;
		info.cls->size = sizeof( Attenuation );
		info.cls->isComponent = false;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< Attenuation >;
		info.cls->members.SetCapacityAndElementCount( 3 );

		info.cls->members[ 0 ].name = "constant";
		info.cls->members[ 0 ].typeName = "r32";
		info.cls->members[ 0 ].typeID = GetTypeID<r32>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( Attenuation, Attenuation::constant );
		info.cls->members[ 0 ].size = sizeof( r32 );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		info.cls->members[ 1 ].name = "linear";
		info.cls->members[ 1 ].typeName = "r32";
		info.cls->members[ 1 ].typeID = GetTypeID<r32>();
		info.cls->members[ 1 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 1 ].offset = offsetof( Attenuation, Attenuation::linear );
		info.cls->members[ 1 ].size = sizeof( r32 );
		info.cls->members[ 1 ].isStatic = false;
		info.cls->members[ 1 ].isConst = false;
		info.cls->members[ 1 ].isPointer = false;
		info.cls->members[ 1 ].arrayLength = 1;
		info.cls->members[ 1 ].uiSliderMin = v4(0.0);
		info.cls->members[ 1 ].uiSliderMax = v4(0.0);
		info.cls->members[ 1 ].uiSliderSpeed = 0.1;
		info.cls->members[ 1 ].is32BitBool = false;

		info.cls->members[ 2 ].name = "quadratic";
		info.cls->members[ 2 ].typeName = "r32";
		info.cls->members[ 2 ].typeID = GetTypeID<r32>();
		info.cls->members[ 2 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 2 ].offset = offsetof( Attenuation, Attenuation::quadratic );
		info.cls->members[ 2 ].size = sizeof( r32 );
		info.cls->members[ 2 ].isStatic = false;
		info.cls->members[ 2 ].isConst = false;
		info.cls->members[ 2 ].isPointer = false;
		info.cls->members[ 2 ].arrayLength = 1;
		info.cls->members[ 2 ].uiSliderMin = v4(0.0);
		info.cls->members[ 2 ].uiSliderMax = v4(0.0);
		info.cls->members[ 2 ].uiSliderSpeed = 0.1;
		info.cls->members[ 2 ].is32BitBool = false;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < ShadowInfo > () { return 27; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<ShadowInfo>()
	{
		MetadataInfo info;
		info.id = 27;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "ShadowInfo";
		info.cls->baseClassName = "";
		info.cls->baseClassSize = 0;
		info.cls->size = sizeof( ShadowInfo );
		info.cls->isComponent = false;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< ShadowInfo >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < Light3D > () { return 28; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<Light3D>()
	{
		MetadataInfo info;
		info.id = 28;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "Light3D";
		info.cls->baseClassName = "";
		info.cls->baseClassSize = 0;
		info.cls->size = sizeof( Light3D );
		info.cls->isComponent = false;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< Light3D >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < ECSComponent > () { return 29; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<ECSComponent>()
	{
		MetadataInfo info;
		info.id = 29;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "ECSComponent";
		info.cls->baseClassName = "";
		info.cls->baseClassSize = 0;
		info.cls->size = sizeof( ECSComponent );
		info.cls->isComponent = false;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< ECSComponent >;
		info.cls->members.SetCapacityAndElementCount( 2 );

		info.cls->members[ 0 ].name = "enabled";
		info.cls->members[ 0 ].typeName = "b32";
		info.cls->members[ 0 ].typeID = GetTypeID<b32>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( ECSComponent, ECSComponent::enabled );
		info.cls->members[ 0 ].size = sizeof( b32 );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		info.cls->members[ 1 ].name = "parent";
		info.cls->members[ 1 ].typeName = "EntityID";
		info.cls->members[ 1 ].typeID = GetTypeID<EntityID>();
		info.cls->members[ 1 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 1 ].offset = offsetof( ECSComponent, ECSComponent::parent );
		info.cls->members[ 1 ].size = sizeof( EntityID );
		info.cls->members[ 1 ].isStatic = false;
		info.cls->members[ 1 ].isConst = false;
		info.cls->members[ 1 ].isPointer = false;
		info.cls->members[ 1 ].arrayLength = 1;
		info.cls->members[ 1 ].uiSliderMin = v4(0.0);
		info.cls->members[ 1 ].uiSliderMax = v4(0.0);
		info.cls->members[ 1 ].uiSliderSpeed = 0.1;
		info.cls->members[ 1 ].is32BitBool = false;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < ECSEvent > () { return 30; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<ECSEvent>()
	{
		MetadataInfo info;
		info.id = 30;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "ECSEvent";
		info.cls->baseClassName = "";
		info.cls->baseClassSize = 0;
		info.cls->size = sizeof( ECSEvent );
		info.cls->isComponent = false;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< ECSEvent >;
		info.cls->members.SetCapacityAndElementCount( 1 );

		info.cls->members[ 0 ].name = "time";
		info.cls->members[ 0 ].typeName = "r32";
		info.cls->members[ 0 ].typeID = GetTypeID<r32>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( ECSEvent, ECSEvent::time );
		info.cls->members[ 0 ].size = sizeof( r32 );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < ECSSystem > () { return 31; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<ECSSystem>()
	{
		MetadataInfo info;
		info.id = 31;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "ECSSystem";
		info.cls->baseClassName = "";
		info.cls->baseClassSize = 0;
		info.cls->size = sizeof( ECSSystem );
		info.cls->isComponent = false;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< ECSSystem >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < CameraComponent > () { return 32; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<CameraComponent>()
	{
		MetadataInfo info;
		info.id = 32;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "CameraComponent";
		info.cls->baseClassName = "ECSComponent";
		info.cls->baseClassSize = sizeof( ECSComponent );
		info.cls->size = sizeof( CameraComponent );
		info.cls->isComponent = true;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< CameraComponent >;
		info.cls->members.SetCapacityAndElementCount( 1 );

		info.cls->members[ 0 ].name = "fov";
		info.cls->members[ 0 ].typeName = "r32";
		info.cls->members[ 0 ].typeID = GetTypeID<r32>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( CameraComponent, CameraComponent::fov );
		info.cls->members[ 0 ].size = sizeof( r32 );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < KeyboardMovement3DComponent > () { return 33; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<KeyboardMovement3DComponent>()
	{
		MetadataInfo info;
		info.id = 33;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "KeyboardMovement3DComponent";
		info.cls->baseClassName = "ECSComponent";
		info.cls->baseClassSize = sizeof( ECSComponent );
		info.cls->size = sizeof( KeyboardMovement3DComponent );
		info.cls->isComponent = true;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< KeyboardMovement3DComponent >;
		info.cls->members.SetCapacityAndElementCount( 7 );

		info.cls->members[ 0 ].name = "forward";
		info.cls->members[ 0 ].typeName = "Key";
		info.cls->members[ 0 ].typeID = GetTypeID<Key>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( KeyboardMovement3DComponent, KeyboardMovement3DComponent::forward );
		info.cls->members[ 0 ].size = sizeof( Key );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		info.cls->members[ 1 ].name = "back";
		info.cls->members[ 1 ].typeName = "Key";
		info.cls->members[ 1 ].typeID = GetTypeID<Key>();
		info.cls->members[ 1 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 1 ].offset = offsetof( KeyboardMovement3DComponent, KeyboardMovement3DComponent::back );
		info.cls->members[ 1 ].size = sizeof( Key );
		info.cls->members[ 1 ].isStatic = false;
		info.cls->members[ 1 ].isConst = false;
		info.cls->members[ 1 ].isPointer = false;
		info.cls->members[ 1 ].arrayLength = 1;
		info.cls->members[ 1 ].uiSliderMin = v4(0.0);
		info.cls->members[ 1 ].uiSliderMax = v4(0.0);
		info.cls->members[ 1 ].uiSliderSpeed = 0.1;
		info.cls->members[ 1 ].is32BitBool = false;

		info.cls->members[ 2 ].name = "left";
		info.cls->members[ 2 ].typeName = "Key";
		info.cls->members[ 2 ].typeID = GetTypeID<Key>();
		info.cls->members[ 2 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 2 ].offset = offsetof( KeyboardMovement3DComponent, KeyboardMovement3DComponent::left );
		info.cls->members[ 2 ].size = sizeof( Key );
		info.cls->members[ 2 ].isStatic = false;
		info.cls->members[ 2 ].isConst = false;
		info.cls->members[ 2 ].isPointer = false;
		info.cls->members[ 2 ].arrayLength = 1;
		info.cls->members[ 2 ].uiSliderMin = v4(0.0);
		info.cls->members[ 2 ].uiSliderMax = v4(0.0);
		info.cls->members[ 2 ].uiSliderSpeed = 0.1;
		info.cls->members[ 2 ].is32BitBool = false;

		info.cls->members[ 3 ].name = "right";
		info.cls->members[ 3 ].typeName = "Key";
		info.cls->members[ 3 ].typeID = GetTypeID<Key>();
		info.cls->members[ 3 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 3 ].offset = offsetof( KeyboardMovement3DComponent, KeyboardMovement3DComponent::right );
		info.cls->members[ 3 ].size = sizeof( Key );
		info.cls->members[ 3 ].isStatic = false;
		info.cls->members[ 3 ].isConst = false;
		info.cls->members[ 3 ].isPointer = false;
		info.cls->members[ 3 ].arrayLength = 1;
		info.cls->members[ 3 ].uiSliderMin = v4(0.0);
		info.cls->members[ 3 ].uiSliderMax = v4(0.0);
		info.cls->members[ 3 ].uiSliderSpeed = 0.1;
		info.cls->members[ 3 ].is32BitBool = false;

		info.cls->members[ 4 ].name = "sprint";
		info.cls->members[ 4 ].typeName = "Key";
		info.cls->members[ 4 ].typeID = GetTypeID<Key>();
		info.cls->members[ 4 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 4 ].offset = offsetof( KeyboardMovement3DComponent, KeyboardMovement3DComponent::sprint );
		info.cls->members[ 4 ].size = sizeof( Key );
		info.cls->members[ 4 ].isStatic = false;
		info.cls->members[ 4 ].isConst = false;
		info.cls->members[ 4 ].isPointer = false;
		info.cls->members[ 4 ].arrayLength = 1;
		info.cls->members[ 4 ].uiSliderMin = v4(0.0);
		info.cls->members[ 4 ].uiSliderMax = v4(0.0);
		info.cls->members[ 4 ].uiSliderSpeed = 0.1;
		info.cls->members[ 4 ].is32BitBool = false;

		info.cls->members[ 5 ].name = "speed";
		info.cls->members[ 5 ].typeName = "r32";
		info.cls->members[ 5 ].typeID = GetTypeID<r32>();
		info.cls->members[ 5 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 5 ].offset = offsetof( KeyboardMovement3DComponent, KeyboardMovement3DComponent::speed );
		info.cls->members[ 5 ].size = sizeof( r32 );
		info.cls->members[ 5 ].isStatic = false;
		info.cls->members[ 5 ].isConst = false;
		info.cls->members[ 5 ].isPointer = false;
		info.cls->members[ 5 ].arrayLength = 1;
		info.cls->members[ 5 ].uiSliderMin = v4(0.0);
		info.cls->members[ 5 ].uiSliderMax = v4(0.0);
		info.cls->members[ 5 ].uiSliderSpeed = 0.1;
		info.cls->members[ 5 ].is32BitBool = false;

		info.cls->members[ 6 ].name = "speedSprintMultiplier";
		info.cls->members[ 6 ].typeName = "r32";
		info.cls->members[ 6 ].typeID = GetTypeID<r32>();
		info.cls->members[ 6 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 6 ].offset = offsetof( KeyboardMovement3DComponent, KeyboardMovement3DComponent::speedSprintMultiplier );
		info.cls->members[ 6 ].size = sizeof( r32 );
		info.cls->members[ 6 ].isStatic = false;
		info.cls->members[ 6 ].isConst = false;
		info.cls->members[ 6 ].isPointer = false;
		info.cls->members[ 6 ].arrayLength = 1;
		info.cls->members[ 6 ].uiSliderMin = v4(0.0);
		info.cls->members[ 6 ].uiSliderMax = v4(0.0);
		info.cls->members[ 6 ].uiSliderSpeed = 0.1;
		info.cls->members[ 6 ].is32BitBool = false;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < MaterialComponent > () { return 34; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<MaterialComponent>()
	{
		MetadataInfo info;
		info.id = 34;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "MaterialComponent";
		info.cls->baseClassName = "ECSComponent";
		info.cls->baseClassSize = sizeof( ECSComponent );
		info.cls->size = sizeof( MaterialComponent );
		info.cls->isComponent = true;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< MaterialComponent >;
		info.cls->members.SetCapacityAndElementCount( 2 );

		info.cls->members[ 0 ].name = "material";
		info.cls->members[ 0 ].typeName = "MaterialID";
		info.cls->members[ 0 ].typeID = GetTypeID<MaterialID>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( MaterialComponent, MaterialComponent::material );
		info.cls->members[ 0 ].size = sizeof( MaterialID );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		info.cls->members[ 1 ].name = "modifier";
		info.cls->members[ 1 ].typeName = "MaterialModifierID";
		info.cls->members[ 1 ].typeID = GetTypeID<MaterialModifierID>();
		info.cls->members[ 1 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 1 ].offset = offsetof( MaterialComponent, MaterialComponent::modifier );
		info.cls->members[ 1 ].size = sizeof( MaterialModifierID );
		info.cls->members[ 1 ].isStatic = false;
		info.cls->members[ 1 ].isConst = false;
		info.cls->members[ 1 ].isPointer = false;
		info.cls->members[ 1 ].arrayLength = 1;
		info.cls->members[ 1 ].uiSliderMin = v4(0.0);
		info.cls->members[ 1 ].uiSliderMax = v4(0.0);
		info.cls->members[ 1 ].uiSliderSpeed = 0.1;
		info.cls->members[ 1 ].is32BitBool = false;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < ModelComponent > () { return 35; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<ModelComponent>()
	{
		MetadataInfo info;
		info.id = 35;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "ModelComponent";
		info.cls->baseClassName = "ECSComponent";
		info.cls->baseClassSize = sizeof( ECSComponent );
		info.cls->size = sizeof( ModelComponent );
		info.cls->isComponent = true;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< ModelComponent >;
		info.cls->members.SetCapacityAndElementCount( 2 );

		info.cls->members[ 0 ].name = "model";
		info.cls->members[ 0 ].typeName = "ModelID";
		info.cls->members[ 0 ].typeID = GetTypeID<ModelID>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( ModelComponent, ModelComponent::model );
		info.cls->members[ 0 ].size = sizeof( ModelID );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		info.cls->members[ 1 ].name = "wireframe";
		info.cls->members[ 1 ].typeName = "b32";
		info.cls->members[ 1 ].typeID = GetTypeID<b32>();
		info.cls->members[ 1 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 1 ].offset = offsetof( ModelComponent, ModelComponent::wireframe );
		info.cls->members[ 1 ].size = sizeof( b32 );
		info.cls->members[ 1 ].isStatic = false;
		info.cls->members[ 1 ].isConst = false;
		info.cls->members[ 1 ].isPointer = false;
		info.cls->members[ 1 ].arrayLength = 1;
		info.cls->members[ 1 ].uiSliderMin = v4(0.0);
		info.cls->members[ 1 ].uiSliderMax = v4(0.0);
		info.cls->members[ 1 ].uiSliderSpeed = 0.1;
		info.cls->members[ 1 ].is32BitBool = true;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < MouseLookAround3DComponent > () { return 36; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<MouseLookAround3DComponent>()
	{
		MetadataInfo info;
		info.id = 36;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "MouseLookAround3DComponent";
		info.cls->baseClassName = "ECSComponent";
		info.cls->baseClassSize = sizeof( ECSComponent );
		info.cls->size = sizeof( MouseLookAround3DComponent );
		info.cls->isComponent = true;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< MouseLookAround3DComponent >;
		info.cls->members.SetCapacityAndElementCount( 2 );

		info.cls->members[ 0 ].name = "sensitivity";
		info.cls->members[ 0 ].typeName = "r32";
		info.cls->members[ 0 ].typeID = GetTypeID<r32>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( MouseLookAround3DComponent, MouseLookAround3DComponent::sensitivity );
		info.cls->members[ 0 ].size = sizeof( r32 );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		info.cls->members[ 1 ].name = "toggleKey";
		info.cls->members[ 1 ].typeName = "Key";
		info.cls->members[ 1 ].typeID = GetTypeID<Key>();
		info.cls->members[ 1 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 1 ].offset = offsetof( MouseLookAround3DComponent, MouseLookAround3DComponent::toggleKey );
		info.cls->members[ 1 ].size = sizeof( Key );
		info.cls->members[ 1 ].isStatic = false;
		info.cls->members[ 1 ].isConst = false;
		info.cls->members[ 1 ].isPointer = false;
		info.cls->members[ 1 ].arrayLength = 1;
		info.cls->members[ 1 ].uiSliderMin = v4(0.0);
		info.cls->members[ 1 ].uiSliderMax = v4(0.0);
		info.cls->members[ 1 ].uiSliderSpeed = 0.1;
		info.cls->members[ 1 ].is32BitBool = false;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < Transform3DComponent > () { return 37; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<Transform3DComponent>()
	{
		MetadataInfo info;
		info.id = 37;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "Transform3DComponent";
		info.cls->baseClassName = "ECSComponent";
		info.cls->baseClassSize = sizeof( ECSComponent );
		info.cls->size = sizeof( Transform3DComponent );
		info.cls->isComponent = true;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< Transform3DComponent >;
		info.cls->members.SetCapacityAndElementCount( 1 );

		info.cls->members[ 0 ].name = "localTransform";
		info.cls->members[ 0 ].typeName = "Transform3D";
		info.cls->members[ 0 ].typeID = GetTypeID<Transform3D>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( Transform3DComponent, Transform3DComponent::localTransform );
		info.cls->members[ 0 ].size = sizeof( Transform3D );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < Light3DComponent > () { return 38; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<Light3DComponent>()
	{
		MetadataInfo info;
		info.id = 38;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "Light3DComponent";
		info.cls->baseClassName = "ECSComponent";
		info.cls->baseClassSize = sizeof( ECSComponent );
		info.cls->size = sizeof( Light3DComponent );
		info.cls->isComponent = true;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< Light3DComponent >;
		info.cls->members.SetCapacityAndElementCount( 4 );

		info.cls->members[ 0 ].name = "type";
		info.cls->members[ 0 ].typeName = "Light3DType";
		info.cls->members[ 0 ].typeID = GetTypeID<Light3DType>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( Light3DComponent, Light3DComponent::type );
		info.cls->members[ 0 ].size = sizeof( Light3DType );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		info.cls->members[ 1 ].name = "intensity";
		info.cls->members[ 1 ].typeName = "r32";
		info.cls->members[ 1 ].typeID = GetTypeID<r32>();
		info.cls->members[ 1 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 1 ].offset = offsetof( Light3DComponent, Light3DComponent::intensity );
		info.cls->members[ 1 ].size = sizeof( r32 );
		info.cls->members[ 1 ].isStatic = false;
		info.cls->members[ 1 ].isConst = false;
		info.cls->members[ 1 ].isPointer = false;
		info.cls->members[ 1 ].arrayLength = 1;
		info.cls->members[ 1 ].uiSliderMin = v4(0.0);
		info.cls->members[ 1 ].uiSliderMax = v4(0.0);
		info.cls->members[ 1 ].uiSliderSpeed = 0.1;
		info.cls->members[ 1 ].is32BitBool = false;

		info.cls->members[ 2 ].name = "color";
		info.cls->members[ 2 ].typeName = "v3";
		info.cls->members[ 2 ].typeID = GetTypeID<v3>();
		info.cls->members[ 2 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 2 ].offset = offsetof( Light3DComponent, Light3DComponent::color );
		info.cls->members[ 2 ].size = sizeof( v3 );
		info.cls->members[ 2 ].isStatic = false;
		info.cls->members[ 2 ].isConst = false;
		info.cls->members[ 2 ].isPointer = false;
		info.cls->members[ 2 ].arrayLength = 1;
		info.cls->members[ 2 ].uiSliderMin = v4(0.0);
		info.cls->members[ 2 ].uiSliderMax = v4(0.0);
		info.cls->members[ 2 ].uiSliderSpeed = 0.1;
		info.cls->members[ 2 ].is32BitBool = false;

		info.cls->members[ 3 ].name = "attenuation";
		info.cls->members[ 3 ].typeName = "Attenuation";
		info.cls->members[ 3 ].typeID = GetTypeID<Attenuation>();
		info.cls->members[ 3 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 3 ].offset = offsetof( Light3DComponent, Light3DComponent::attenuation );
		info.cls->members[ 3 ].size = sizeof( Attenuation );
		info.cls->members[ 3 ].isStatic = false;
		info.cls->members[ 3 ].isConst = false;
		info.cls->members[ 3 ].isPointer = false;
		info.cls->members[ 3 ].arrayLength = 1;
		info.cls->members[ 3 ].uiSliderMin = v4(0.0);
		info.cls->members[ 3 ].uiSliderMax = v4(0.0);
		info.cls->members[ 3 ].uiSliderSpeed = 0.1;
		info.cls->members[ 3 ].is32BitBool = false;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < Transform2DComponent > () { return 39; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<Transform2DComponent>()
	{
		MetadataInfo info;
		info.id = 39;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "Transform2DComponent";
		info.cls->baseClassName = "ECSComponent";
		info.cls->baseClassSize = sizeof( ECSComponent );
		info.cls->size = sizeof( Transform2DComponent );
		info.cls->isComponent = true;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< Transform2DComponent >;
		info.cls->members.SetCapacityAndElementCount( 1 );

		info.cls->members[ 0 ].name = "localTransform";
		info.cls->members[ 0 ].typeName = "Transform2D";
		info.cls->members[ 0 ].typeID = GetTypeID<Transform2D>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( Transform2DComponent, Transform2DComponent::localTransform );
		info.cls->members[ 0 ].size = sizeof( Transform2D );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < SpriteComponent > () { return 40; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<SpriteComponent>()
	{
		MetadataInfo info;
		info.id = 40;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "SpriteComponent";
		info.cls->baseClassName = "ECSComponent";
		info.cls->baseClassSize = sizeof( ECSComponent );
		info.cls->size = sizeof( SpriteComponent );
		info.cls->isComponent = true;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< SpriteComponent >;
		info.cls->members.SetCapacityAndElementCount( 5 );

		info.cls->members[ 0 ].name = "size";
		info.cls->members[ 0 ].typeName = "v2";
		info.cls->members[ 0 ].typeID = GetTypeID<v2>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( SpriteComponent, SpriteComponent::size );
		info.cls->members[ 0 ].size = sizeof( v2 );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		info.cls->members[ 1 ].name = "color";
		info.cls->members[ 1 ].typeName = "v4";
		info.cls->members[ 1 ].typeID = GetTypeID<v4>();
		info.cls->members[ 1 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 1 ].offset = offsetof( SpriteComponent, SpriteComponent::color );
		info.cls->members[ 1 ].size = sizeof( v4 );
		info.cls->members[ 1 ].isStatic = false;
		info.cls->members[ 1 ].isConst = false;
		info.cls->members[ 1 ].isPointer = false;
		info.cls->members[ 1 ].arrayLength = 1;
		info.cls->members[ 1 ].uiSliderMin = v4(0.0);
		info.cls->members[ 1 ].uiSliderMax = v4(0.0);
		info.cls->members[ 1 ].uiSliderSpeed = 0.1;
		info.cls->members[ 1 ].is32BitBool = false;

		info.cls->members[ 2 ].name = "spriteSheet";
		info.cls->members[ 2 ].typeName = "SpriteSheet";
		info.cls->members[ 2 ].typeID = GetTypeID<SpriteSheet>();
		info.cls->members[ 2 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 2 ].offset = offsetof( SpriteComponent, SpriteComponent::spriteSheet );
		info.cls->members[ 2 ].size = sizeof( SpriteSheet );
		info.cls->members[ 2 ].isStatic = false;
		info.cls->members[ 2 ].isConst = false;
		info.cls->members[ 2 ].isPointer = false;
		info.cls->members[ 2 ].arrayLength = 1;
		info.cls->members[ 2 ].uiSliderMin = v4(0.0);
		info.cls->members[ 2 ].uiSliderMax = v4(0.0);
		info.cls->members[ 2 ].uiSliderSpeed = 0.1;
		info.cls->members[ 2 ].is32BitBool = false;

		info.cls->members[ 3 ].name = "texturePos";
		info.cls->members[ 3 ].typeName = "v2";
		info.cls->members[ 3 ].typeID = GetTypeID<v2>();
		info.cls->members[ 3 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 3 ].offset = offsetof( SpriteComponent, SpriteComponent::texturePos );
		info.cls->members[ 3 ].size = sizeof( v2 );
		info.cls->members[ 3 ].isStatic = false;
		info.cls->members[ 3 ].isConst = false;
		info.cls->members[ 3 ].isPointer = false;
		info.cls->members[ 3 ].arrayLength = 1;
		info.cls->members[ 3 ].uiSliderMin = v4(0.0);
		info.cls->members[ 3 ].uiSliderMax = v4(0.0);
		info.cls->members[ 3 ].uiSliderSpeed = 0.1;
		info.cls->members[ 3 ].is32BitBool = false;

		info.cls->members[ 4 ].name = "offset";
		info.cls->members[ 4 ].typeName = "v2";
		info.cls->members[ 4 ].typeID = GetTypeID<v2>();
		info.cls->members[ 4 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 4 ].offset = offsetof( SpriteComponent, SpriteComponent::offset );
		info.cls->members[ 4 ].size = sizeof( v2 );
		info.cls->members[ 4 ].isStatic = false;
		info.cls->members[ 4 ].isConst = false;
		info.cls->members[ 4 ].isPointer = false;
		info.cls->members[ 4 ].arrayLength = 1;
		info.cls->members[ 4 ].uiSliderMin = v4(0.0);
		info.cls->members[ 4 ].uiSliderMax = v4(0.0);
		info.cls->members[ 4 ].uiSliderSpeed = 0.1;
		info.cls->members[ 4 ].is32BitBool = false;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < SpriteGroupComponent > () { return 41; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<SpriteGroupComponent>()
	{
		MetadataInfo info;
		info.id = 41;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "SpriteGroupComponent";
		info.cls->baseClassName = "ECSComponent";
		info.cls->baseClassSize = sizeof( ECSComponent );
		info.cls->size = sizeof( SpriteGroupComponent );
		info.cls->isComponent = true;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< SpriteGroupComponent >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < GuiClickResponseComponent > () { return 42; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<GuiClickResponseComponent>()
	{
		MetadataInfo info;
		info.id = 42;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "GuiClickResponseComponent";
		info.cls->baseClassName = "ECSComponent";
		info.cls->baseClassSize = sizeof( ECSComponent );
		info.cls->size = sizeof( GuiClickResponseComponent );
		info.cls->isComponent = true;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< GuiClickResponseComponent >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < GuiComponent > () { return 43; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<GuiComponent>()
	{
		MetadataInfo info;
		info.id = 43;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "GuiComponent";
		info.cls->baseClassName = "ECSComponent";
		info.cls->baseClassSize = sizeof( ECSComponent );
		info.cls->size = sizeof( GuiComponent );
		info.cls->isComponent = true;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< GuiComponent >;
		info.cls->members.SetCapacityAndElementCount( 6 );

		info.cls->members[ 0 ].name = "size";
		info.cls->members[ 0 ].typeName = "v2";
		info.cls->members[ 0 ].typeID = GetTypeID<v2>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( GuiComponent, GuiComponent::size );
		info.cls->members[ 0 ].size = sizeof( v2 );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		info.cls->members[ 1 ].name = "color";
		info.cls->members[ 1 ].typeName = "v4";
		info.cls->members[ 1 ].typeID = GetTypeID<v4>();
		info.cls->members[ 1 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 1 ].offset = offsetof( GuiComponent, GuiComponent::color );
		info.cls->members[ 1 ].size = sizeof( v4 );
		info.cls->members[ 1 ].isStatic = false;
		info.cls->members[ 1 ].isConst = false;
		info.cls->members[ 1 ].isPointer = false;
		info.cls->members[ 1 ].arrayLength = 1;
		info.cls->members[ 1 ].uiSliderMin = v4(0.0);
		info.cls->members[ 1 ].uiSliderMax = v4(0.0);
		info.cls->members[ 1 ].uiSliderSpeed = 0.1;
		info.cls->members[ 1 ].is32BitBool = false;

		info.cls->members[ 2 ].name = "text";
		info.cls->members[ 2 ].typeName = "String";
		info.cls->members[ 2 ].typeID = GetTypeID<String>();
		info.cls->members[ 2 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 2 ].offset = offsetof( GuiComponent, GuiComponent::text );
		info.cls->members[ 2 ].size = sizeof( String );
		info.cls->members[ 2 ].isStatic = false;
		info.cls->members[ 2 ].isConst = false;
		info.cls->members[ 2 ].isPointer = false;
		info.cls->members[ 2 ].arrayLength = 1;
		info.cls->members[ 2 ].uiSliderMin = v4(0.0);
		info.cls->members[ 2 ].uiSliderMax = v4(0.0);
		info.cls->members[ 2 ].uiSliderSpeed = 0.1;
		info.cls->members[ 2 ].is32BitBool = false;

		info.cls->members[ 3 ].name = "panel";
		info.cls->members[ 3 ].typeName = "EntityID";
		info.cls->members[ 3 ].typeID = GetTypeID<EntityID>();
		info.cls->members[ 3 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 3 ].offset = offsetof( GuiComponent, GuiComponent::panel );
		info.cls->members[ 3 ].size = sizeof( EntityID );
		info.cls->members[ 3 ].isStatic = false;
		info.cls->members[ 3 ].isConst = false;
		info.cls->members[ 3 ].isPointer = false;
		info.cls->members[ 3 ].arrayLength = 1;
		info.cls->members[ 3 ].uiSliderMin = v4(0.0);
		info.cls->members[ 3 ].uiSliderMax = v4(0.0);
		info.cls->members[ 3 ].uiSliderSpeed = 0.1;
		info.cls->members[ 3 ].is32BitBool = false;

		info.cls->members[ 4 ].name = "offset";
		info.cls->members[ 4 ].typeName = "v2";
		info.cls->members[ 4 ].typeID = GetTypeID<v2>();
		info.cls->members[ 4 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 4 ].offset = offsetof( GuiComponent, GuiComponent::offset );
		info.cls->members[ 4 ].size = sizeof( v2 );
		info.cls->members[ 4 ].isStatic = false;
		info.cls->members[ 4 ].isConst = false;
		info.cls->members[ 4 ].isPointer = false;
		info.cls->members[ 4 ].arrayLength = 1;
		info.cls->members[ 4 ].uiSliderMin = v4(0.0);
		info.cls->members[ 4 ].uiSliderMax = v4(0.0);
		info.cls->members[ 4 ].uiSliderSpeed = 0.1;
		info.cls->members[ 4 ].is32BitBool = false;

		info.cls->members[ 5 ].name = "affectedByScroll";
		info.cls->members[ 5 ].typeName = "b32";
		info.cls->members[ 5 ].typeID = GetTypeID<b32>();
		info.cls->members[ 5 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 5 ].offset = offsetof( GuiComponent, GuiComponent::affectedByScroll );
		info.cls->members[ 5 ].size = sizeof( b32 );
		info.cls->members[ 5 ].isStatic = false;
		info.cls->members[ 5 ].isConst = false;
		info.cls->members[ 5 ].isPointer = false;
		info.cls->members[ 5 ].arrayLength = 1;
		info.cls->members[ 5 ].uiSliderMin = v4(0.0);
		info.cls->members[ 5 ].uiSliderMax = v4(0.0);
		info.cls->members[ 5 ].uiSliderSpeed = 0.1;
		info.cls->members[ 5 ].is32BitBool = true;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < Text2DComponent > () { return 44; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<Text2DComponent>()
	{
		MetadataInfo info;
		info.id = 44;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "Text2DComponent";
		info.cls->baseClassName = "ECSComponent";
		info.cls->baseClassSize = sizeof( ECSComponent );
		info.cls->size = sizeof( Text2DComponent );
		info.cls->isComponent = true;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< Text2DComponent >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < ModelAnimationComponent > () { return 45; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<ModelAnimationComponent>()
	{
		MetadataInfo info;
		info.id = 45;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "ModelAnimationComponent";
		info.cls->baseClassName = "ECSComponent";
		info.cls->baseClassSize = sizeof( ECSComponent );
		info.cls->size = sizeof( ModelAnimationComponent );
		info.cls->isComponent = true;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< ModelAnimationComponent >;
		info.cls->members.SetCapacityAndElementCount( 3 );

		info.cls->members[ 0 ].name = "name";
		info.cls->members[ 0 ].typeName = "String";
		info.cls->members[ 0 ].typeID = GetTypeID<String>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( ModelAnimationComponent, ModelAnimationComponent::name );
		info.cls->members[ 0 ].size = sizeof( String );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		info.cls->members[ 1 ].name = "speed";
		info.cls->members[ 1 ].typeName = "r32";
		info.cls->members[ 1 ].typeID = GetTypeID<r32>();
		info.cls->members[ 1 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 1 ].offset = offsetof( ModelAnimationComponent, ModelAnimationComponent::speed );
		info.cls->members[ 1 ].size = sizeof( r32 );
		info.cls->members[ 1 ].isStatic = false;
		info.cls->members[ 1 ].isConst = false;
		info.cls->members[ 1 ].isPointer = false;
		info.cls->members[ 1 ].arrayLength = 1;
		info.cls->members[ 1 ].uiSliderMin = v4(0.0);
		info.cls->members[ 1 ].uiSliderMax = v4(0.0);
		info.cls->members[ 1 ].uiSliderSpeed = 0.1;
		info.cls->members[ 1 ].is32BitBool = false;

		info.cls->members[ 2 ].name = "currentTime";
		info.cls->members[ 2 ].typeName = "r32";
		info.cls->members[ 2 ].typeID = GetTypeID<r32>();
		info.cls->members[ 2 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 2 ].offset = offsetof( ModelAnimationComponent, ModelAnimationComponent::currentTime );
		info.cls->members[ 2 ].size = sizeof( r32 );
		info.cls->members[ 2 ].isStatic = false;
		info.cls->members[ 2 ].isConst = false;
		info.cls->members[ 2 ].isPointer = false;
		info.cls->members[ 2 ].arrayLength = 1;
		info.cls->members[ 2 ].uiSliderMin = v4(0.0);
		info.cls->members[ 2 ].uiSliderMax = v4(0.0);
		info.cls->members[ 2 ].uiSliderSpeed = 0.1;
		info.cls->members[ 2 ].is32BitBool = false;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < KeyboardMovement2DComponent > () { return 46; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<KeyboardMovement2DComponent>()
	{
		MetadataInfo info;
		info.id = 46;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "KeyboardMovement2DComponent";
		info.cls->baseClassName = "ECSComponent";
		info.cls->baseClassSize = sizeof( ECSComponent );
		info.cls->size = sizeof( KeyboardMovement2DComponent );
		info.cls->isComponent = true;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< KeyboardMovement2DComponent >;
		info.cls->members.SetCapacityAndElementCount( 5 );

		info.cls->members[ 0 ].name = "up";
		info.cls->members[ 0 ].typeName = "Key";
		info.cls->members[ 0 ].typeID = GetTypeID<Key>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( KeyboardMovement2DComponent, KeyboardMovement2DComponent::up );
		info.cls->members[ 0 ].size = sizeof( Key );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		info.cls->members[ 1 ].name = "down";
		info.cls->members[ 1 ].typeName = "Key";
		info.cls->members[ 1 ].typeID = GetTypeID<Key>();
		info.cls->members[ 1 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 1 ].offset = offsetof( KeyboardMovement2DComponent, KeyboardMovement2DComponent::down );
		info.cls->members[ 1 ].size = sizeof( Key );
		info.cls->members[ 1 ].isStatic = false;
		info.cls->members[ 1 ].isConst = false;
		info.cls->members[ 1 ].isPointer = false;
		info.cls->members[ 1 ].arrayLength = 1;
		info.cls->members[ 1 ].uiSliderMin = v4(0.0);
		info.cls->members[ 1 ].uiSliderMax = v4(0.0);
		info.cls->members[ 1 ].uiSliderSpeed = 0.1;
		info.cls->members[ 1 ].is32BitBool = false;

		info.cls->members[ 2 ].name = "left";
		info.cls->members[ 2 ].typeName = "Key";
		info.cls->members[ 2 ].typeID = GetTypeID<Key>();
		info.cls->members[ 2 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 2 ].offset = offsetof( KeyboardMovement2DComponent, KeyboardMovement2DComponent::left );
		info.cls->members[ 2 ].size = sizeof( Key );
		info.cls->members[ 2 ].isStatic = false;
		info.cls->members[ 2 ].isConst = false;
		info.cls->members[ 2 ].isPointer = false;
		info.cls->members[ 2 ].arrayLength = 1;
		info.cls->members[ 2 ].uiSliderMin = v4(0.0);
		info.cls->members[ 2 ].uiSliderMax = v4(0.0);
		info.cls->members[ 2 ].uiSliderSpeed = 0.1;
		info.cls->members[ 2 ].is32BitBool = false;

		info.cls->members[ 3 ].name = "right";
		info.cls->members[ 3 ].typeName = "Key";
		info.cls->members[ 3 ].typeID = GetTypeID<Key>();
		info.cls->members[ 3 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 3 ].offset = offsetof( KeyboardMovement2DComponent, KeyboardMovement2DComponent::right );
		info.cls->members[ 3 ].size = sizeof( Key );
		info.cls->members[ 3 ].isStatic = false;
		info.cls->members[ 3 ].isConst = false;
		info.cls->members[ 3 ].isPointer = false;
		info.cls->members[ 3 ].arrayLength = 1;
		info.cls->members[ 3 ].uiSliderMin = v4(0.0);
		info.cls->members[ 3 ].uiSliderMax = v4(0.0);
		info.cls->members[ 3 ].uiSliderSpeed = 0.1;
		info.cls->members[ 3 ].is32BitBool = false;

		info.cls->members[ 4 ].name = "speed";
		info.cls->members[ 4 ].typeName = "r32";
		info.cls->members[ 4 ].typeID = GetTypeID<r32>();
		info.cls->members[ 4 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 4 ].offset = offsetof( KeyboardMovement2DComponent, KeyboardMovement2DComponent::speed );
		info.cls->members[ 4 ].size = sizeof( r32 );
		info.cls->members[ 4 ].isStatic = false;
		info.cls->members[ 4 ].isConst = false;
		info.cls->members[ 4 ].isPointer = false;
		info.cls->members[ 4 ].arrayLength = 1;
		info.cls->members[ 4 ].uiSliderMin = v4(0.0);
		info.cls->members[ 4 ].uiSliderMax = v4(0.0);
		info.cls->members[ 4 ].uiSliderSpeed = 0.1;
		info.cls->members[ 4 ].is32BitBool = false;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < RigidBodyShapeType > () { return 47; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo< RigidBodyShapeType >()
	{
		MetadataInfo info;
		info.id = 47;
		info.type = METADATA_ENUM;
		info.enm = Eunoia::Metadata::AllocateEnum( true );
		info.enm->name = "RigidBodyShapeType";
		info.enm->values.SetCapacityAndElementCount( 4 );
		info.enm->values[ 0 ].name = "RIGID_BODY_SHAPE_STATIC_PLANE";
		info.enm->values[ 0 ].value = RIGID_BODY_SHAPE_STATIC_PLANE;
		info.enm->values[ 1 ].name = "RIGID_BODY_SHAPE_SPHERE";
		info.enm->values[ 1 ].value = RIGID_BODY_SHAPE_SPHERE;
		info.enm->values[ 2 ].name = "RIGID_BODY_SHAPE_BOX";
		info.enm->values[ 2 ].value = RIGID_BODY_SHAPE_BOX;
		info.enm->values[ 3 ].name = "NUM_RIGID_BODY_SHAPES";
		info.enm->values[ 3 ].value = NUM_RIGID_BODY_SHAPES;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < RigidBody > () { return 48; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<RigidBody>()
	{
		MetadataInfo info;
		info.id = 48;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "RigidBody";
		info.cls->baseClassName = "";
		info.cls->baseClassSize = 0;
		info.cls->size = sizeof( RigidBody );
		info.cls->isComponent = false;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< RigidBody >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < RigidBodyComponent > () { return 49; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<RigidBodyComponent>()
	{
		MetadataInfo info;
		info.id = 49;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "RigidBodyComponent";
		info.cls->baseClassName = "ECSComponent";
		info.cls->baseClassSize = sizeof( ECSComponent );
		info.cls->size = sizeof( RigidBodyComponent );
		info.cls->isComponent = true;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< RigidBodyComponent >;
		info.cls->members.SetCapacityAndElementCount( 2 );

		info.cls->members[ 0 ].name = "body";
		info.cls->members[ 0 ].typeName = "RigidBody";
		info.cls->members[ 0 ].typeID = GetTypeID<RigidBody>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( RigidBodyComponent, RigidBodyComponent::body );
		info.cls->members[ 0 ].size = sizeof( RigidBody );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		info.cls->members[ 1 ].name = "debugDraw";
		info.cls->members[ 1 ].typeName = "b32";
		info.cls->members[ 1 ].typeID = GetTypeID<b32>();
		info.cls->members[ 1 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 1 ].offset = offsetof( RigidBodyComponent, RigidBodyComponent::debugDraw );
		info.cls->members[ 1 ].size = sizeof( b32 );
		info.cls->members[ 1 ].isStatic = false;
		info.cls->members[ 1 ].isConst = false;
		info.cls->members[ 1 ].isPointer = false;
		info.cls->members[ 1 ].arrayLength = 1;
		info.cls->members[ 1 ].uiSliderMin = v4(0.0);
		info.cls->members[ 1 ].uiSliderMax = v4(0.0);
		info.cls->members[ 1 ].uiSliderSpeed = 0.1;
		info.cls->members[ 1 ].is32BitBool = true;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < Gamepad3DComponent > () { return 50; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<Gamepad3DComponent>()
	{
		MetadataInfo info;
		info.id = 50;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "Gamepad3DComponent";
		info.cls->baseClassName = "ECSComponent";
		info.cls->baseClassSize = sizeof( ECSComponent );
		info.cls->size = sizeof( Gamepad3DComponent );
		info.cls->isComponent = true;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< Gamepad3DComponent >;
		info.cls->members.SetCapacityAndElementCount( 7 );

		info.cls->members[ 0 ].name = "gamepad";
		info.cls->members[ 0 ].typeName = "Gamepad";
		info.cls->members[ 0 ].typeID = GetTypeID<Gamepad>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( Gamepad3DComponent, Gamepad3DComponent::gamepad );
		info.cls->members[ 0 ].size = sizeof( Gamepad );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		info.cls->members[ 1 ].name = "speed";
		info.cls->members[ 1 ].typeName = "r32";
		info.cls->members[ 1 ].typeID = GetTypeID<r32>();
		info.cls->members[ 1 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 1 ].offset = offsetof( Gamepad3DComponent, Gamepad3DComponent::speed );
		info.cls->members[ 1 ].size = sizeof( r32 );
		info.cls->members[ 1 ].isStatic = false;
		info.cls->members[ 1 ].isConst = false;
		info.cls->members[ 1 ].isPointer = false;
		info.cls->members[ 1 ].arrayLength = 1;
		info.cls->members[ 1 ].uiSliderMin = v4(0.0);
		info.cls->members[ 1 ].uiSliderMax = v4(0.0);
		info.cls->members[ 1 ].uiSliderSpeed = 0.1;
		info.cls->members[ 1 ].is32BitBool = false;

		info.cls->members[ 2 ].name = "sensitivity";
		info.cls->members[ 2 ].typeName = "r32";
		info.cls->members[ 2 ].typeID = GetTypeID<r32>();
		info.cls->members[ 2 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 2 ].offset = offsetof( Gamepad3DComponent, Gamepad3DComponent::sensitivity );
		info.cls->members[ 2 ].size = sizeof( r32 );
		info.cls->members[ 2 ].isStatic = false;
		info.cls->members[ 2 ].isConst = false;
		info.cls->members[ 2 ].isPointer = false;
		info.cls->members[ 2 ].arrayLength = 1;
		info.cls->members[ 2 ].uiSliderMin = v4(0.0);
		info.cls->members[ 2 ].uiSliderMax = v4(0.0);
		info.cls->members[ 2 ].uiSliderSpeed = 0.1;
		info.cls->members[ 2 ].is32BitBool = false;

		info.cls->members[ 3 ].name = "sprintSpeedMultiplier";
		info.cls->members[ 3 ].typeName = "r32";
		info.cls->members[ 3 ].typeID = GetTypeID<r32>();
		info.cls->members[ 3 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 3 ].offset = offsetof( Gamepad3DComponent, Gamepad3DComponent::sprintSpeedMultiplier );
		info.cls->members[ 3 ].size = sizeof( r32 );
		info.cls->members[ 3 ].isStatic = false;
		info.cls->members[ 3 ].isConst = false;
		info.cls->members[ 3 ].isPointer = false;
		info.cls->members[ 3 ].arrayLength = 1;
		info.cls->members[ 3 ].uiSliderMin = v4(0.0);
		info.cls->members[ 3 ].uiSliderMax = v4(0.0);
		info.cls->members[ 3 ].uiSliderSpeed = 0.1;
		info.cls->members[ 3 ].is32BitBool = false;

		info.cls->members[ 4 ].name = "sprintButton";
		info.cls->members[ 4 ].typeName = "GamepadButton";
		info.cls->members[ 4 ].typeID = GetTypeID<GamepadButton>();
		info.cls->members[ 4 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 4 ].offset = offsetof( Gamepad3DComponent, Gamepad3DComponent::sprintButton );
		info.cls->members[ 4 ].size = sizeof( GamepadButton );
		info.cls->members[ 4 ].isStatic = false;
		info.cls->members[ 4 ].isConst = false;
		info.cls->members[ 4 ].isPointer = false;
		info.cls->members[ 4 ].arrayLength = 1;
		info.cls->members[ 4 ].uiSliderMin = v4(0.0);
		info.cls->members[ 4 ].uiSliderMax = v4(0.0);
		info.cls->members[ 4 ].uiSliderSpeed = 0.1;
		info.cls->members[ 4 ].is32BitBool = false;

		info.cls->members[ 5 ].name = "toggleButton";
		info.cls->members[ 5 ].typeName = "GamepadButton";
		info.cls->members[ 5 ].typeID = GetTypeID<GamepadButton>();
		info.cls->members[ 5 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 5 ].offset = offsetof( Gamepad3DComponent, Gamepad3DComponent::toggleButton );
		info.cls->members[ 5 ].size = sizeof( GamepadButton );
		info.cls->members[ 5 ].isStatic = false;
		info.cls->members[ 5 ].isConst = false;
		info.cls->members[ 5 ].isPointer = false;
		info.cls->members[ 5 ].arrayLength = 1;
		info.cls->members[ 5 ].uiSliderMin = v4(0.0);
		info.cls->members[ 5 ].uiSliderMax = v4(0.0);
		info.cls->members[ 5 ].uiSliderSpeed = 0.1;
		info.cls->members[ 5 ].is32BitBool = false;

		info.cls->members[ 6 ].name = "invertY";
		info.cls->members[ 6 ].typeName = "b32";
		info.cls->members[ 6 ].typeID = GetTypeID<b32>();
		info.cls->members[ 6 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 6 ].offset = offsetof( Gamepad3DComponent, Gamepad3DComponent::invertY );
		info.cls->members[ 6 ].size = sizeof( b32 );
		info.cls->members[ 6 ].isStatic = false;
		info.cls->members[ 6 ].isConst = false;
		info.cls->members[ 6 ].isPointer = false;
		info.cls->members[ 6 ].arrayLength = 1;
		info.cls->members[ 6 ].uiSliderMin = v4(0.0);
		info.cls->members[ 6 ].uiSliderMax = v4(0.0);
		info.cls->members[ 6 ].uiSliderSpeed = 0.1;
		info.cls->members[ 6 ].is32BitBool = true;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < KeyboardLookAround3DComponent > () { return 51; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<KeyboardLookAround3DComponent>()
	{
		MetadataInfo info;
		info.id = 51;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "KeyboardLookAround3DComponent";
		info.cls->baseClassName = "ECSComponent";
		info.cls->baseClassSize = sizeof( ECSComponent );
		info.cls->size = sizeof( KeyboardLookAround3DComponent );
		info.cls->isComponent = true;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< KeyboardLookAround3DComponent >;
		info.cls->members.SetCapacityAndElementCount( 5 );

		info.cls->members[ 0 ].name = "up";
		info.cls->members[ 0 ].typeName = "Key";
		info.cls->members[ 0 ].typeID = GetTypeID<Key>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( KeyboardLookAround3DComponent, KeyboardLookAround3DComponent::up );
		info.cls->members[ 0 ].size = sizeof( Key );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		info.cls->members[ 1 ].name = "down";
		info.cls->members[ 1 ].typeName = "Key";
		info.cls->members[ 1 ].typeID = GetTypeID<Key>();
		info.cls->members[ 1 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 1 ].offset = offsetof( KeyboardLookAround3DComponent, KeyboardLookAround3DComponent::down );
		info.cls->members[ 1 ].size = sizeof( Key );
		info.cls->members[ 1 ].isStatic = false;
		info.cls->members[ 1 ].isConst = false;
		info.cls->members[ 1 ].isPointer = false;
		info.cls->members[ 1 ].arrayLength = 1;
		info.cls->members[ 1 ].uiSliderMin = v4(0.0);
		info.cls->members[ 1 ].uiSliderMax = v4(0.0);
		info.cls->members[ 1 ].uiSliderSpeed = 0.1;
		info.cls->members[ 1 ].is32BitBool = false;

		info.cls->members[ 2 ].name = "left";
		info.cls->members[ 2 ].typeName = "Key";
		info.cls->members[ 2 ].typeID = GetTypeID<Key>();
		info.cls->members[ 2 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 2 ].offset = offsetof( KeyboardLookAround3DComponent, KeyboardLookAround3DComponent::left );
		info.cls->members[ 2 ].size = sizeof( Key );
		info.cls->members[ 2 ].isStatic = false;
		info.cls->members[ 2 ].isConst = false;
		info.cls->members[ 2 ].isPointer = false;
		info.cls->members[ 2 ].arrayLength = 1;
		info.cls->members[ 2 ].uiSliderMin = v4(0.0);
		info.cls->members[ 2 ].uiSliderMax = v4(0.0);
		info.cls->members[ 2 ].uiSliderSpeed = 0.1;
		info.cls->members[ 2 ].is32BitBool = false;

		info.cls->members[ 3 ].name = "right";
		info.cls->members[ 3 ].typeName = "Key";
		info.cls->members[ 3 ].typeID = GetTypeID<Key>();
		info.cls->members[ 3 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 3 ].offset = offsetof( KeyboardLookAround3DComponent, KeyboardLookAround3DComponent::right );
		info.cls->members[ 3 ].size = sizeof( Key );
		info.cls->members[ 3 ].isStatic = false;
		info.cls->members[ 3 ].isConst = false;
		info.cls->members[ 3 ].isPointer = false;
		info.cls->members[ 3 ].arrayLength = 1;
		info.cls->members[ 3 ].uiSliderMin = v4(0.0);
		info.cls->members[ 3 ].uiSliderMax = v4(0.0);
		info.cls->members[ 3 ].uiSliderSpeed = 0.1;
		info.cls->members[ 3 ].is32BitBool = false;

		info.cls->members[ 4 ].name = "sensitivity";
		info.cls->members[ 4 ].typeName = "r32";
		info.cls->members[ 4 ].typeID = GetTypeID<r32>();
		info.cls->members[ 4 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 4 ].offset = offsetof( KeyboardLookAround3DComponent, KeyboardLookAround3DComponent::sensitivity );
		info.cls->members[ 4 ].size = sizeof( r32 );
		info.cls->members[ 4 ].isStatic = false;
		info.cls->members[ 4 ].isConst = false;
		info.cls->members[ 4 ].isPointer = false;
		info.cls->members[ 4 ].arrayLength = 1;
		info.cls->members[ 4 ].uiSliderMin = v4(0.0);
		info.cls->members[ 4 ].uiSliderMax = v4(0.0);
		info.cls->members[ 4 ].uiSliderSpeed = 0.1;
		info.cls->members[ 4 ].is32BitBool = false;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < Camera2DComponent > () { return 52; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<Camera2DComponent>()
	{
		MetadataInfo info;
		info.id = 52;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "Camera2DComponent";
		info.cls->baseClassName = "ECSComponent";
		info.cls->baseClassSize = sizeof( ECSComponent );
		info.cls->size = sizeof( Camera2DComponent );
		info.cls->isComponent = true;
		info.cls->isSystem = false;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< Camera2DComponent >;
		info.cls->members.SetCapacityAndElementCount( 1 );

		info.cls->members[ 0 ].name = "orthoScale";
		info.cls->members[ 0 ].typeName = "r32";
		info.cls->members[ 0 ].typeID = GetTypeID<r32>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( Camera2DComponent, Camera2DComponent::orthoScale );
		info.cls->members[ 0 ].size = sizeof( r32 );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < KeyboardMovement3DSystem > () { return 53; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<KeyboardMovement3DSystem>()
	{
		MetadataInfo info;
		info.id = 53;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "KeyboardMovement3DSystem";
		info.cls->baseClassName = "ECSSystem";
		info.cls->baseClassSize = sizeof( ECSSystem );
		info.cls->size = sizeof( KeyboardMovement3DSystem );
		info.cls->isComponent = false;
		info.cls->isSystem = true;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< KeyboardMovement3DSystem >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < ModelSubmissionSystem > () { return 54; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<ModelSubmissionSystem>()
	{
		MetadataInfo info;
		info.id = 54;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "ModelSubmissionSystem";
		info.cls->baseClassName = "ECSSystem";
		info.cls->baseClassSize = sizeof( ECSSystem );
		info.cls->size = sizeof( ModelSubmissionSystem );
		info.cls->isComponent = false;
		info.cls->isSystem = true;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< ModelSubmissionSystem >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < MouseLookAround3DSystem > () { return 55; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<MouseLookAround3DSystem>()
	{
		MetadataInfo info;
		info.id = 55;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "MouseLookAround3DSystem";
		info.cls->baseClassName = "ECSSystem";
		info.cls->baseClassSize = sizeof( ECSSystem );
		info.cls->size = sizeof( MouseLookAround3DSystem );
		info.cls->isComponent = false;
		info.cls->isSystem = true;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< MouseLookAround3DSystem >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < ViewProjectionSystem > () { return 56; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<ViewProjectionSystem>()
	{
		MetadataInfo info;
		info.id = 56;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "ViewProjectionSystem";
		info.cls->baseClassName = "ECSSystem";
		info.cls->baseClassSize = sizeof( ECSSystem );
		info.cls->size = sizeof( ViewProjectionSystem );
		info.cls->isComponent = false;
		info.cls->isSystem = true;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< ViewProjectionSystem >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < LightSubmissionSystem > () { return 57; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<LightSubmissionSystem>()
	{
		MetadataInfo info;
		info.id = 57;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "LightSubmissionSystem";
		info.cls->baseClassName = "ECSSystem";
		info.cls->baseClassSize = sizeof( ECSSystem );
		info.cls->size = sizeof( LightSubmissionSystem );
		info.cls->isComponent = false;
		info.cls->isSystem = true;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< LightSubmissionSystem >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < SpriteSubmissionSystem > () { return 58; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<SpriteSubmissionSystem>()
	{
		MetadataInfo info;
		info.id = 58;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "SpriteSubmissionSystem";
		info.cls->baseClassName = "ECSSystem";
		info.cls->baseClassSize = sizeof( ECSSystem );
		info.cls->size = sizeof( SpriteSubmissionSystem );
		info.cls->isComponent = false;
		info.cls->isSystem = true;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< SpriteSubmissionSystem >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < TransformHierarchy3DSystem > () { return 59; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<TransformHierarchy3DSystem>()
	{
		MetadataInfo info;
		info.id = 59;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "TransformHierarchy3DSystem";
		info.cls->baseClassName = "ECSSystem";
		info.cls->baseClassSize = sizeof( ECSSystem );
		info.cls->size = sizeof( TransformHierarchy3DSystem );
		info.cls->isComponent = false;
		info.cls->isSystem = true;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< TransformHierarchy3DSystem >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < TransformHierarchy2DSystem > () { return 60; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<TransformHierarchy2DSystem>()
	{
		MetadataInfo info;
		info.id = 60;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "TransformHierarchy2DSystem";
		info.cls->baseClassName = "ECSSystem";
		info.cls->baseClassSize = sizeof( ECSSystem );
		info.cls->size = sizeof( TransformHierarchy2DSystem );
		info.cls->isComponent = false;
		info.cls->isSystem = true;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< TransformHierarchy2DSystem >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < GuiSystem > () { return 61; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<GuiSystem>()
	{
		MetadataInfo info;
		info.id = 61;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "GuiSystem";
		info.cls->baseClassName = "ECSSystem";
		info.cls->baseClassSize = sizeof( ECSSystem );
		info.cls->size = sizeof( GuiSystem );
		info.cls->isComponent = false;
		info.cls->isSystem = true;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< GuiSystem >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < Text2DSubmissionSystem > () { return 62; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<Text2DSubmissionSystem>()
	{
		MetadataInfo info;
		info.id = 62;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "Text2DSubmissionSystem";
		info.cls->baseClassName = "ECSSystem";
		info.cls->baseClassSize = sizeof( ECSSystem );
		info.cls->size = sizeof( Text2DSubmissionSystem );
		info.cls->isComponent = false;
		info.cls->isSystem = true;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< Text2DSubmissionSystem >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < ModelAnimationSystem > () { return 63; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<ModelAnimationSystem>()
	{
		MetadataInfo info;
		info.id = 63;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "ModelAnimationSystem";
		info.cls->baseClassName = "ECSSystem";
		info.cls->baseClassSize = sizeof( ECSSystem );
		info.cls->size = sizeof( ModelAnimationSystem );
		info.cls->isComponent = false;
		info.cls->isSystem = true;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< ModelAnimationSystem >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < PhysicsSystem > () { return 64; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<PhysicsSystem>()
	{
		MetadataInfo info;
		info.id = 64;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "PhysicsSystem";
		info.cls->baseClassName = "ECSSystem";
		info.cls->baseClassSize = sizeof( ECSSystem );
		info.cls->size = sizeof( PhysicsSystem );
		info.cls->isComponent = false;
		info.cls->isSystem = true;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< PhysicsSystem >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < Gamepad3DSystem > () { return 65; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<Gamepad3DSystem>()
	{
		MetadataInfo info;
		info.id = 65;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "Gamepad3DSystem";
		info.cls->baseClassName = "ECSSystem";
		info.cls->baseClassSize = sizeof( ECSSystem );
		info.cls->size = sizeof( Gamepad3DSystem );
		info.cls->isComponent = false;
		info.cls->isSystem = true;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< Gamepad3DSystem >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < KeyboardLookAround3DSystem > () { return 66; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<KeyboardLookAround3DSystem>()
	{
		MetadataInfo info;
		info.id = 66;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "KeyboardLookAround3DSystem";
		info.cls->baseClassName = "ECSSystem";
		info.cls->baseClassSize = sizeof( ECSSystem );
		info.cls->size = sizeof( KeyboardLookAround3DSystem );
		info.cls->isComponent = false;
		info.cls->isSystem = true;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< KeyboardLookAround3DSystem >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < KeyboardMovement2DSystem > () { return 67; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<KeyboardMovement2DSystem>()
	{
		MetadataInfo info;
		info.id = 67;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "KeyboardMovement2DSystem";
		info.cls->baseClassName = "ECSSystem";
		info.cls->baseClassSize = sizeof( ECSSystem );
		info.cls->size = sizeof( KeyboardMovement2DSystem );
		info.cls->isComponent = false;
		info.cls->isSystem = true;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< KeyboardMovement2DSystem >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < ViewProjection2DSystem > () { return 68; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<ViewProjection2DSystem>()
	{
		MetadataInfo info;
		info.id = 68;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "ViewProjection2DSystem";
		info.cls->baseClassName = "ECSSystem";
		info.cls->baseClassSize = sizeof( ECSSystem );
		info.cls->size = sizeof( ViewProjection2DSystem );
		info.cls->isComponent = false;
		info.cls->isSystem = true;
		info.cls->isEvent = false;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< ViewProjection2DSystem >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < GuiElementOnClickEvent > () { return 69; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<GuiElementOnClickEvent>()
	{
		MetadataInfo info;
		info.id = 69;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "GuiElementOnClickEvent";
		info.cls->baseClassName = "ECSEvent";
		info.cls->baseClassSize = sizeof( ECSEvent );
		info.cls->size = sizeof( GuiElementOnClickEvent );
		info.cls->isComponent = false;
		info.cls->isSystem = false;
		info.cls->isEvent = true;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< GuiElementOnClickEvent >;
		info.cls->members.SetCapacityAndElementCount( 4 );

		info.cls->members[ 0 ].name = "elementClicked";
		info.cls->members[ 0 ].typeName = "EntityID";
		info.cls->members[ 0 ].typeID = GetTypeID<EntityID>();
		info.cls->members[ 0 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 0 ].offset = offsetof( GuiElementOnClickEvent, GuiElementOnClickEvent::elementClicked );
		info.cls->members[ 0 ].size = sizeof( EntityID );
		info.cls->members[ 0 ].isStatic = false;
		info.cls->members[ 0 ].isConst = false;
		info.cls->members[ 0 ].isPointer = false;
		info.cls->members[ 0 ].arrayLength = 1;
		info.cls->members[ 0 ].uiSliderMin = v4(0.0);
		info.cls->members[ 0 ].uiSliderMax = v4(0.0);
		info.cls->members[ 0 ].uiSliderSpeed = 0.1;
		info.cls->members[ 0 ].is32BitBool = false;

		info.cls->members[ 1 ].name = "button";
		info.cls->members[ 1 ].typeName = "MouseButton";
		info.cls->members[ 1 ].typeID = GetTypeID<MouseButton>();
		info.cls->members[ 1 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 1 ].offset = offsetof( GuiElementOnClickEvent, GuiElementOnClickEvent::button );
		info.cls->members[ 1 ].size = sizeof( MouseButton );
		info.cls->members[ 1 ].isStatic = false;
		info.cls->members[ 1 ].isConst = false;
		info.cls->members[ 1 ].isPointer = false;
		info.cls->members[ 1 ].arrayLength = 1;
		info.cls->members[ 1 ].uiSliderMin = v4(0.0);
		info.cls->members[ 1 ].uiSliderMax = v4(0.0);
		info.cls->members[ 1 ].uiSliderSpeed = 0.1;
		info.cls->members[ 1 ].is32BitBool = false;

		info.cls->members[ 2 ].name = "pos";
		info.cls->members[ 2 ].typeName = "v2";
		info.cls->members[ 2 ].typeID = GetTypeID<v2>();
		info.cls->members[ 2 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 2 ].offset = offsetof( GuiElementOnClickEvent, GuiElementOnClickEvent::pos );
		info.cls->members[ 2 ].size = sizeof( v2 );
		info.cls->members[ 2 ].isStatic = false;
		info.cls->members[ 2 ].isConst = false;
		info.cls->members[ 2 ].isPointer = false;
		info.cls->members[ 2 ].arrayLength = 1;
		info.cls->members[ 2 ].uiSliderMin = v4(0.0);
		info.cls->members[ 2 ].uiSliderMax = v4(0.0);
		info.cls->members[ 2 ].uiSliderSpeed = 0.1;
		info.cls->members[ 2 ].is32BitBool = false;

		info.cls->members[ 3 ].name = "enabled";
		info.cls->members[ 3 ].typeName = "b32";
		info.cls->members[ 3 ].typeID = GetTypeID<b32>();
		info.cls->members[ 3 ].accessModifier = METADATA_ACCESS_MODIFIER_PUBLIC;
		info.cls->members[ 3 ].offset = offsetof( GuiElementOnClickEvent, GuiElementOnClickEvent::enabled );
		info.cls->members[ 3 ].size = sizeof( b32 );
		info.cls->members[ 3 ].isStatic = false;
		info.cls->members[ 3 ].isConst = false;
		info.cls->members[ 3 ].isPointer = false;
		info.cls->members[ 3 ].arrayLength = 1;
		info.cls->members[ 3 ].uiSliderMin = v4(0.0);
		info.cls->members[ 3 ].uiSliderMax = v4(0.0);
		info.cls->members[ 3 ].uiSliderSpeed = 0.1;
		info.cls->members[ 3 ].is32BitBool = false;

		return info;
	}

	template<>
	EU_API metadata_typeid Metadata::GetTypeID < RigidBodyTransformModifiedEvent > () { return 70; }

	template<>
	MetadataInfo Metadata::ConstructMetadataInfo<RigidBodyTransformModifiedEvent>()
	{
		MetadataInfo info;
		info.id = 70;
		info.type = METADATA_CLASS;
		info.cls = Eunoia::Metadata::AllocateClass( true );
		info.cls->name = "RigidBodyTransformModifiedEvent";
		info.cls->baseClassName = "ECSEvent";
		info.cls->baseClassSize = sizeof( ECSEvent );
		info.cls->size = sizeof( RigidBodyTransformModifiedEvent );
		info.cls->isComponent = false;
		info.cls->isSystem = false;
		info.cls->isEvent = true;
		info.cls->DefaultConstructor = Eunoia::MetadataCreateInstance< RigidBodyTransformModifiedEvent >;
		info.cls->members.SetCapacityAndElementCount( 0 );

		return info;
	}
	const metadata_typeid Metadata::LastEngineTypeID = 70;

	void Metadata::InitMetadataInfos()
	{
		RegisterMetadataInfo( ConstructMetadataInfo< String >() );
		RegisterMetadataInfo( ConstructMetadataInfo< Gamepad >() );
		RegisterMetadataInfo( ConstructMetadataInfo< Key >() );
		RegisterMetadataInfo( ConstructMetadataInfo< MouseButton >() );
		RegisterMetadataInfo( ConstructMetadataInfo< GamepadButton >() );
		RegisterMetadataInfo( ConstructMetadataInfo< GamepadTrigger >() );
		RegisterMetadataInfo( ConstructMetadataInfo< GamepadThumbstick >() );
		RegisterMetadataInfo( ConstructMetadataInfo< v2 >() );
		RegisterMetadataInfo( ConstructMetadataInfo< v3 >() );
		RegisterMetadataInfo( ConstructMetadataInfo< v4 >() );
		RegisterMetadataInfo( ConstructMetadataInfo< m3 >() );
		RegisterMetadataInfo( ConstructMetadataInfo< m4 >() );
		RegisterMetadataInfo( ConstructMetadataInfo< quat >() );
		RegisterMetadataInfo( ConstructMetadataInfo< Transform2D >() );
		RegisterMetadataInfo( ConstructMetadataInfo< Transform3D >() );
		RegisterMetadataInfo( ConstructMetadataInfo< SpriteSheet >() );
		RegisterMetadataInfo( ConstructMetadataInfo< Light3DType >() );
		RegisterMetadataInfo( ConstructMetadataInfo< Attenuation >() );
		RegisterMetadataInfo( ConstructMetadataInfo< ShadowInfo >() );
		RegisterMetadataInfo( ConstructMetadataInfo< Light3D >() );
		RegisterMetadataInfo( ConstructMetadataInfo< ECSComponent >() );
		RegisterMetadataInfo( ConstructMetadataInfo< ECSEvent >() );
		RegisterMetadataInfo( ConstructMetadataInfo< ECSSystem >() );
		RegisterMetadataInfo( ConstructMetadataInfo< CameraComponent >() );
		RegisterMetadataInfo( ConstructMetadataInfo< KeyboardMovement3DComponent >() );
		RegisterMetadataInfo( ConstructMetadataInfo< MaterialComponent >() );
		RegisterMetadataInfo( ConstructMetadataInfo< ModelComponent >() );
		RegisterMetadataInfo( ConstructMetadataInfo< MouseLookAround3DComponent >() );
		RegisterMetadataInfo( ConstructMetadataInfo< Transform3DComponent >() );
		RegisterMetadataInfo( ConstructMetadataInfo< Light3DComponent >() );
		RegisterMetadataInfo( ConstructMetadataInfo< Transform2DComponent >() );
		RegisterMetadataInfo( ConstructMetadataInfo< SpriteComponent >() );
		RegisterMetadataInfo( ConstructMetadataInfo< SpriteGroupComponent >() );
		RegisterMetadataInfo( ConstructMetadataInfo< GuiClickResponseComponent >() );
		RegisterMetadataInfo( ConstructMetadataInfo< GuiComponent >() );
		RegisterMetadataInfo( ConstructMetadataInfo< Text2DComponent >() );
		RegisterMetadataInfo( ConstructMetadataInfo< ModelAnimationComponent >() );
		RegisterMetadataInfo( ConstructMetadataInfo< KeyboardMovement2DComponent >() );
		RegisterMetadataInfo( ConstructMetadataInfo< RigidBodyShapeType >() );
		RegisterMetadataInfo( ConstructMetadataInfo< RigidBody >() );
		RegisterMetadataInfo( ConstructMetadataInfo< RigidBodyComponent >() );
		RegisterMetadataInfo( ConstructMetadataInfo< Gamepad3DComponent >() );
		RegisterMetadataInfo( ConstructMetadataInfo< KeyboardLookAround3DComponent >() );
		RegisterMetadataInfo( ConstructMetadataInfo< Camera2DComponent >() );
		RegisterMetadataInfo( ConstructMetadataInfo< KeyboardMovement3DSystem >() );
		RegisterMetadataInfo( ConstructMetadataInfo< ModelSubmissionSystem >() );
		RegisterMetadataInfo( ConstructMetadataInfo< MouseLookAround3DSystem >() );
		RegisterMetadataInfo( ConstructMetadataInfo< ViewProjectionSystem >() );
		RegisterMetadataInfo( ConstructMetadataInfo< LightSubmissionSystem >() );
		RegisterMetadataInfo( ConstructMetadataInfo< SpriteSubmissionSystem >() );
		RegisterMetadataInfo( ConstructMetadataInfo< TransformHierarchy3DSystem >() );
		RegisterMetadataInfo( ConstructMetadataInfo< TransformHierarchy2DSystem >() );
		RegisterMetadataInfo( ConstructMetadataInfo< GuiSystem >() );
		RegisterMetadataInfo( ConstructMetadataInfo< Text2DSubmissionSystem >() );
		RegisterMetadataInfo( ConstructMetadataInfo< ModelAnimationSystem >() );
		RegisterMetadataInfo( ConstructMetadataInfo< PhysicsSystem >() );
		RegisterMetadataInfo( ConstructMetadataInfo< Gamepad3DSystem >() );
		RegisterMetadataInfo( ConstructMetadataInfo< KeyboardLookAround3DSystem >() );
		RegisterMetadataInfo( ConstructMetadataInfo< KeyboardMovement2DSystem >() );
		RegisterMetadataInfo( ConstructMetadataInfo< ViewProjection2DSystem >() );
		RegisterMetadataInfo( ConstructMetadataInfo< GuiElementOnClickEvent >() );
		RegisterMetadataInfo( ConstructMetadataInfo< RigidBodyTransformModifiedEvent >() );
	}

}