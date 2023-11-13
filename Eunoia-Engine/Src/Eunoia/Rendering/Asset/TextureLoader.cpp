#include "TextureLoader.h"
#include "../../Utils/Log.h"

namespace Eunoia {

	struct EutexMetadata
	{
		u32 width;
		u32 height;
	};

	u8* TextureLoader::LoadEutexTexture(const String& path, u32* width, u32* height)
	{
		String p = path;
		FILE* file = fopen(p.C_Str(), "rb");
		char header[5];
		fread(header, 1, 5, file);
		if (!(header[0] == 'e' && header[1] == 'u' && header[2] == 't' && header[3] == 'e' && header[4] == 'x'))
		{
			EU_LOG_WARN("This is not a eutex format");
			return 0;
		}
		EutexMetadata metadata;
		fread(&metadata, sizeof(EutexMetadata), 1, file);
		if (metadata.width == 0 || metadata.height == 0)
			return 0;

		*width = metadata.width;
		*height = metadata.height;

		u8* pixels = (u8*)malloc(metadata.width * metadata.height * 4);
		fread(pixels, 1, metadata.width * metadata.height * 4, file);
		fclose(file);

		return pixels;
	}

	void TextureLoader::FreeEutexTexture(u8* pixels)
	{
		free(pixels);
	}

}
