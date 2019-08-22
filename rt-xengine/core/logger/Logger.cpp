#include "pch.h"
#include "Logger.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Core
{
	std::shared_ptr<spdlog::logger> Log::s_logger;

	void Log::Init(LogLevelTarget level)
	{
		spdlog::set_pattern("%^[%T] %n: %v%$"); 
		// colored multi-threaded
		s_logger = spdlog::stdout_color_mt("RT-XENGINE");

		switch(level)
		{
			case LLT_TRACE:
				s_logger->set_level(spdlog::level::level_enum::trace);
				break;

			case LLT_DEBUG:
				s_logger->set_level(spdlog::level::level_enum::debug);
				break;

			case LLT_INFO:
				s_logger->set_level(spdlog::level::level_enum::info);
				break;

			case LLT_WARN:
				s_logger->set_level(spdlog::level::level_enum::warn);
				break;

			case LLT_ERROR:
				s_logger->set_level(spdlog::level::level_enum::err);
				break;

			case LLT_CRITICAL:
				s_logger->set_level(spdlog::level::level_enum::critical);
				break;

			case LLT_OFF: default:
				s_logger->set_level(spdlog::level::level_enum::off);
				break;
		}

		RT_XENGINE_LOG_AT_LOWEST_LEVEL("Initialized RT_XEngine Logger, level: {}",
			spdlog::level::to_string_view(s_logger->level()));
	}
}
