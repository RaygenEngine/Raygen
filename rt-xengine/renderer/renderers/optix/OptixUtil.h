#ifndef OPTIXUTIL_H
#define OPTIXUTIL_H

// Utils for Optix 6.0

#include "Optix/optixu/optixpp_namespace.h"
#include "glad/glad.h"

#include <string>                    

#define ACCEL(x) TranslateAS(x)
#define ACCEL_PROP(x) TranslateASP(x)

namespace Renderer::Optix
{
	enum ACCELERATION_STRUCTURE
	{
		AS_TRBVH = 0,
		AS_SBVH,
		AS_BVH,
		AS_NOACCEL
	};

	enum ACCELERATION_STRUCTURE_PROPERTY
	{
		ASP_REFIT = 0,
		ASP_VERTEX_BUFFER_NAME,
		ASP_VERTEX_BUFFER_STRIDE,
		ASP_INDEX_BUFFER_NAME,
		ASP_INDEX_BUFFER_STRIDE,
		ASP_CHUNK_SIZE,
	};

	constexpr const char* TranslateASP(const ACCELERATION_STRUCTURE_PROPERTY accelProp)
	{
		switch(accelProp)
		{
			case ASP_REFIT:
				return "refit";
			case ASP_VERTEX_BUFFER_NAME:
				return "vertex_buffer_name";
			case ASP_VERTEX_BUFFER_STRIDE:
				return "vertex_buffer_stride";
			case ASP_INDEX_BUFFER_NAME:
				return "index_buffer_name";
			case ASP_INDEX_BUFFER_STRIDE:
				return "index_buffer_stride";
			case ASP_CHUNK_SIZE:
				return "chunk_size";
			default:
				return nullptr;
		}
	}

	constexpr const char* TranslateAS(const ACCELERATION_STRUCTURE accel)
	{
		switch(accel)
		{
			case AS_TRBVH:
				return "Trbvh";
			case AS_SBVH:
				return "Sbvh";
			case AS_BVH:
				return "Bvh";
			case AS_NOACCEL:
				return "NoAccel";
			default:
				return "NoAccel";
		}
	}

	inline optix::Buffer CreateOptixBuffer(optix::Context context,
		RTbuffertype type,
		RTformat format,
		uint32 width,
		uint32 height,
		bool usePbo)
	{
		optix::Buffer buffer;
		if (usePbo)
		{
			// First allocate the memory for the GL buffer, then attach it to OptiX.

			// Assume ubyte4 or float4
			uint32 elemSize = format == RT_FORMAT_UNSIGNED_BYTE4 ? 4u : 16u;

			GLuint pbo{};
			glGenBuffers(1, &pbo);
			glBindBuffer(GL_ARRAY_BUFFER, pbo);
			glBufferData(GL_ARRAY_BUFFER, elemSize * width * height, nullptr, GL_STREAM_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			buffer = context->createBufferFromGLBO(type, pbo);
			buffer->setFormat(format);
			buffer->setSize(width, height);
		}
		else
		{
			buffer = context->createBuffer(type, format, width, height);
		}

		return buffer;
	}

	inline void ResizeOptixBuffer(optix::Buffer buffer, uint32 width, uint32 height)
	{
		buffer->setSize(width, height);

		// Check if we have a GL interop display buffer
		const uint32 pboId = buffer->getGLBOId();
		if (pboId)
		{
			buffer->unregisterGLBuffer();
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboId);
			glBufferData(GL_PIXEL_UNPACK_BUFFER, buffer->getElementSize() * width * height, nullptr, GL_STREAM_DRAW);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			buffer->registerGLBuffer();
		}
	}


	inline void CalculateCameraTracingVariables(float aspectRatio, const glm::vec3& lookat, const glm::vec3& position,
	                                            const glm::vec3& upDir, float fov, glm::vec3& u, glm::vec3& v,
	                                            glm::vec3& w)
	{
		w = lookat - position; // Do not normalize W -- it implies focal length
		float wlen = glm::length(w);
		u = glm::normalize(glm::cross(w, upDir));
		v = glm::normalize(glm::cross(u, w)); 
		// fov is vertical
		const float vlen = wlen * tanf(0.5f * glm::radians(fov));
		v *= vlen;
		const float ulen = vlen * aspectRatio;
		u *= ulen;
	}

	inline void PrintGeometryInstanceRecursively(const optix::GeometryInstance& geometryInstance,
	                                             std::string& treeString, uint depth)
	{
		treeString += std::string("    ") * depth + "|" + std::string("--");
		treeString += "geometry_instance\n";
		treeString += std::string("    ") * (depth + 1) + "|" + std::string("--");
		treeString += "geometry\n";
		for(uint i = 0; i < geometryInstance->getMaterialCount(); ++i)
		{
			treeString += std::string("    ") * (depth + 1) + "|" + std::string("--");
			treeString += "material\n";
		}
	}

	inline void PrintGeometryGroupRecursively(const optix::GeometryGroup& geometryGroup, std::string& treeString,
	                                          uint depth = 0)
	{
		treeString += std::string("    ") * depth + "|" + std::string("--");
		treeString += "geometry_group ----> accel(" + geometryGroup->getAcceleration()->getTraverser() + ")\n";
		for(uint i = 0; i < geometryGroup->getChildCount(); ++i)
		{
			PrintGeometryInstanceRecursively(geometryGroup->getChild(i), treeString, depth + 1);
		}
	}

	inline void PrintTransformRecursively(const optix::Transform&, std::string&, uint);

	inline void PrintGroupRecursively(const optix::Group& group, std::string& treeString, uint depth = 0)
	{
		treeString += std::string("    ") * depth + "|" + std::string("--");
		treeString += "group ----> accel(" + group->getAcceleration()->getTraverser() + ")\n";
		for(uint i = 0; i < group->getChildCount(); ++i)
		{
			const auto childType = group->getChildType(i);
			switch(childType)
			{
				case RT_OBJECTTYPE_GROUP:
					PrintGroupRecursively(group->getChild<optix::Group>(i), treeString, depth + 1);
					break;
				case RT_OBJECTTYPE_GEOMETRY_GROUP:
					PrintGeometryGroupRecursively(group->getChild<optix::GeometryGroup>(i), treeString, depth + 1);
					break;
				case RT_OBJECTTYPE_TRANSFORM:
					PrintTransformRecursively(group->getChild<optix::Transform>(i), treeString, depth + 1);
					break;
				case RT_OBJECTTYPE_SELECTOR:
					break;
				default: ;
			}
		}
	}

	inline void PrintTransformRecursively(const optix::Transform& transform, std::string& treeString, uint depth = 0)
	{
		treeString += std::string("    ") * depth + "|" + std::string("--");
		treeString += "transform\n";
		switch (transform->getChildType())
		{
		case RT_OBJECTTYPE_GROUP:

			PrintGroupRecursively(transform->getChild<optix::Group>(), treeString, depth + 1);
			break;

		case RT_OBJECTTYPE_GEOMETRY_GROUP:

			PrintGeometryGroupRecursively(transform->getChild<optix::GeometryGroup>(), treeString, depth + 1);
			break;

		case RT_OBJECTTYPE_TRANSFORM:

			PrintTransformRecursively(transform->getChild<optix::Transform>(), treeString, depth + 1);
			break;

		case RT_OBJECTTYPE_SELECTOR:
			break;

		default:;
		}
	}
	
	inline void MarkDirtyTransformRecursively(const optix::Transform&);

	inline void MarkDirtyGroupRecursively(const optix::Group& group)
	{
		for(uint i = 0; i < group->getChildCount(); ++i)
		{
			switch(group->getChildType(i))
			{
				case RT_OBJECTTYPE_GROUP:
					MarkDirtyGroupRecursively(group->getChild<optix::Group>(i));
					break;
				case RT_OBJECTTYPE_GEOMETRY_GROUP:
					group->getChild<optix::GeometryGroup>(i)->getAcceleration()->markDirty();
					break;
				case RT_OBJECTTYPE_TRANSFORM:
					MarkDirtyTransformRecursively(group->getChild<optix::Transform>(i));
					break;
				case RT_OBJECTTYPE_SELECTOR:
					break;
				default: ;
			}
		}
		group->getAcceleration()->markDirty();
	}

	inline void MarkDirtyTransformRecursively(const optix::Transform& transform)
	{
		switch(transform->getChildType())
		{
			case RT_OBJECTTYPE_GROUP:
				MarkDirtyGroupRecursively(transform->getChild<optix::Group>());
				break;
			case RT_OBJECTTYPE_GEOMETRY_GROUP:
				transform->getChild<optix::GeometryGroup>()->getAcceleration()->markDirty();
				break;
			case RT_OBJECTTYPE_TRANSFORM:
				MarkDirtyTransformRecursively(transform->getChild<optix::Transform>());
				break;
			case RT_OBJECTTYPE_SELECTOR:
				break;
			default: ;
		}
	}
}

#endif // OPTIXUTIL_H
