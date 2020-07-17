#pragma once
#include "editor/windows/EdWindow.h"

namespace ed {
class MeshGenerator : public UniqueWindow {

	PodHandle<Mesh> mesh;

	enum class MeshGenerationType
	{
		Plane,
		Box,
		Sphere
	} genType{};


public:
	struct PlaneSettings {
		float worldSize;
		int32 quadSide;
	} planeSettings;

	MeshGenerator(std::string_view name)
		: UniqueWindow(name)
	{
	}

	virtual void ImguiDraw();


	virtual ~MeshGenerator() = default;

	void PlaneGenDraw();
};

} // namespace ed
