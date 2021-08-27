#pragma once

#include <string>

// Base class for the custom and default "Game" class.
// Game class provides overrides for the most important classes, functions and settings of the base engine.


inline class App_ {

	fs::path m_workingDirPath{ "assets" };

	// The following paths are relative to workingDirPath
	fs::path m_fileDialogPath{ "." };
	fs::path m_templateScene{ "engine-data/default.json" };
	// If left empty, template Scene gets used as primary scene
	fs::path m_localScene{};

	std::string m_windowTitle{ "Raygen" };

	glm::uvec2 m_windowSize{ 1920u, 1080u };

protected:
	int32 m_argc{ 1 };
	char** m_argv{ nullptr };

public:
	App_();

	virtual ~App_();

public:
	virtual void PreMainInit(int32 argc_, char* argv_[]);
	virtual int32 Main(int32 argc_, char* argv_[]);

	virtual void MainLoop();

	virtual void WhileResizing();


	std::string GetWindowTitle() const { return m_windowTitle; }
	fs::path GetWorkingDirPath() const { return m_workingDirPath; }
	fs::path GetFileDialogPath() const { return m_fileDialogPath; }
	fs::path GetTemplateScene() const { return m_templateScene; }
	fs::path GetLocalScene() const { return m_localScene; }
	glm::uvec2 GetWindowSize() const { return m_windowSize; }

} * App{ nullptr };
