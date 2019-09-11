#include "pch.h"

#include "assets/other/gltf/GltfFile.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include "tinygltf/tiny_gltf.h"


bool GltfFile::Load()
{
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	auto ext = m_uri.extension();

	bool ret = false;

	{
		TIMER_STATIC_SCOPE("load model time");

		if (ext.compare(".gltf") == 0)
		{
			m_gltfData = new tinygltf::Model;
			ret = loader.LoadASCIIFromFile(m_gltfData, &err, &warn, m_uri.string());
		}
		else if (ext.compare(".glb") == 0)
		{
			RT_XENGINE_ASSERT(false, "glb data is not handled yet");
			ret = loader.LoadBinaryFromFile(m_gltfData, &err, &warn, m_uri.string());
		}
	}

	CLOG_WARN(!warn.empty(), warn.c_str());
	CLOG_ERROR(!err.empty(), err.c_str());

	if (!ret)
	{
		delete m_gltfData;
		return false;
	}

	return true;
}

void GltfFile::Unload()
{
	delete m_gltfData;
}
