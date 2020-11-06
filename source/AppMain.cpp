#include "App.h"

#include "engine/Logger.h"

int32 main(int32 argc, char* argv[])
{
	// Init logger (global access, not engine, app or window bound)
	Log.Init(LogLevel::Info);

	App_ app;
	app.PreMainInit(argc, argv);
	return app.Main(argc, argv);
}
