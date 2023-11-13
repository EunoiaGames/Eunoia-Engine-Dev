#include "FileUtils.h"
#include <filesystem>

#ifdef EU_PLATFORM_WINDOWS
#include <Windows.h>
#endif


String FileUtils::LoadTextFile(const String& path, b32* loaded)
{
	//String p = "../Eunoia-Engine/" + path; //TEMPORARY
	String p = path;

	FILE* file = fopen(p.C_Str(), "rb");
	if (!file)
	{
		if(loaded)
			*loaded = false;
		return String("Could not find file") + path;
	}

	fseek(file, 0, SEEK_END);
	mem_size size = ftell(file);
	fseek(file, 0, SEEK_SET);
	char* characters = (char*)malloc(size + 1);
	fread(characters, 1, size, file);
	characters[size] = 0;
	String string(characters);
	free(characters);
	fclose(file);
	if(loaded)
		*loaded = true;
	return string;
}

u8* FileUtils::LoadBinaryFile(const String& path, mem_size* fileSize, b32* loaded)
{
	FILE* file = fopen(path.C_Str(), "rb");
	if (!file)
	{
		if (loaded)
			*loaded = false;
		return 0;
	}

	fseek(file, 0, SEEK_END);
	mem_size size = ftell(file);
	if (fileSize)
		*fileSize = size;
	fseek(file, 0, SEEK_SET);
	u8* bytes = (u8*)malloc(size);
	fread(bytes, 1, size, file);
	fclose(file);
	if (loaded)
		*loaded = true;
	return bytes;
}

String FileUtils::LoadTextFileWithIncludePreProcessor(const String& path)
{
	//String p = "../Eunoia-Engine/" + path; //TEMPORARY
	String p = path;

	std::ifstream file;
	file.open(p.C_Str());

	s32 lastSlashIndex = p.FindLastOf("/");
	String relPath = lastSlashIndex == -1 ? "" : p.SubString(0, lastSlashIndex);

	if (!file.good())
	{
		return String("Could not find file") + p;
	}

	String contents;

	char ln[4096];
	while (file.getline(ln, 4096))
	{
		String line = ln;
		line = line.Trim();
		if (line.BeginsWith("#include"))
		{
			s32 openQuote = line.FindFirstOf("\"");
			s32 closeQuote = line.FindLastOf("\"");
			if (openQuote != -1 && closeQuote != -1 && openQuote != closeQuote)
			{
				String includeFile = line.SubString(openQuote + 1, closeQuote - 1);
				includeFile = includeFile.Trim();
				String includePath = relPath + includeFile;
				contents += LoadTextFileWithIncludePreProcessor(includePath);
			}
		}
		else
		{
			contents += (line + "\n");
		}
	}

	file.close();

	return contents;
}

void FileUtils::WriteFile(const String& path, const String& text)
{
	FILE* file = fopen(path.C_Str(), "w");
	fwrite(text.C_Str(), 1, text.Length(), file);
	fclose(file);
}

void FileUtils::WriteFileBinary(const String& path, const u8* bytes, u32 numBytes)
{
	FILE* file = fopen(path.C_Str(), "wb");
	fwrite(bytes, 1, numBytes, file);
	fclose(file);
}
