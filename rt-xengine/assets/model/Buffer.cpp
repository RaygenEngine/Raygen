#include "pch.h"

#include "assets/model/Buffer.h"
#include "assets/model/GltfAux.h"

void Buffer::LoadFromGltfData(const tinygltf::Model& modelData, const tinygltf::Accessor& accessorData)
{
	auto& bufferView = modelData.bufferViews.at(accessorData.bufferView);
	auto& buffer = modelData.buffers.at(bufferView.buffer);

	m_elementType = GetElementTypeFromGltf(accessorData.type);
	m_componentType = GetComponentTypeFromGltf(accessorData.componentType);
	
	const auto byteOffset = accessorData.byteOffset + bufferView.byteOffset;
	
	auto elementBytes = (GetComponentTypeByteCount(m_componentType) * GetElementComponentCount(m_elementType));
	auto upper = elementBytes * accessorData.count;

	uint64 j = 0;
	for (uint64 i = 0; i < upper; ++i, j+= bufferView.byteStride)
	{
		m_data.push_back(buffer.data[byteOffset + i + j]);
	}
}

int32 Buffer::GetElementCount() const
{
	const auto elementBytes = GetComponentTypeByteCount(m_componentType) * GetElementComponentCount(m_elementType);
	return static_cast<int32>(m_data.size()) / elementBytes;
}

