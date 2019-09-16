#include "pch.h"
#include "system/reflection/Reflector.h"



// Example of reflecting a struct statically
struct MyPod
{
	STATIC_REFLECTOR(MyPod)
	{
		S_REFLECT_VAR(number);
		S_REFLECT_VAR(v, PropertyFlags::Color);
	}



	int32 number;
	glm::vec3 v;

};


