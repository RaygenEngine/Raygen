#include "Rendering.h"

#include "rendering/Layer.h"

void Rendering::Init()
{
	Layer = new Layer_();
}

void Rendering::Destroy()
{
	delete Layer;
}

void Rendering::DrawFrame()
{
	Layer->DrawFrame();
}
