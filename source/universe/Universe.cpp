#include "pch.h"
#include "Universe.h"

void Universe::Init()
{
	MainWorld = new World(new NodeFactory());
}

void Universe::Destroy()
{
	delete MainWorld;
}
