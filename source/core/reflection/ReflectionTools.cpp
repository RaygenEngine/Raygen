/*
#include "ReflectionTools.h"


namespace 
{
	using namespace PropertyFlags;


	struct CopyIntoFromReflector
	{
		CopyIntoFromReflector(Reflector& inReflector)
			: reflector(inReflector) {}
		Reflector& reflector;

		size_t GetElementCount() const
		{
			return reflector.GetProperties().size();
		}

		template<typename T>
		void operator()(ExactProperty& destProperty, ReflectorOperationResult& operationResult)
		{
			auto result = reflector.GetPropertyByName(destProperty.GetName());
			if (!result)
			{
				operationResult.PropertiesNotFoundInSource++;
				return;
			}

			if (!result->IsA<T>())
			{
				operationResult.TypeMissmatches++;
				return;
			}

			if (!result->HasSameFlags(destProperty))
			{
				operationResult.FlagMissmatches++;
			}

			operationResult.PropertiesNotFoundInDestination--;
			destProperty.GetRef<T>() = result->GetRef<T>();
		}
	};

	struct CopyIntoFromMapOfAny
	{
		CopyIntoFromMapOfAny(std::unordered_map<std::string, std::any>& inMap)
			: map(inMap) {}

		std::unordered_map<std::string, std::any>& map;

		size_t GetElementCount() const
		{
			return map.size();
		}

		template<typename T>
		void operator()(ExactProperty& destProperty, ReflectorOperationResult& operationResult)
		{
			auto it = map.find(destProperty.GetName());
			if (it == map.end())
			{
				operationResult.PropertiesNotFoundInSource++;
				return;
			}

			std::any& data = it->second;

			// why did we even bother with std::any when you need rtti for this...
			if (data.type() != typeid(T))
			{
				operationResult.TypeMissmatches++;
				return;
			}

			operationResult.PropertiesNotFoundInDestination--;
			destProperty.GetRef<T>() = std::any_cast<T>(data);
		}
	};

	template<typename CopyIntoFunctor>
	struct CopyIntoReflectorVisitor
	{
		CopyIntoReflectorVisitor(CopyIntoFunctor& inFunctor)
			: functor(inFunctor) {};

		CopyIntoFunctor& functor;
		ReflectorOperationResult result{};

		void Begin(Reflector&) 
		{
			result.PropertiesNotFoundInDestination = functor.GetElementCount();
		}

		template<typename T>
		void Visit(T& dest, ExactProperty& destProperty)
		{
			functor.operator()<T>(destProperty, result);
		}
	};
}

ReflectorOperationResult CopyReflectorInto(Reflector& Source, Reflector& Destination)
{
	CopyIntoFromReflector copyF(Source);
	CopyIntoReflectorVisitor<decltype(copyF)> visitor(copyF);
	
	CallVisitorOnEveryProperty(Destination, visitor);

	return visitor.result;
}

ReflectorOperationResult ApplyMapToReflector(std::unordered_map<std::string, std::any>& Source, Reflector& Destination)
{
	CopyIntoFromMapOfAny copyF(Source);
	CopyIntoReflectorVisitor<decltype(copyF)> visitor(copyF);

	CallVisitorOnEveryProperty(Destination, visitor);

	return visitor.result;
}

*/