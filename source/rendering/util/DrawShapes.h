#pragma once

#include "rendering/wrappers/Buffer.h"

namespace rvk {

struct ShapeDataInfo {
	vl::RBuffer* vertexBuffer;
	uint32 vertexCount;
	vl::RBuffer* indexBuffer;
	uint32 indexCount;
};

struct Shapes {
	static void InitShapes();
	static void DeinitShapes();
};

// TODO: request cache based on shape, primitive, winding, etc

void bindCubeLines(vk::CommandBuffer cmdBuffer);
void drawCubeLines(vk::CommandBuffer cmdBuffer);

void bindUnitRectTriangleStrip(vk::CommandBuffer cmdBuffer);
void drawUnitRectTriangleStrip(vk::CommandBuffer cmdBuffer);

ShapeDataInfo getUnitRectTriangleListInfo();
void bindUnitRectTriangleList(vk::CommandBuffer cmdBuffer);
void drawUnitRectTriangleList(vk::CommandBuffer cmdBuffer);

void bindCube(vk::CommandBuffer cmdBuffer);
void drawCube(vk::CommandBuffer cmdBuffer);

void bindSphere18x9(vk::CommandBuffer cmdBuffer);
void drawSphere18x9(vk::CommandBuffer cmdBuffer);

void bindSphere36x18(vk::CommandBuffer cmdBuffer);
void drawSphere36x18(vk::CommandBuffer cmdBuffer);


} // namespace rvk
