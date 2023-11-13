#pragma once

#include <fstream>
#include <iostream>
#include <stdio.h>
#include "String.h"


class FileUtils
{
public:
	static String LoadTextFile(const String& path, b32* loaded = 0);
	static u8* LoadBinaryFile(const String& path, mem_size* fileSize = 0, b32* loaded = 0);
	static String LoadTextFileWithIncludePreProcessor(const String& path);

	static void WriteFile(const String& path, const String& text);
	static void WriteFileBinary(const String& path, const u8* bytes, u32 numBytes);
};
