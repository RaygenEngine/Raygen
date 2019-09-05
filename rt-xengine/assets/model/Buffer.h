#pragma once

namespace tinygltf
{
	class Model;
	struct Accessor;
}


// RTTI buffer
class Buffer
{
	BufferComponentType m_componentType;
	BufferElementType m_elementType;

	std::vector<byte> m_data;

public:

	void LoadFromGltfData(const tinygltf::Model& modelData, const tinygltf::Accessor& accessorData);

	BufferComponentType GetComponentType() const { return m_componentType; }
	BufferElementType GetElementType() const { return m_elementType; }

	const std::vector<byte>& GetData() const { return m_data; }

	bool IsEmpty() const { return m_data.empty(); }

	int32 GetByteCount() const { return static_cast<int32>(m_data.size()); }
	int32 GetElementCount() const;

	template<typename T>
	T* GetTypedDataPtr() { return reinterpret_cast<T*>(m_data.data()); }
};

