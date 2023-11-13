#pragma once

#include "LoadedMaterial.h"

namespace Eunoia {

	enum EumtlLoadError
	{
		EUMTL_LOAD_ERROR_NOT_FOUND,
		EUMTL_LOAD_ERROR_NOT_EUMTL_FORMAT,
		EUMTL_LOAD_ERROR_UNSUPPORTED_VERSION,
		EUMTL_LOAD_SUCCESS
	};

	class EU_API MaterialLoader
	{
	public:
		static EumtlLoadError LoadEumtlMaterial(const String& path, LoadedMaterialFile* loadedMaterialFile);
		static void WriteEumtlMaterial(const String& path, const LoadedMaterialFile& data);
	};

}