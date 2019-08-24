#ifndef GAME_H
#define GAME_H

#include "world/NodeFactory.h"

//
// This file is included by the engine to allow for per 'game' overrides for specific pointer implementations
// and other per app settings. You can replace what these functions return to change them.
// 
//

static World::NodeFactory* CreateGameNodeFactory() 
{
	RT_XENGINE_LOG_FATAL("called from game-default/game.h");
	return new World::NodeFactory();
}

#endif //GAME_H