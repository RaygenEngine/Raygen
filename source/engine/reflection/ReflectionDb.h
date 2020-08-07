#pragma once

#include <unordered_map>
class ReflClass;

class ReflectionDb {
	ReflectionDb() {}

	std::unordered_map<std::string, ReflClass*> classes;
	static ReflectionDb& Get()
	{
		static ReflectionDb instance;
		return instance;
	}

public:
	ReflectionDb(const ReflectionDb&) = delete;
	ReflectionDb(ReflectionDb&&) = delete;
	ReflectionDb& operator=(const ReflectionDb&) = delete;
	ReflectionDb& operator=(ReflectionDb&&) = delete;

	template<typename T>
	static void RegisterReflClass()
	{

		ReflClass* cl = &T::Z_MutableClass();
		Get().classes.emplace(cl->GetNameStr(), cl);
	}

	[[nodiscard]] static std::unordered_map<std::string, ReflClass*>& GetEntries() { return Get().classes; }
};


template<typename K>
struct ReflectionRegistrar {
	ReflectionRegistrar() { ReflectionDb::RegisterReflClass<K>(); }
};
