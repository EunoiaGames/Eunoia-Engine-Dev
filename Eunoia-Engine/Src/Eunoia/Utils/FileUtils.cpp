#include "FileUtils.h"
#include "Log.h"
#include <filesystem>

#ifdef EU_PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace Eunoia {

	String FileUtils::LoadTextFile(const String& path, b32* loaded)
	{
		//String p = "../Eunoia-Engine/" + path; //TEMPORARY
		String p = path;

		FILE* file = fopen(p.C_Str(), "rb");
		if (!file)
		{
			EU_LOG_WARN((String("Could not find file: ") + path).C_Str());
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
			EU_LOG_WARN((String("Could not find file: ") + path).C_Str());
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
			EU_LOG_WARN((String("Could not find file: ") + p).C_Str());
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

	void FileUtils::RenameFileOrDirectory(const String& oldName, const String& newName)
	{
		rename(oldName.C_Str(), newName.C_Str());
	}

	void FileUtils::EUCreateDirectory(const String& path)
	{
#ifdef EU_PLATFORM_WINDOWS
		CreateDirectoryA(path.C_Str(), 0);
#endif
	}

	void FileUtils::EUCreateFile(const String& path)
	{
		FILE* file = fopen(path.C_Str(), "w");
		fclose(file);
	}

	void FileUtils::EUDeletDirectory(const String& path)
	{
		
	}

	void FileUtils::EUDeleteFile(const String& path)
	{
		remove(path.C_Str());
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

	EUDirectory::EUDirectory(const String& dir, EUDirectory* parent)
	{
		path = dir;
		name = dir.SubString(dir.FindLastOf("/") + 1);
		parentDirectory = parent;

		InitFilesAndDirs();
	}

	EUDirectory::~EUDirectory()
	{
		for (u32 i = 0; i < directories.Size(); i++)
		{
			delete directories[i];
		}
	}

	void EUDirectory::Refresh()
	{
		for (u32 i = 0; i < directories.Size(); i++)
		{
			delete directories[i];
		}

		files.Clear();
		directories.Clear();
		InitFilesAndDirs();
	}

	void EUDirectory::CreateNewDir(const String& name)
	{
		directories.Push(new EUDirectory(path + "/" + name, this));
	}

	void EUDirectory::CreateNewFile(const String& name)
	{
		String filePath = path + "/" + name;
		fclose(fopen(filePath.C_Str(), "wb"));

		EUFile file;
		file.name = name;
		file.path = filePath;
		file.extension = name.SubString(name.FindLastOf(".") + 1);
		file.parent = this;
		files.Push(file);
	}

	void EUDirectory::MoveDirectory(u32 dTarget, u32 dSource)
	{
		rename(directories[dSource]->path.C_Str(), (directories[dTarget]->path + "/" + directories[dSource]->name).C_Str());

		delete directories[dSource];
		directories.Remove(dSource);
		directories[dTarget]->Refresh();
	}

	void EUDirectory::MoveFileIntoDir(u32 dTarget, u32 fSource)
	{
		rename(files[fSource].path.C_Str(), (directories[dTarget]->path + "/" + files[fSource].name).C_Str());
		files.Remove(fSource);
		directories[dTarget]->Refresh();
	}

	void EUDirectory::Rename(const String& name)
	{
		String newDir = path.SubString(0, path.FindLastOf("/")) + name;
		rename(path.C_Str(), newDir.C_Str());

		this->path = newDir;
		this->name = name;
	}

	void EUDirectory::Delete()
	{
	}

	void EUDirectory::InitFilesAndDirs()
	{
		wchar_t dir_[256];
		std::mbstowcs(dir_, path.C_Str(), 256);
		CreateDirectory(dir_, 0);

		std::filesystem::directory_iterator it(path.C_Str());
		for (const auto& entry : it)
		{
			if (entry.is_directory())
			{
				String subDirPath = entry.path().generic_string().c_str();
				EUDirectory* dir = new EUDirectory(subDirPath, this);
				directories.Push(dir);
			}
			else
			{
				EUFile file;
				file.path = entry.path().generic_string().c_str();
				file.name = file.path.SubString(file.path.FindLastOf("/") + 1);
				file.extension = file.name.SubString(file.name.FindLastOf(".") + 1);
				file.parent = this;
				files.Push(file);
			}
		}
	}
	
	EUFile::EUFile() :
		userData(EU_NULL)
	{
	}

	void EUFile::Rename(const String& name)
	{
		String newFile = path.SubString(0, path.FindLastOf("/")) + name;
		rename(path.C_Str(), newFile.C_Str());

		this->path = newFile;
		this->name = name;
	}

	void EUFile::Delete() const
	{
		remove(path.C_Str());
		for (u32 i = 0; i < parent->files.Size(); i++)
		{
			if (parent->files[i].path == path)
			{
				parent->files.Remove(i);
				break;
			}
		}

	}

	String EUFile::LoadText(b32 includePreProcessor)
	{
		if (includePreProcessor)
			return FileUtils::LoadTextFileWithIncludePreProcessor(path);
		else
			return FileUtils::LoadTextFile(path);
	}

	u8* EUFile::LoadBinary()
	{
		return FileUtils::LoadBinaryFile(path);
	}

	String FileSystem::s_WorkingDir;
	EUDirectory* FileSystem::s_AssetDir;

	void FileSystem::Init(const String& workingDir)
	{
		s_WorkingDir = workingDir;
		s_AssetDir = new EUDirectory("Assets");
	}

	EUDirectory* FileSystem::GetAssetDir()
	{
		return s_AssetDir;
	}

	const String& FileSystem::GetWorkingDir()
	{
		return s_WorkingDir;
	}

}
