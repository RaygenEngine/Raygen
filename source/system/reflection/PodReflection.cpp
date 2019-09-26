#include "system/reflection/PodReflection.h"

#include "ctti/detailed_nameof.hpp"
#include <iostream>

using PodTypeId = ctti::type_id_t;

std::vector<AssetPod*> GetOneOfEach()
{
	using FactoryFunction = std::function<AssetPod* ()>;

	static auto podFactory = CreateMapOnPodType<FactoryFunction>(
	[](auto typeCarrier)
	{
		using PodType = std::remove_pointer_t<decltype(typeCarrier)>; // Hack to enable "templated" lambdas (solved in cpp20)

		std::cout << "Registered a factory method for pod type: " << ctti::detailed_nameof<PodType>().full_name() << "\n";
		return []() -> AssetPod * { return new PodType(); };
	});


	std::vector<AssetPod*> OneOfEach;
	for (auto& [typeinfo, factory] : podFactory)
	{
		std::cout << "Generating: " << typeinfo.name() << "\n";
		OneOfEach.push_back(factory());
		
		if (typeinfo.hash() == GetPodTypeHash<ImagePod>())
		{
			std::cout << "Generating: " << typeinfo.name() << "\n";
			// not really one of each, 2 image pods.
			OneOfEach.push_back(factory());
		}
	}

	return OneOfEach;
}

void FindVariableInAllPods()
{
	std::string varname;
	while (1)
	{
		std::cout << "Search for: ";
		std::cin >> varname;

		ForEachPodType(
			[&](auto typedDummy) {
			if (GetReflector(typedDummy).HasProperty(varname))
			{
				std::cout << "Found '" << varname << "' in: " << GetTypeIdPtr(typedDummy).name() << "\n";
			}
		});
	}
}

Reflector GetPodReflector(AssetPod* pod)
{
	static auto reflectorMap = CreateMapOnPodType<StaticReflector*>(
	[](auto d)
	{
		using PodType = std::remove_pointer_t<decltype(d)>;
		
		return &const_cast<StaticReflector&>(PodType::StaticReflect());
	}
	);

	return reflectorMap[pod->type]->ToAbsoluteReflector(pod);
}
