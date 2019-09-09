#pragma once

#include "assets/texture/Texture.h"

class CubeMap : public Object
{
	std::shared_ptr<Texture> m_faces[CMF_COUNT];

	uint32 m_width;
	uint32 m_height;

	bool m_hdr;

	std::once_flag m_initFlag;
	
public:
	CubeMap()
		: m_width(0),
		  m_height(0),
		  m_hdr(false) {}

	bool LoadFaceTexture(CubeMapFace index, const std::string& path);

	uint32 GetWidth() const { return m_width; }
	uint32 GetHeight() const { return m_height; }
	bool IsHdr() const { return m_hdr; }

	Texture* GetFace(CubeMapFace faceIndex) const { return m_faces[faceIndex].get(); }
};
