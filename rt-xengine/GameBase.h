#ifndef GAMEBASE_H
#define GAMEBASE_H

// Base class for the custom and default "Game" class.
// Game class provides overrides for the most important classes, functions and settings of the base engine.

#include "world/NodeFactory.h"
#include "renderer/renderers/opengl/test/GLTestRenderer.h"
#include "system/Engine.h"


class GameBase
{
public:
	virtual World::NodeFactory* MakeNodeFactory()
	{
		return new World::NodeFactory();
	}

	virtual std::string GetInitScene()
	{
		return "test.xcsn";
	}

	virtual System::Engine* MakeEngine()
	{
		return new System::Engine();
	}

	virtual Renderer::Renderer* MakeRenderer(System::Engine* engineContext)
	{
		return new Renderer::OpenGL::GLTestRenderer(engineContext);
	}

	virtual std::string GetGameName()
	{
		return "Default Engine";
	}

	//virtual int Main(int argc, char* argv[]) 
	//{
	//
	//}
};


#endif //GAMEBASE_H