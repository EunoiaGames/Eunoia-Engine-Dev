#include "Log.h"
#include "../../Vendor/spdlog/sinks/stdout_color_sinks.h"

namespace Eunoia {

	std::shared_ptr<spdlog::logger> Logger::s_EngineLogger;
	std::shared_ptr<spdlog::logger> Logger::s_AppLogger;
	std::shared_ptr<spdlog::logger> Logger::s_EditorLogger;

	void Logger::Init()
	{
#ifndef EU_DIST
		spdlog::set_pattern("%^[%T] %n: %v%$");
		
		s_EngineLogger = spdlog::stdout_color_mt("EUNOIA");
		s_EngineLogger->set_level(spdlog::level::trace);
		s_AppLogger = spdlog::stdout_color_mt("APP");
		s_AppLogger->set_level(spdlog::level::trace);
		s_EditorLogger = spdlog::stdout_color_mt("EDITOR");
		s_EditorLogger->set_level(spdlog::level::trace);
		EU_LOG_INFO("Logger initialized");
#endif
	}

}