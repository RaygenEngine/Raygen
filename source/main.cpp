// This is a custom entry point for the engine (you can build your own using any library and API)
// However, the following core tools and logic should be programmed in the same order/fashion
// 1) at the very start perform a call to RT_XENGINE_LOGGER_INIT(..);
// 2) create the engine and engine->InitDirectories(..) given appropriate asset directory name (use argv[0] for
// application dir) 3) load a world using the engine->CreateWorldFromFile(..) (path to scene xml file) 4) use the
// renderer registration system found inside the engine container to register one or more renderers (and obtain their
// registration indices) 5) start the renderer by performing the following calls in the order below (you can use &&
// short circuiting as those are all bool functions)
//    a) engine->SwitchRenderer(registrationIndex); (call requires registered renderers)
//    b) renderer->InitRendering(..); (call may require window target - currently general Renderer abstraction has a
//    virtual Windows-based function) c) renderer->InitScene(..); (call requires world)
// 6) (suggestion) before the mainloop it is wise to unload all the disk assets (main memory) (however if you want to
// switch fast between renderers, don't 7) main loop should be generally constructed as follows:
//    a) input.ClearSoftState() (clear soft state of the input devices)
//    b) handle events - update input devices state, calls to WindowResize of EngineObjects (always world first)
//    c) registered renderer switching logic (e.g. use XVK_TAB to switch between the renderers)
//    d) world->Update() (first, its updates most likely will affect the renderer result
//    e) renderer->Update()
//    f) renderer->Render

// Note: if your work does not require dynamic updates, is not resource intensive, etc. you can avoid some of the steps
// shown above

#include "pch/pch.h"
#include "App.h"

int32 main(int32 argc, char* argv[])
{
	// Init logger (global access, not engine, app or window bound)
	LOGGER_INIT(LogLevelTarget::INFO);
	App app;
	app.PreMainInit(argc, argv);
	return app.Main(argc, argv);
}
