#pragma once

#include <string>

// Base class for the custom and default "Game" class.
// Game class provides overrides for the most important classes, functions and settings of the base engine.


inline class App_ {
public:
	fs::path workingDirPath{ "assets" };

	// The following paths are relative to workingDirPath
	fs::path fileDialogPath{ "." };
	fs::path templateScene{ "engine-data/default.json" };
	// If left empty, template Scene gets used as primary scene
	fs::path localScene{};

	std::string windowTitle{ "Raygen" };

	glm::ivec2 windowSize{ 2304, 1296 };

protected:
	int32 argc{ 1 };
	char** argv{ nullptr };

public:
	App_();

	virtual ~App_();

public:
	virtual void PreMainInit(int32 argc_, char* argv_[]);
	virtual int32 Main(int32 argc_, char* argv_[]);

	virtual void MainLoop();

	virtual void WhileResizing();

	friend class Engine_;
} * App{ nullptr };
