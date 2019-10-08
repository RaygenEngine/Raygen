#pragma once

class Object {
	utl::UID m_id;

protected:
	std::string m_name;

public:
	Object()
		: m_id(utl::UUIDGenerator::GenerateUUID())
	{
	}
	virtual ~Object() = default;

	[[nodiscard]] utl::UID GetUID() const { return m_id; }
	[[nodiscard]] std::string GetName() const { return m_name; }

	void SetName(const std::string& name) { m_name = name; }

	// TODO: return ostream
	virtual void ToString(std::ostream& os) const { os << "ObjectId: " << m_id; }
};