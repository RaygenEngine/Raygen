#ifdef HAS_INCLUDED_TEST_HEADER
#	error This header should only be included in a translation unit once. You should not write tests in header files.
#else
#	define HAS_INCLUDED_TEST_HEADER
#endif

#include "pch/pch.h"
#include <catch2/catch.hpp>

// renames
// CATCH2 REQUIRE(...)
#define REQ(...) REQUIRE(__VA_ARGS__)
// CATCH2 TEST_CASE(...)
#define TEST(...) TEST_CASE(__VA_ARGS__)
// CATCH2 SECTION(...)
#define SECT(...) SECTION(__VA_ARGS__)
