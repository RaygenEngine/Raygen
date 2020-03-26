#include "pch.h"

#include "App.h"
#include "engine/Logger.h"
#include "universe/nodes/Node.h"

int32 main(int32 argc, char* argv[])
{
	// Init logger (global access, not engine, app or window bound)
	Log.Init(LogLevel::Info);

	App app;
	app.PreMainInit(argc, argv);
	return app.Main(argc, argv);
}
