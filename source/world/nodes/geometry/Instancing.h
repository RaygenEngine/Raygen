#pragma once

#include "asset/util/ParsingAux.h"

struct Instance {
	glm::vec3 localTranslation;
	glm::quat localOrientation;
	glm::vec3 localScale;
	glm::mat4 localMatrix;

	glm::vec3 worldTranslation;
	glm::quat worldOrientation;
	glm::vec3 worldScale;
	glm::mat4 worldMatrix;

	Instance()
		: localTranslation(0.f, 0.f, 0.f)
		, localOrientation(0.f, 0.f, 0.f, 1.f)
		, localScale(1.f, 1.f, 1.f)
		, localMatrix(glm::mat4(1))
		, worldTranslation(0.f, 0.f, 0.f)
		, worldOrientation(0.f, 0.f, 0.f, 1.f)
		, worldScale(1.f, 1.f, 1.f)
		, worldMatrix(glm::mat4(1))
	{
	}

	void LoadFromXML(const tinyxml2::XMLElement* xmlData)
	{
		ParsingAux::ReadFloatsAttribute(xmlData, "translation", localTranslation);
		glm::vec3 eulerPYR{ 0.f, 0.f, 0.f };
		ParsingAux::ReadFloatsAttribute(xmlData, "euler_pyr", eulerPYR);
		localOrientation = glm::quat(glm::radians(eulerPYR));
		ParsingAux::ReadFloatsAttribute(xmlData, "scale", localScale);

		// calculate local matrix after loading
		localMatrix = utl::GetTransformMat(localTranslation, localOrientation, localScale);
	}

	void Update(const glm::mat4& parentMat)
	{
		worldMatrix = parentMat * localMatrix;

		// TODO: optimize
		glm::vec3 skew;
		glm::vec4 persp;
		glm::decompose(worldMatrix, worldScale, worldOrientation, worldTranslation, skew, persp);
	}
};

// TODO dirty instances on geom moves
class InstanceGroup {
	std::unordered_map<std::string, Instance> m_instances;

public:
	InstanceGroup() = default;
	~InstanceGroup() = default;

	uint32 GetCount() const { return static_cast<uint32>(m_instances.size()); }

	// TODO optimize this
	std::vector<glm::mat4> GetMatricesBlock() const
	{
		std::vector<glm::mat4> mats;
		for (auto& ins : m_instances)
			mats.push_back(ins.second.worldMatrix);

		return mats;
	}

	void AddInstanceFromXML(const tinyxml2::XMLElement* xmlData)
	{
		std::string name;
		ParsingAux::ReadFillEntityName(xmlData, name);

		Instance inst{};

		inst.LoadFromXML(xmlData);

		m_instances[name] = inst;
	}

	void UpdateInstances(const glm::mat4& parentMat)
	{
		for (auto& ins : m_instances)
			ins.second.Update(parentMat);
	}

	// iterators
	auto begin() { return m_instances.begin(); }
	auto end() { return m_instances.end(); }

	auto begin() const { return m_instances.begin(); }
	auto end() const { return m_instances.end(); }

	auto cbegin() const { return m_instances.begin(); }
	auto cend() const { return m_instances.end(); }
};
