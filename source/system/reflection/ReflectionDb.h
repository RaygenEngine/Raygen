#pragma once
#include <unordered_map>


class ReflClass;

class ReflectionDb {
	ReflectionDb() {}

	static ReflectionDb& Get()
	{
		static ReflectionDb instance; // new?
		return instance;
	}


	// TODO: this should become a tree
	std::unordered_map<std::string, ReflClass*> classes;


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
struct ReflectionRegistar {
	ReflectionRegistar() { ReflectionDb::RegisterReflClass<K>(); }
};
