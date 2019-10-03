#pragma once

// Base class for the custom and default "Game" class.
// Game class provides overrides for the most important classes, functions and settings of the base engine.

#include "world/NodeFactory.h"
#include "system/Engine.h"
#include "platform/Window.h"

class AppBase
{
protected:
	std::string m_name;
	std::string m_initialScene;
	std::string m_assetPath;


	std::string m_windowTitle;
	int32 m_windowHeight;
	int32 m_windowWidth;

	bool m_handleControllers;

	bool m_lockMouse;

	int32 m_argc;
	char** m_argv;

	bool m_enableEditor;
public:
	AppBase();

	virtual ~AppBase() = default;

public:
	virtual void PreMainInit(int32 argc, char* argv[]);
	virtual int32 Main(int32 argc, char* argv[]);

	virtual void MainLoop();
	
	// Override here to register your custom renderer
	virtual void RegisterRenderers();

	// Return a 'new Win32Window()' with your parameters
	virtual Win32Window* CreateAppWindow();

	// Return a 'new NodeFactory()' subtype of node factory with your own factory
	virtual NodeFactory* MakeNodeFactory();

	friend class Engine;
};
