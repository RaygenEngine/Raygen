#include "pch/pch.h"

#include "system/Logger.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/ostream_sink.h"


std::stringstream Log::s_editorLogStream;
std::shared_ptr<spdlog::logger> g_logger;

void Log::Init(LogLevelTarget level)
{
	if (!g_logger) {
		BasicSetup();
	}

	switch (level) {
		case LogLevelTarget::TRACE: g_logger->set_level(spdlog::level::level_enum::trace); break;

		case LogLevelTarget::DEBUG: g_logger->set_level(spdlog::level::level_enum::debug); break;

		case LogLevelTarget::INFO: g_logger->set_level(spdlog::level::level_enum::info); break;

		case LogLevelTarget::WARN: g_logger->set_level(spdlog::level::level_enum::warn); break;

		case LogLevelTarget::ERR: g_logger->set_level(spdlog::level::level_enum::err); break;

		case LogLevelTarget::CRITICAL: g_logger->set_level(spdlog::level::level_enum::critical); break;

		case LogLevelTarget::OFF:
		default: g_logger->set_level(spdlog::level::level_enum::off); break;
	}

	LOG_INFO("Raygen Logger level: {}", spdlog::level::to_string_view(g_logger->level()));
}

void Log::EarlyInit()
{
	if (!g_logger) {
		BasicSetup();
		g_logger->set_level(spdlog::level::level_enum::warn);
	}
}

void Log::Debug(const std::string& str)
{
	g_logger->debug(str);
}

void Log::Info(const std::string& str)
{
	g_logger->info(str);
}

void Log::Warn(const std::string& str)
{
	g_logger->warn(str);
}

void Log::Error(const std::string& str)
{
	g_logger->error(str);
}

void Log::Critical(const std::string& str)
{
	g_logger->critical(str);
}

void Log::Flush()
{
	g_logger->flush();
}

void Log::BasicSetup()
{
	spdlog::set_pattern("%^[%T] %n: %v%$");
	// colored multi-threaded
	g_logger = spdlog::stdout_color_mt("Raygen");

	auto editorOssSink = std::make_shared<spdlog::sinks::ostream_sink_mt>(s_editorLogStream);
	editorOssSink->set_pattern("[%T.%e] %L:\t%v");
	g_logger->sinks().push_back(editorOssSink);
}
