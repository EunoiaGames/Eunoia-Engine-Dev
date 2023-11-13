#pragma once

#include "../Common.h"
#include <memory>
#include "../../Vendor/spdlog/spdlog.h"

#ifndef EU_DIST
#ifdef EU_ENGINE
#define EU_LOG_TRACE(...) Eunoia::Logger::GetEngineLogger()->trace(__VA_ARGS__)
#define EU_LOG_INFO(...) Eunoia::Logger::GetEngineLogger()->info(__VA_ARGS__)
#define EU_LOG_WARN(...) Eunoia::Logger::GetEngineLogger()->warn(__VA_ARGS__)
#define EU_LOG_ERROR(...) Eunoia::Logger::GetEngineLogger()->error(__VA_ARGS__)
#define EU_LOG_FATAL(...) Eunoia::Logger::GetEngineLogger()->critical(__VA_ARGS__)
#elif defined EU_PROJECT
#define EU_LOG_TRACE(...) Eunoia::Logger::GetAppLogger()->trace(__VA_ARGS__)
#define EU_LOG_INFO(...) Eunoia::Logger::GetAppLogger()->info(__VA_ARGS__)
#define EU_LOG_WARN(...) Eunoia::Logger::GetAppLogger()->warn(__VA_ARGS__)
#define EU_LOG_ERROR(...) Eunoia::Logger::GetAppLogger()->error(__VA_ARGS__)
#define EU_LOG_FATAL(...) Eunoia::Logger::GetAppLogger()->critical(__VA_ARGS__)
#elif defined EU_EDITOR
#define EU_LOG_TRACE(...) Eunoia::Logger::GetEditorLogger()->trace(__VA_ARGS__)
#define EU_LOG_INFO(...) Eunoia::Logger::GetEditorLogger()->info(__VA_ARGS__)
#define EU_LOG_WARN(...) Eunoia::Logger::GetEditorLogger()->warn(__VA_ARGS__)
#define EU_LOG_ERROR(...) Eunoia::Logger::GetEditorLogger()->error(__VA_ARGS__)
#define EU_LOG_FATAL(...) Eunoia::Logger::GetEditorLogger()->critical(__VA_ARGS__)
#endif
#else
#define EU_LOG_TRACE
#define EU_LOG_INFO
#define EU_LOG_WARN
#define EU_LOG_ERROR
#define EU_LOG_FATAL
#endif

namespace Eunoia {

	class EU_API Logger
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetEngineLogger() { return s_EngineLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetAppLogger() { return s_AppLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetEditorLogger() { return s_EditorLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_EngineLogger;
		static std::shared_ptr<spdlog::logger> s_AppLogger;
		static std::shared_ptr<spdlog::logger> s_EditorLogger;
	};

}

