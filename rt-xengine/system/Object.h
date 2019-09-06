#pragma once

class Object
{
	utl::UID m_id;

public:
	Object()
		: m_id (utl::UUIDGenerator::GenerateUUID()) {}
	virtual ~Object() = default;
	
	[[nodiscard]] utl::UID GetUID() const { return m_id; }

	// TODO: return ostream
	virtual void ToString(std::ostream& os) const { os << "ObjectId: " << m_id; }
};