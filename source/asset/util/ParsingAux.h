#pragma once

#include <tinyxml2/tinyxml2.h>

namespace ParsingAux {
inline bool AttributeExists(const tinyxml2::XMLElement* xmlElement, const char* attribute)
{
	return xmlElement->FindAttribute(attribute);
}
// TODO: glm::make_vec
template<typename T>
bool ReadFloats(const char* data, T& floats)
{
	const auto count = sizeof(T) / sizeof(float);
	float parts[count];
	if (data && utl::SplitStringIntoTArray(parts, count, data, ',')) {
		floats = reinterpret_cast<T&>(parts);
		return true;
	}
	return false;
}

template<typename T>
bool ReadFloatsAttribute(const tinyxml2::XMLElement* xmlElement, const char* attribute, T& floats)
{
	LOG_TRACE("Reading XML attribute: \"{0}\"", attribute);

	if (AttributeExists(xmlElement, attribute)) {
		ReadFloats<T>(xmlElement->Attribute(attribute), floats);
		return true;
	}

	LOG_TRACE("Missing XML attribute: \"{0}\", defaulting", attribute);
	return false;
}

inline bool ReadStringAttribute(const tinyxml2::XMLElement* xmlElement, const char* attribute, std::string& str)
{
	LOG_TRACE("Reading attribute \"{0}\"", attribute);

	if (AttributeExists(xmlElement, attribute)) {
		str = xmlElement->Attribute(attribute);
		return true;
	}

	LOG_TRACE("Missing string attribute \"{0}\", defaulting", attribute);
	return false;
}

inline bool ReadFillEntityName(const tinyxml2::XMLElement* xmlElement, std::string& entityName)
{
	if (AttributeExists(xmlElement, "name")) {
		ReadStringAttribute(xmlElement, "name", entityName);
		return true;
	}

	entityName = "unnamed$uid$" + std::to_string(utl::UUIDGenerator::GenerateUUID());
	LOG_WARN("Missing XML Entity / Node name, defaulting to {}", entityName);

	return false;
}

inline bool ReadFillEntityType(const tinyxml2::XMLElement* xmlElement, std::string& entityType)
{
	entityType = std::string(xmlElement->Name());
	return true;
}

template<typename T>
std::string FloatsToString(T floats)
{
	std::stringstream ss;

	// static_assert(std::is_same_v<glm::vec, T>, "Expects a glm type"); // find how to do this for glm
	ss << floats[0];

	for (int32 i = 1; i < floats.length(); ++i) {
		ss << ", ";
		ss << floats[i];
	}
	return ss.str();
}
} // namespace ParsingAux
