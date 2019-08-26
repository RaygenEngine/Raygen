#pragma once

#include "tinyxml2/tinyxml2.h"

namespace Assets
{

	inline bool AttributeExists(const tinyxml2::XMLElement* xmlElement, const char* attribute)
	{
		return xmlElement->FindAttribute(attribute);
	}

	template <typename T>
	bool ReadFloats(const char* data, T& floats)
	{
		const auto count = sizeof(T) / sizeof(float);
		float parts[count];
		if (data && Core::SplitStringIntoTArray(parts, count, data, ','))
		{
			floats = reinterpret_cast<T&>(parts);
			return true;
		}
		return false;
	}

	template <typename T>
	bool ReadFloatsAttribute(const tinyxml2::XMLElement* xmlElement, const char* attribute, T& floats)
	{
		RT_XENGINE_LOG_TRACE("Reading XML attribute: \"{0}\"", attribute);

		if (AttributeExists(xmlElement, attribute))
		{
			ReadFloats<T>(xmlElement->Attribute(attribute), floats);
			return true;
		}

		RT_XENGINE_LOG_WARN("Missing XML attribute: \"{0}\", defaulting", attribute);
		return false;
	}

	inline bool ReadStringAttribute(const tinyxml2::XMLElement* xmlElement, const char* attribute, std::string& str)
	{
		RT_XENGINE_LOG_TRACE("Reading attribute \"{0}\"", attribute);

		if (AttributeExists(xmlElement, attribute))
		{
			str = xmlElement->Attribute(attribute);
			return true;
		}

		RT_XENGINE_LOG_WARN("Missing string attribute \"{0}\", defaulting", attribute);
		return false;
	}

	inline bool ReadFillEntityName(const tinyxml2::XMLElement* xmlElement, std::string& entityName)
	{
		if (AttributeExists(xmlElement, "name"))
		{
			ReadStringAttribute(xmlElement, "name", entityName);
			return true;
		}

		entityName = "unnamed$uid$" + std::to_string(Core::UUIDGenerator::GenerateUUID());
		RT_XENGINE_LOG_WARN("Missing XML Entity / Node name, defaulting to {}", entityName);

		return false;
	}
}
