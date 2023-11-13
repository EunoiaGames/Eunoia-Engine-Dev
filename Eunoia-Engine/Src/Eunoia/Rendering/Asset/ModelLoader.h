#pragma once

#include "LoadedModel.h"
#include "AssetTypeIDs.h"

namespace Eunoia {

	enum EumdlLoadError
	{
		EUMDL_LOAD_ERROR_NOT_FOUND,
		EUMDL_LOAD_ERROR_NOT_EUMDL_FORMAT,
		EUMDL_LOAD_ERROR_UNSUPPORTED_EUMDL_VERSION,
		EUMDL_LOAD_ERROR_INVALID_VERTEX_OR_INDEX_COUNT,
		EUMDL_LOAD_ERROR_EXTRA_DATA_NOT_PARSED,
		EUMDL_LOAD_SUCCESS
	};

	class EU_API ModelLoader
	{
	public:
		static EumdlLoadError LoadEumdlModel(const String& path, LoadedModel* loadedModel);
	};

}
