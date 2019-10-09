#pragma once

class Object {
	utl::UID m_id{};

public:
	Object() { m_id = utl::UUIDGenerator::GenerateUUID(); }
	Object(const Object&) = delete;
	Object(Object&&) = delete;
	Object& operator=(const Object&) = delete;
	Object& operator=(Object&&) = delete;
	virtual ~Object() = default;

	[[nodiscard]] utl::UID GetUID() const { return m_id; }
};
