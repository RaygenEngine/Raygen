#include "pch.h"
#include "Rendering.h"

#include "rendering/Layer.h"

using namespace vl;

void Rendering::Init()
{
	Layer = new S_Layer();
}

void Rendering::Destroy()
{
	delete Layer;
}

void Rendering::DrawFrame()
{
	Layer->DrawFrame();
}
