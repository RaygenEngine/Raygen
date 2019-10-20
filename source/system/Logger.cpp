#include "pch/pch.h"

#include "system/Logger.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/ostream_sink.h"

namespace utl {
std::shared_ptr<spdlog::logger> Log::s_logger;
std::stringstream Log::s_editorLogStream;

void Log::Init(LogLevelTarget level)
{
	spdlog::set_pattern("%^[%T] %n: %v%$");
	// colored multi-threaded
	s_logger = spdlog::stdout_color_mt("Rayxen");

	auto editorOssSink = std::make_shared<spdlog::sinks::ostream_sink_mt>(s_editorLogStream);
	editorOssSink->set_pattern("[%T.%e] %L:\t%v");
	s_logger->sinks().push_back(editorOssSink);


	switch (level) {
		case LogLevelTarget::TRACE: s_logger->set_level(spdlog::level::level_enum::trace); break;

		case LogLevelTarget::DEBUG: s_logger->set_level(spdlog::level::level_enum::debug); break;

		case LogLevelTarget::INFO: s_logger->set_level(spdlog::level::level_enum::info); break;

		case LogLevelTarget::WARN: s_logger->set_level(spdlog::level::level_enum::warn); break;

		case LogLevelTarget::ERR: s_logger->set_level(spdlog::level::level_enum::err); break;

		case LogLevelTarget::CRITICAL: s_logger->set_level(spdlog::level::level_enum::critical); break;

		case LogLevelTarget::OFF:
		default: s_logger->set_level(spdlog::level::level_enum::off); break;
	}

	LOG_INFO("Initialized Rayxen Logger, level: {}", spdlog::level::to_string_view(s_logger->level()));
}
} // namespace utl
