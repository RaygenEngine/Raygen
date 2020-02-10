#pragma once

#define RGN_WITH_LOG(X)                           (RGN_WITH_LOG_PRIVATE_DEF_##X() && RGN_WITH_LOG_PRIVATE_DEF_LOG_ANY())
#define RGN_WITH_LOG_PRIVATE_DEF_CLOG_ABORT()     1
#define RGN_WITH_LOG_PRIVATE_DEF_LOG_BELOW_WARN() 1
#define RGN_WITH_LOG_PRIVATE_DEF_LOG_ANY()        1

#define RGN_WITH_FEATURE(X)                         (RGN_WITH_FEATURE_PRIVATE_DEF_##X())
#define RGN_WITH_FEATURE_PRIVATE_DEF_FAST_RELEASE() 0


#ifdef RGN_NO_LOG
#	undef RGN_WITH_LOG_PRIVATE_DEF_LOG_ANY
#	define RGN_WITH_LOG_PRIVATE_DEF_LOG_ANY() 0
#endif

#ifdef RGN_FAST_RELEASE
#	undef RGN_WITH_LOG_PRIVATE_DEF_CLOG_ABORT
#	define RGN_WITH_LOG_PRIVATE_DEF_CLOG_ABORT() 0

#	undef RGN_WITH_LOG_PRIVATE_DEF_LOG_BELOW_WARN
#	define RGN_WITH_LOG_PRIVATE_DEF_LOG_BELOW_WARN() 0

#	undef RGN_WITH_FEATURE_PRIVATE_DEF_FAST_RELEASE
#	define RGN_WITH_FEATURE_PRIVATE_DEF_FAST_RELEASE() 1
#endif

// CONCEPTS INTELLISENSE WORKAROUND
// TODO: when intellisense supports concepts replace ALL uses of the macro and deprecate
// parentheses omited on purpose
#ifdef __INTELLISENSE__
#	define REQUIRES(...)
#	define CONC(...) typename // Use when inlining concept types. Ugly :(
#else
#	define REQUIRES(...) requires __VA_ARGS__
#	define CONC(...)     __VA_ARGS__
#endif
