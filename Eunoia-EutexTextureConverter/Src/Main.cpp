#define STB_IMAGE_IMPLEMENTATION
#include "Vendor\stb_image\stb_image.h"
#include <cstring>

typedef unsigned int u32;

struct eutex_metadata
{
	u32 width;
	u32 height;
};

int main(int argc, char** argv)
{
	if (argc == 1)
		return 1;

	char Header[6] = "eutex";

	char NewPath[512];
	for (u32 i = 1; i < argc; i++)
	{
		char* Path = argv[i];
		int Width, Height;
		stbi_uc* Pixels = stbi_load(Path, &Width, &Height, 0, 4);

		u32 Index = 0;
		while (Path[Index] != '.')
		{
			NewPath[Index] = Path[Index];
			Index++;
		}

		strcpy(&NewPath[Index], ".eutex");
		NewPath[Index + 6] = 0;

		FILE* File = fopen(NewPath, "wb");
		if (!File)
		{
			return 1;
		}

		eutex_metadata Metadata;
		Metadata.width = Width;
		Metadata.height = Height;

		fwrite(Header, 1, 5, File);
		fwrite(&Metadata, sizeof(Metadata), 1, File);
		fwrite(Pixels, sizeof(stbi_uc), Width * Height * 4, File);
		fclose(File);
	}
}