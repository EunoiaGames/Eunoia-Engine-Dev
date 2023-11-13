#pragma once

#include <fstream>
#include <iostream>
#include <stdio.h>
#include "../DataStructures/String.h"
#include "../DataStructures/List.h"

namespace Eunoia {

	class EUDirectory;
	struct EU_API EUFile
	{
		EUFile();
		void Rename(const String& name);
		void Delete() const;

		String LoadText(b32 includePreProcessor = false);
		u8* LoadBinary();

		EUDirectory* parent;
		String name;
		String path;
		String extension;
		void* userData;
	};

	class EU_API EUDirectory
	{
	public:
		EUDirectory(const String& dir, EUDirectory* parent = 0);
		~EUDirectory();

		void Refresh();

		void CreateNewDir(const String& name);
		void CreateNewFile(const String& name);

		void MoveDirectory(u32 dTarget, u32 dSource);
		void MoveFileIntoDir(u32 dTarget, u32 fSource);

		void Rename(const String& name);
		void Delete();

		String name;
		String path;
		List<EUFile> files;
		List<EUDirectory*> directories;
		EUDirectory* parentDirectory;
	private:
		void InitFilesAndDirs();
	};

	class EU_API FileSystem
	{
	public:
		static void Init(const String& workingDir);
		static EUDirectory* GetAssetDir();

		static const String& GetWorkingDir();
	private:
		static String s_WorkingDir;
		static EUDirectory* s_AssetDir;
	};

	class EU_API FileUtils
	{
	public:
		static String LoadTextFile(const String& path, b32* loaded = 0);
		static u8* LoadBinaryFile(const String& path, mem_size* fileSize = 0, b32* loaded = 0);
		static String LoadTextFileWithIncludePreProcessor(const String& path);

		static void RenameFileOrDirectory(const String& oldName, const String& newName);

		static void EUCreateDirectory(const String& path);
		static void EUCreateFile(const String& path);

		static void EUDeletDirectory(const String& path);
		static void EUDeleteFile(const String& path);

		static void WriteFile(const String& path, const String& text);
		static void WriteFileBinary(const String& path, const u8* bytes, u32 numBytes);
	};
}