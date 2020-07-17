#pragma once
#include "core/Types.h"

#include <string>

// Base class for the custom and default "Game" class.
// Game class provides overrides for the most important classes, functions and settings of the base engine.

class NodeFactory;

class App {
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

public:
	App();

	virtual ~App() = default;

public:
	virtual void PreMainInit(int32 argc, char* argv[]);
	virtual int32 Main(int32 argc, char* argv[]);

	virtual void MainLoop();

	virtual void WhileResizing();

	// Return a 'new NodeFactory()' subtype of node factory with your own factory
	virtual NodeFactory* MakeNodeFactory();

	friend class Engine_;
};
