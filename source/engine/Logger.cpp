#include "pch.h"

#include "engine/Logger.h"

#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>


std::shared_ptr<spdlog::logger> g_logger;

void S_Log::Init(LogLevel level)
{
	if (!g_logger) {
		BasicSetup();
	}

	switch (level) {
		case LogLevel::Trace: g_logger->set_level(spdlog::level::level_enum::trace); break;
		case LogLevel::Debug: g_logger->set_level(spdlog::level::level_enum::debug); break;
		case LogLevel::Info: g_logger->set_level(spdlog::level::level_enum::info); break;
		case LogLevel::Warn: g_logger->set_level(spdlog::level::level_enum::warn); break;
		case LogLevel::Error: g_logger->set_level(spdlog::level::level_enum::err); break;
		case LogLevel::Critical: g_logger->set_level(spdlog::level::level_enum::critical); break;
		case LogLevel::Off:
		default: g_logger->set_level(spdlog::level::level_enum::off); break;
	}

	LOG_INFO("Raygen Logger level: {}", spdlog::level::to_string_view(g_logger->level()));
}

void S_Log::EarlyInit()
{
	if (!g_logger) {
		BasicSetup();
		g_logger->set_level(spdlog::level::level_enum::warn);
	}
}

void S_Log::Debug(const std::string& str)
{
	g_logger->debug(str);
}

void S_Log::Info(const std::string& str)
{
	g_logger->info(str);
}

void S_Log::Warn(const std::string& str)
{
	g_logger->warn(str);
}

void S_Log::Error(const std::string& str)
{
	g_logger->error(str);
}

void S_Log::Critical(const std::string& str)
{
	g_logger->critical(str);
}

void S_Log::Flush()
{
	g_logger->flush();
}

void S_Log::BasicSetup()
{
	spdlog::set_pattern("%^[%T] %n: %v%$");
	// colored multi-threaded
	g_logger = spdlog::stdout_color_mt("Raygen");

	auto editorOssSink = std::make_shared<spdlog::sinks::ostream_sink_mt>(s_editorLogStream);
	editorOssSink->set_pattern("[%T.%e] %L:\t%v");
	g_logger->sinks().push_back(editorOssSink);
}
