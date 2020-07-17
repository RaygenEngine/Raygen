#pragma once


namespace detail {
template<typename Pod>
struct GpuTypeExtractor {
	using type = void;
};
} // namespace detail


// The code that gets generated for each pod type.
// Forward declares the Pod itself and the GpuPod
// Also declares a specialization for the GpuTypeExtractor on forward declarations. (A type table from Pod -> GpuPod)
#define ZZZ_PER_POD_EXPANSION(Pod)                                                                                     \
	struct Pod;                                                                                                        \
	struct Gpu##Pod;                                                                                                   \
	namespace detail {                                                                                                 \
		template<>                                                                                                     \
		struct GpuTypeExtractor<Pod> {                                                                                 \
			using type = Gpu##Pod;                                                                                     \
		};                                                                                                             \
	}


// Pretty access to the GpuTypeExtractor table. Converts the forward declared structs like so: Gpu<Image> == GpuImage
template<typename Pod>
using Gpu = typename detail::GpuTypeExtractor<Pod>::type;


// Macro argument overloading based on: https://stackoverflow.com/a/26408195

// MSVC fix based on: https://stackoverflow.com/questions/5134523/msvc-doesnt-expand-va-args-correctly
// CHECK:
// Should be fixable with /preprocessor:experimental in msvc 16.5 (other headers fail to compile)
// See also: https://docs.microsoft.com/en-us/cpp/preprocessor/preprocessor-experimental-overview?view=vs-2019
//

#define ZZZ_EXPAND(x) x

// Argument Count Detector
#define ZZZ__NARG__(...)  ZZZ__NARG_I_(__VA_ARGS__, ZZZ__RSEQ_N())
#define ZZZ__NARG_I_(...) ZZZ_EXPAND(ZZZ__ARG_N(__VA_ARGS__))
#define ZZZ__ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21,     \
	_22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, \
	_45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, N, ...)             \
	N
#define ZZZ__RSEQ_N()                                                                                                  \
	63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36,    \
		35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8,  \
		7, 6, 5, 4, 3, 2, 1, 0

// General definition for any function name
#define ZZZ_VFUNC_(name, n) name##n
#define ZZZ_VFUNC(name, n)  ZZZ_VFUNC_(name, n)
#define ZZZVFUNC(func, ...) ZZZ_EXPAND(ZZZ_VFUNC(func, ZZZ__NARG__(__VA_ARGS__))(__VA_ARGS__))

#define ENGINE_PODS_FWD(...) ZZZ_EXPAND(ZZZVFUNC(ZZZ_POD_EXP, __VA_ARGS__))


//
// Python script to generate the overload macros: (change the range to get more macros)
// ===================================================================================
//
// for i in range(1, 32):
//   print("#define ZZZ_POD_EXP" + str(i) + "(", end='')
//   for j in range(1, i):
//     print("a" + str(j)+", ", end='')
//   print("a"+ str(i), end='')
//   print(") ", end='')
//
//   for j in range(1, i+1):
//     print("ZZZ_PER_POD_EXPANSION(a" + str(j) + ")", end=' ')
//   print()
//
// ===================================================================================
//


#define ZZZ_POD_EXP1(a1)         ZZZ_PER_POD_EXPANSION(a1)
#define ZZZ_POD_EXP2(a1, a2)     ZZZ_PER_POD_EXPANSION(a1) ZZZ_PER_POD_EXPANSION(a2)
#define ZZZ_POD_EXP3(a1, a2, a3) ZZZ_PER_POD_EXPANSION(a1) ZZZ_PER_POD_EXPANSION(a2) ZZZ_PER_POD_EXPANSION(a3)
#define ZZZ_POD_EXP4(a1, a2, a3, a4)                                                                                   \
	ZZZ_PER_POD_EXPANSION(a1) ZZZ_PER_POD_EXPANSION(a2) ZZZ_PER_POD_EXPANSION(a3) ZZZ_PER_POD_EXPANSION(a4)
#define ZZZ_POD_EXP5(a1, a2, a3, a4, a5)                                                                               \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2) ZZZ_PER_POD_EXPANSION(a3) ZZZ_PER_POD_EXPANSION(a4) ZZZ_PER_POD_EXPANSION(a5)
#define ZZZ_POD_EXP6(a1, a2, a3, a4, a5, a6)                                                                           \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3) ZZZ_PER_POD_EXPANSION(a4) ZZZ_PER_POD_EXPANSION(a5) ZZZ_PER_POD_EXPANSION(a6)
#define ZZZ_POD_EXP7(a1, a2, a3, a4, a5, a6, a7)                                                                       \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4) ZZZ_PER_POD_EXPANSION(a5) ZZZ_PER_POD_EXPANSION(a6) ZZZ_PER_POD_EXPANSION(a7)
#define ZZZ_POD_EXP8(a1, a2, a3, a4, a5, a6, a7, a8)                                                                   \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a5) ZZZ_PER_POD_EXPANSION(a6) ZZZ_PER_POD_EXPANSION(a7) ZZZ_PER_POD_EXPANSION(a8)
#define ZZZ_POD_EXP9(a1, a2, a3, a4, a5, a6, a7, a8, a9)                                                               \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a5)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a6) ZZZ_PER_POD_EXPANSION(a7) ZZZ_PER_POD_EXPANSION(a8) ZZZ_PER_POD_EXPANSION(a9)
#define ZZZ_POD_EXP10(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)                                                         \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a5)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a6)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a7) ZZZ_PER_POD_EXPANSION(a8) ZZZ_PER_POD_EXPANSION(a9) ZZZ_PER_POD_EXPANSION(a10)
#define ZZZ_POD_EXP11(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11)                                                    \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a5)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a6)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a7)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a8) ZZZ_PER_POD_EXPANSION(a9) ZZZ_PER_POD_EXPANSION(a10) ZZZ_PER_POD_EXPANSION(a11)
#define ZZZ_POD_EXP12(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12)                                               \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a5)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a6)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a7)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a8)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a9) ZZZ_PER_POD_EXPANSION(a10) ZZZ_PER_POD_EXPANSION(a11) ZZZ_PER_POD_EXPANSION(a12)
#define ZZZ_POD_EXP13(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13)                                          \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a5)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a6)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a7)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a8)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a9)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a10) ZZZ_PER_POD_EXPANSION(a11) ZZZ_PER_POD_EXPANSION(a12) ZZZ_PER_POD_EXPANSION(a13)
#define ZZZ_POD_EXP14(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14)                                     \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a5)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a6)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a7)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a8)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a9)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a10)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a11) ZZZ_PER_POD_EXPANSION(a12) ZZZ_PER_POD_EXPANSION(a13) ZZZ_PER_POD_EXPANSION(a14)
#define ZZZ_POD_EXP15(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15)                                \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a5)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a6)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a7)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a8)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a9)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a10)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a11)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a12) ZZZ_PER_POD_EXPANSION(a13) ZZZ_PER_POD_EXPANSION(a14) ZZZ_PER_POD_EXPANSION(a15)
#define ZZZ_POD_EXP16(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16)                           \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a5)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a6)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a7)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a8)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a9)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a10)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a11)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a12)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a13) ZZZ_PER_POD_EXPANSION(a14) ZZZ_PER_POD_EXPANSION(a15) ZZZ_PER_POD_EXPANSION(a16)
#define ZZZ_POD_EXP17(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17)                      \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a5)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a6)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a7)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a8)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a9)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a10)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a11)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a12)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a13)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a14) ZZZ_PER_POD_EXPANSION(a15) ZZZ_PER_POD_EXPANSION(a16) ZZZ_PER_POD_EXPANSION(a17)
#define ZZZ_POD_EXP18(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18)                 \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a5)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a6)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a7)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a8)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a9)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a10)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a11)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a12)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a13)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a14)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a15) ZZZ_PER_POD_EXPANSION(a16) ZZZ_PER_POD_EXPANSION(a17) ZZZ_PER_POD_EXPANSION(a18)
#define ZZZ_POD_EXP19(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19)            \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a5)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a6)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a7)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a8)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a9)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a10)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a11)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a12)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a13)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a14)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a15)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a16) ZZZ_PER_POD_EXPANSION(a17) ZZZ_PER_POD_EXPANSION(a18) ZZZ_PER_POD_EXPANSION(a19)
#define ZZZ_POD_EXP20(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20)       \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a5)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a6)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a7)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a8)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a9)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a10)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a11)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a12)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a13)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a14)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a15)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a16) ZZZ_PER_POD_EXPANSION(a17) ZZZ_PER_POD_EXPANSION(a18) ZZZ_PER_POD_EXPANSION(a19)        \
		ZZZ_PER_POD_EXPANSION(a20)
#define ZZZ_POD_EXP21(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21)  \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a5)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a6)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a7)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a8)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a9)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a10)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a11)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a12)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a13)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a14)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a15)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a16) ZZZ_PER_POD_EXPANSION(a17) ZZZ_PER_POD_EXPANSION(a18) ZZZ_PER_POD_EXPANSION(a19)        \
		ZZZ_PER_POD_EXPANSION(a20) ZZZ_PER_POD_EXPANSION(a21)
#define ZZZ_POD_EXP22(                                                                                                 \
	a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22)               \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a5)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a6)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a7)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a8)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a9)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a10)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a11)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a12)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a13)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a14)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a15)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a16) ZZZ_PER_POD_EXPANSION(a17) ZZZ_PER_POD_EXPANSION(a18) ZZZ_PER_POD_EXPANSION(a19)        \
		ZZZ_PER_POD_EXPANSION(a20) ZZZ_PER_POD_EXPANSION(a21) ZZZ_PER_POD_EXPANSION(a22)
#define ZZZ_POD_EXP23(                                                                                                 \
	a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23)          \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a5)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a6)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a7)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a8)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a9)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a10)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a11)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a12)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a13)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a14)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a15)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a16) ZZZ_PER_POD_EXPANSION(a17) ZZZ_PER_POD_EXPANSION(a18) ZZZ_PER_POD_EXPANSION(a19)        \
		ZZZ_PER_POD_EXPANSION(a20) ZZZ_PER_POD_EXPANSION(a21) ZZZ_PER_POD_EXPANSION(a22) ZZZ_PER_POD_EXPANSION(a23)
#define ZZZ_POD_EXP24(                                                                                                 \
	a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24)     \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a5)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a6)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a7)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a8)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a9)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a10)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a11)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a12)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a13)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a14)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a15)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a16) ZZZ_PER_POD_EXPANSION(a17) ZZZ_PER_POD_EXPANSION(a18) ZZZ_PER_POD_EXPANSION(a19)        \
		ZZZ_PER_POD_EXPANSION(a20) ZZZ_PER_POD_EXPANSION(a21) ZZZ_PER_POD_EXPANSION(a22) ZZZ_PER_POD_EXPANSION(a23)    \
			ZZZ_PER_POD_EXPANSION(a24)
#define ZZZ_POD_EXP25(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21,  \
	a22, a23, a24, a25)                                                                                                \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a5)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a6)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a7)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a8)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a9)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a10)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a11)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a12)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a13)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a14)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a15)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a16) ZZZ_PER_POD_EXPANSION(a17) ZZZ_PER_POD_EXPANSION(a18) ZZZ_PER_POD_EXPANSION(a19)        \
		ZZZ_PER_POD_EXPANSION(a20) ZZZ_PER_POD_EXPANSION(a21) ZZZ_PER_POD_EXPANSION(a22) ZZZ_PER_POD_EXPANSION(a23)    \
			ZZZ_PER_POD_EXPANSION(a24) ZZZ_PER_POD_EXPANSION(a25)
#define ZZZ_POD_EXP26(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21,  \
	a22, a23, a24, a25, a26)                                                                                           \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a5)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a6)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a7)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a8)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a9)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a10)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a11)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a12)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a13)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a14)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a15)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a16) ZZZ_PER_POD_EXPANSION(a17) ZZZ_PER_POD_EXPANSION(a18) ZZZ_PER_POD_EXPANSION(a19)        \
		ZZZ_PER_POD_EXPANSION(a20) ZZZ_PER_POD_EXPANSION(a21) ZZZ_PER_POD_EXPANSION(a22) ZZZ_PER_POD_EXPANSION(a23)    \
			ZZZ_PER_POD_EXPANSION(a24) ZZZ_PER_POD_EXPANSION(a25) ZZZ_PER_POD_EXPANSION(a26)
#define ZZZ_POD_EXP27(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21,  \
	a22, a23, a24, a25, a26, a27)                                                                                      \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a5)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a6)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a7)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a8)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a9)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a10)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a11)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a12)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a13)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a14)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a15)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a16) ZZZ_PER_POD_EXPANSION(a17) ZZZ_PER_POD_EXPANSION(a18) ZZZ_PER_POD_EXPANSION(a19)        \
		ZZZ_PER_POD_EXPANSION(a20) ZZZ_PER_POD_EXPANSION(a21) ZZZ_PER_POD_EXPANSION(a22) ZZZ_PER_POD_EXPANSION(a23)    \
			ZZZ_PER_POD_EXPANSION(a24) ZZZ_PER_POD_EXPANSION(a25) ZZZ_PER_POD_EXPANSION(a26)                           \
				ZZZ_PER_POD_EXPANSION(a27)
#define ZZZ_POD_EXP28(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21,  \
	a22, a23, a24, a25, a26, a27, a28)                                                                                 \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a5)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a6)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a7)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a8)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a9)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a10)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a11)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a12)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a13)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a14)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a15)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a16) ZZZ_PER_POD_EXPANSION(a17) ZZZ_PER_POD_EXPANSION(a18) ZZZ_PER_POD_EXPANSION(a19)        \
		ZZZ_PER_POD_EXPANSION(a20) ZZZ_PER_POD_EXPANSION(a21) ZZZ_PER_POD_EXPANSION(a22) ZZZ_PER_POD_EXPANSION(a23)    \
			ZZZ_PER_POD_EXPANSION(a24) ZZZ_PER_POD_EXPANSION(a25) ZZZ_PER_POD_EXPANSION(a26)                           \
				ZZZ_PER_POD_EXPANSION(a27) ZZZ_PER_POD_EXPANSION(a28)
#define ZZZ_POD_EXP29(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21,  \
	a22, a23, a24, a25, a26, a27, a28, a29)                                                                            \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a5)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a6)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a7)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a8)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a9)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a10)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a11)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a12)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a13)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a14)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a15)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a16) ZZZ_PER_POD_EXPANSION(a17) ZZZ_PER_POD_EXPANSION(a18) ZZZ_PER_POD_EXPANSION(a19)        \
		ZZZ_PER_POD_EXPANSION(a20) ZZZ_PER_POD_EXPANSION(a21) ZZZ_PER_POD_EXPANSION(a22) ZZZ_PER_POD_EXPANSION(a23)    \
			ZZZ_PER_POD_EXPANSION(a24) ZZZ_PER_POD_EXPANSION(a25) ZZZ_PER_POD_EXPANSION(a26)                           \
				ZZZ_PER_POD_EXPANSION(a27) ZZZ_PER_POD_EXPANSION(a28) ZZZ_PER_POD_EXPANSION(a29)
#define ZZZ_POD_EXP30(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21,  \
	a22, a23, a24, a25, a26, a27, a28, a29, a30)                                                                       \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a5)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a6)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a7)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a8)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a9)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a10)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a11)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a12)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a13)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a14)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a15)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a16) ZZZ_PER_POD_EXPANSION(a17) ZZZ_PER_POD_EXPANSION(a18) ZZZ_PER_POD_EXPANSION(a19)        \
		ZZZ_PER_POD_EXPANSION(a20) ZZZ_PER_POD_EXPANSION(a21) ZZZ_PER_POD_EXPANSION(a22) ZZZ_PER_POD_EXPANSION(a23)    \
			ZZZ_PER_POD_EXPANSION(a24) ZZZ_PER_POD_EXPANSION(a25) ZZZ_PER_POD_EXPANSION(a26)                           \
				ZZZ_PER_POD_EXPANSION(a27) ZZZ_PER_POD_EXPANSION(a28) ZZZ_PER_POD_EXPANSION(a29)                       \
					ZZZ_PER_POD_EXPANSION(a30)
#define ZZZ_POD_EXP31(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21,  \
	a22, a23, a24, a25, a26, a27, a28, a29, a30, a31)                                                                  \
	ZZZ_PER_POD_EXPANSION(a1)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a2)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a3)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a4)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a5)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a6)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a7)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a8)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a9)                                                                                          \
	ZZZ_PER_POD_EXPANSION(a10)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a11)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a12)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a13)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a14)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a15)                                                                                         \
	ZZZ_PER_POD_EXPANSION(a16) ZZZ_PER_POD_EXPANSION(a17) ZZZ_PER_POD_EXPANSION(a18) ZZZ_PER_POD_EXPANSION(a19)        \
		ZZZ_PER_POD_EXPANSION(a20) ZZZ_PER_POD_EXPANSION(a21) ZZZ_PER_POD_EXPANSION(a22) ZZZ_PER_POD_EXPANSION(a23)    \
			ZZZ_PER_POD_EXPANSION(a24) ZZZ_PER_POD_EXPANSION(a25) ZZZ_PER_POD_EXPANSION(a26)                           \
				ZZZ_PER_POD_EXPANSION(a27) ZZZ_PER_POD_EXPANSION(a28) ZZZ_PER_POD_EXPANSION(a29)                       \
					ZZZ_PER_POD_EXPANSION(a30) ZZZ_PER_POD_EXPANSION(a31)
