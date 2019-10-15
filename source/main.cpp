#include "pch/pch.h"

#include "App.h"
#include "system/Logger.h"

int32 main(int32 argc, char* argv[])
{
	// Init logger (global access, not engine, app or window bound)
	LOGGER_INIT(LogLevelTarget::INFO);
	App app;
	app.PreMainInit(argc, argv);
	return app.Main(argc, argv);
}
