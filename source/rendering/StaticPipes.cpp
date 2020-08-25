#include "pch.h"
#include "StaticPipes.h"

#include "rendering/passes/lightblend/DirlightBlend.h"

namespace vl {
void StaticPipes::InternalInitRegistered()
{
	StaticPipes::Init<DirlightBlend>();
}
} // namespace vl
