#pragma once

#include "rendering/wrappers/Buffer.h"

namespace rvk {


struct Shapes {
	static void InitShapes();
	static void DeinitShapes();
};

void bindCubeLines(vk::CommandBuffer cmdBuffer);
void drawCubeLines(vk::CommandBuffer cmdBuffer);

void bindUnitRect(vk::CommandBuffer cmdBuffer);
void drawUnitRect(vk::CommandBuffer cmdBuffer);

void bindCube(vk::CommandBuffer cmdBuffer);
void drawCube(vk::CommandBuffer cmdBuffer);

void bindSphere18x9(vk::CommandBuffer cmdBuffer);
void drawSphere18x9(vk::CommandBuffer cmdBuffer);

void bindSphere36x18(vk::CommandBuffer cmdBuffer);
void drawSphere36x18(vk::CommandBuffer cmdBuffer);


} // namespace rvk
