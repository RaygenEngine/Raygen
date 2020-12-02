#include "DrawShapes.h"

// PERF:
namespace {

// clang-format off

	static std::vector<float> cube_triangleStripData = {
		-1.f,  1.f,  1.f,  // Front-top-left
		 1.f,  1.f,  1.f,  // Front-top-right
		-1.f, -1.f,  1.f,  // Front-bottom-left
		 1.f, -1.f,  1.f,  // Front-bottom-right
		 1.f, -1.f, -1.f,  // Back-bottom-right
		 1.f,  1.f,  1.f,  // Front-top-right
		 1.f,  1.f, -1.f,  // Back-top-right
		-1.f,  1.f,  1.f,  // Front-top-left
		-1.f,  1.f, -1.f,  // Back-top-left
		-1.f, -1.f,  1.f,  // Front-bottom-left
		-1.f, -1.f, -1.f,  // Back-bottom-left
		 1.f, -1.f, -1.f,  // Back-bottom-right
		-1.f,  1.f, -1.f,  // Back-top-left
		 1.f,  1.f, -1.f,  // Back-top-right
	};


	static std::vector<float> cube_verticesData = {
		-1.f,  1.f,  1.f,  // Front-top-left
		 1.f,  1.f,  1.f,  // Front-top-right
		-1.f, -1.f,  1.f,  // Front-bottom-left
		 1.f, -1.f,  1.f,  // Front-bottom-right
		-1.f,  1.f, -1.f,  // Back-top-left
		 1.f,  1.f, -1.f,  // Back-top-right
		-1.f, -1.f, -1.f,  // Back-bottom-left
		 1.f, -1.f, -1.f,  // Back-bottom-right
	};

	static std::vector<uint32> cube_lineList_indicesData = {
		0u, 1u, // ftl - ftr
		2u, 3u, // fbl - fbr
		4u, 5u, // btl - btr
		6u, 7u, // bbl - bbr
		1u, 5u, // ftr - btr
		0u, 4u, // ftl - btl
		3u, 7u, // fbr - bbr
		2u, 6u, // fbl - bbl
		0u, 2u, // ftl - fbl
		1u, 3u, // ftr - fbr
		4u, 6u, // btl - bbl
		5u, 7u, // btr - bbr
	};

	static std::vector<float> unitRectangle_triangleStripData = {
	  -0.5f,  0.5f,  0.0f,
	  -0.5f, -0.5f,  0.0f,
	   0.5f,  0.5f,  0.0f,
	   0.5f, -0.5f,  0.0f,
	};

// clang-format on

struct IndexedShapeBuffer {
	vl::RBuffer vertexBuffer;
	vl::RBuffer indexBuffer;
	uint32 indexCount;

	IndexedShapeBuffer() = default;
	IndexedShapeBuffer(const std::string& name, std::vector<float>& vertexData, std::vector<uint32>& indexData)
	{
		vertexBuffer = vl::RBuffer::CreateTransfer(name.c_str(), vertexData, vk::BufferUsageFlagBits::eVertexBuffer);
		indexBuffer = vl::RBuffer::CreateTransfer(name.c_str(), indexData, vk::BufferUsageFlagBits::eIndexBuffer);
		indexCount = indexData.size();
	}
};

struct ShapeBuffer {
	vl::RBuffer vertexBuffer;
	uint32 vertexCount;

	ShapeBuffer() = default;
	ShapeBuffer(const std::string& name, std::vector<float>& data)
	{
		vertexBuffer = vl::RBuffer::CreateTransfer(name.c_str(), data, vk::BufferUsageFlagBits::eVertexBuffer);
		vertexCount = static_cast<uint32>(data.size() / 3);
	}
};

void BindShapeBuffer(vk::CommandBuffer cmdBuffer, const IndexedShapeBuffer& b)
{
	cmdBuffer.bindVertexBuffers(0u, b.vertexBuffer.handle(), vk::DeviceSize(0));
	cmdBuffer.bindIndexBuffer(b.indexBuffer.handle(), vk::DeviceSize(0), vk::IndexType::eUint32);
}

void BindShapeBuffer(vk::CommandBuffer cmdBuffer, const ShapeBuffer& b)
{
	cmdBuffer.bindVertexBuffers(0u, b.vertexBuffer.handle(), vk::DeviceSize(0));
}

void DrawShape(vk::CommandBuffer cmdBuffer, const IndexedShapeBuffer& b)
{
	cmdBuffer.drawIndexed(b.indexCount, 1u, 0u, 0u, 0u);
}

void DrawShape(vk::CommandBuffer cmdBuffer, const ShapeBuffer& b)
{
	cmdBuffer.draw(b.vertexCount, 1u, 0u, 0u);
}

void MakeSphere(int32 sectorCount, int32 stackCount, float radius, IndexedShapeBuffer& sphereBuf)
{
	std::vector<float> vertices;
	std::vector<uint32> indices;

	float x, y, z, xy; // vertex position

	float sectorStep = 2.f * glm::pi<float>() / sectorCount;
	float stackStep = glm::pi<float>() / stackCount;
	float sectorAngle, stackAngle;

	for (int32 i = 0; i <= stackCount; ++i) {
		stackAngle = glm::pi<float>() / 2 - i * stackStep; // starting from pi/2 to -pi/2
		xy = radius * cosf(stackAngle);                    // r * cos(u)
		z = radius * sinf(stackAngle);                     // r * sin(u)

		// add (sectorCount+1) vertices per stack
		// the first and last vertices have same position and normal, but different tex coords
		for (int32 j = 0; j <= sectorCount; ++j) {
			sectorAngle = j * sectorStep; // starting from 0 to 2pi

			// vertex position (x, y, z)
			x = xy * cosf(sectorAngle); // r * cos(u) * cos(v)
			y = xy * sinf(sectorAngle); // r * cos(u) * sin(v)
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);
		}
	}

	int32 k1, k2;
	for (int32 i = 0; i < stackCount; ++i) {
		k1 = i * (sectorCount + 1); // beginning of current stack
		k2 = k1 + sectorCount + 1;  // beginning of next stack

		for (int32 j = 0; j < sectorCount; ++j, ++k1, ++k2) {
			// 2 triangles per sector excluding first and last stacks
			// k1 => k2 => k1+1
			if (i != 0) {
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}

			// k1+1 => k2 => k2+1
			if (i != (stackCount - 1)) {
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}
		}
	}

	sphereBuf = IndexedShapeBuffer("SphereShape", vertices, indices);
}

struct ShapesData {
	ShapeBuffer cube_triangleStrip;
	ShapeBuffer unitRectangle_triangleStrip;

	IndexedShapeBuffer cube_lineList;

	IndexedShapeBuffer sphere18x9;
	IndexedShapeBuffer sphere36x18;

	ShapesData()
	{
		cube_triangleStrip = ShapeBuffer("BoxShape", cube_triangleStripData);
		unitRectangle_triangleStrip = ShapeBuffer("UnitRectangleShape", unitRectangle_triangleStripData);

		cube_lineList = IndexedShapeBuffer("CubeShape", cube_verticesData, cube_lineList_indicesData);

		MakeSphere(18, 9, 1.f, sphere18x9);
		MakeSphere(36, 18, 1.f, sphere36x18);
	}

} * shapesData;

} // namespace

void rvk::bindCubeLines(vk::CommandBuffer cmdBuffer)
{
	BindShapeBuffer(cmdBuffer, shapesData->cube_lineList);
}

void rvk::drawCubeLines(vk::CommandBuffer cmdBuffer)
{
	DrawShape(cmdBuffer, shapesData->cube_lineList);
}

void rvk::bindUnitRect(vk::CommandBuffer cmdBuffer)
{
	BindShapeBuffer(cmdBuffer, shapesData->unitRectangle_triangleStrip);
}

void rvk::drawUnitRect(vk::CommandBuffer cmdBuffer)
{
	DrawShape(cmdBuffer, shapesData->unitRectangle_triangleStrip);
}

void rvk::bindCube(vk::CommandBuffer cmdBuffer)
{
	BindShapeBuffer(cmdBuffer, shapesData->cube_triangleStrip);
}

void rvk::drawCube(vk::CommandBuffer cmdBuffer)
{
	DrawShape(cmdBuffer, shapesData->cube_triangleStrip);
}

void rvk::bindSphere18x9(vk::CommandBuffer cmdBuffer)
{
	BindShapeBuffer(cmdBuffer, shapesData->sphere18x9);
}

void rvk::drawSphere18x9(vk::CommandBuffer cmdBuffer)
{
	DrawShape(cmdBuffer, shapesData->sphere18x9);
}

void rvk::bindSphere36x18(vk::CommandBuffer cmdBuffer)
{
	BindShapeBuffer(cmdBuffer, shapesData->sphere36x18);
}

void rvk::drawSphere36x18(vk::CommandBuffer cmdBuffer)
{
	DrawShape(cmdBuffer, shapesData->sphere36x18);
}

void rvk::Shapes::InitShapes()
{
	shapesData = new ShapesData();
}

void rvk::Shapes::DeinitShapes()
{
	delete shapesData;
}
