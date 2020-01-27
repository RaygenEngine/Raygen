#include "pch/pch.h"

#include "AppBase.h"
#include "system/Logger.h"

#include <iostream>

int32 main(int32 argc, char* argv[])
{
	std::cout << "Hello Test!\n";
	// Init logger (global access, not engine, app or window bound)
	LOGGER_INIT(LogLevelTarget::INFO);
	App app;
	app.PreMainInit(argc, argv);
	return app.Main(argc, argv);
}
