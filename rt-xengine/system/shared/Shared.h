#ifndef SHARED_H
#define SHARED_H

// TODO: create more headers with the orphan functions

#include "GLM/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "GLM/gtx/transform.hpp"
#include "GLM/gtx/quaternion.hpp"
#include "GLM/gtx/matrix_decompose.hpp"   
#include "GLM/ext/matrix_clip_space.hpp" 

#include "system/shared/Types.h"
#include "system/shared/Event.h"
#include "system/shared/InputEnums.h"
#include "system/shared/RenderTarget.h"
#include "system/shared/TargetEnums.h"

#include <ostream>


template<typename T>
bool EqualsZero(T value)
{
	return glm::epsilonEqual(value, 0.f, glm::epsilon<T>());
}

template<typename T>
bool EqualsValue(T input, T value)
{
	return glm::epsilonEqual(input, value, glm::epsilon<T>());
}

// custom formatting
template<class T>
auto operator<<(std::ostream& os, const T& t) -> decltype(t.ToString(os), os)
{
	if (t)
		t->ToString(os);
	else
		os << "nullptr";

	return os;
}

// custom formatting
template<class T>
auto operator<<(std::ostream& os, T* t) -> decltype(t->ToString(os), os)
{
	if (t)
		t->ToString(os);
	else
		os << "nullptr";

	return os;
}

#include <sstream>
#include <iterator>

inline std::string Repeat(const std::string& input, size_t num)
{
	std::ostringstream os;
	std::fill_n(std::ostream_iterator<std::string>(os), num, input);
	return os.str();
}

inline std::string operator*(std::string str, std::size_t n)
{
	return Repeat(str, n);
}

#include "core/logger/Logger.h"

#endif // SHARED_H
