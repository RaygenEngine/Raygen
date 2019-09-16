#include "pch.h"
#include "system/reflection/ReflectionTools.h"
#include "system/reflection/Reflector.h"

#include "asset/pods/CubemapPod.h"
#include "asset/pods/GltfFilePod.h"
#include "asset/pods/ImagePod.h"
#include "asset/pods/MaterialPod.h"
#include "asset/pods/ModelPod.h"
#include "asset/pods/ShaderPod.h"
#include "asset/pods/TextPod.h"
#include "asset/pods/TexturePod.h"
#include "asset/pods/XMLDocPod.h"

#include <iostream>

struct ReflVisitor
{
	template<typename T>
	void visit(T& t, ExactProperty& p)
	{
		std::cout << "Visited by property: " << p.GetName();
	}

	void visit(PodHandle<TextPod>& t, ExactProperty& p)
	{
		std::cout << "Visited by text pod: " << p.GetName();
	}
};

struct StaticReflTest
{
	STATIC_REFLECTOR(StaticReflTest)
	{
		S_REFLECT_VAR(xmlpod);
		S_REFLECT_VAR(txt);
	}

	PodHandle<XMLDocPod> xmlpod;
	PodHandle<TextPod> txt;
};

namespace test {
void test()
{
	ReflVisitor v;
	StaticReflTest s;

	CallVisitorOnEveryProperty(&s, v);
}
}
