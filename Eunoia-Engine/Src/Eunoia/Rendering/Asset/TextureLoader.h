#pragma once

#include "../../DataStructures/String.h"

namespace Eunoia {

	class EU_API TextureLoader
	{
	public:
		static u8* LoadEutexTexture(const String& path, u32* width, u32* height);
		static void FreeEutexTexture(u8* pixels);
	};

}