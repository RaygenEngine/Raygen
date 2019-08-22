#ifndef TIMER_H
#define TIMER_H

#include <chrono>
// TODO: make or use an actual timer
#define TIMING
#ifdef TIMING
#define INIT_TIMER auto start___ = std::chrono::high_resolution_clock::now()
#define START_TIMER  start___ = std::chrono::high_resolution_clock::now()
#define STOP_TIMER(name)  RT_XENGINE_LOG_AT_LOWEST_LEVEL("{0}: {1} ms", name,  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-start___).count())
#else
#define TIME_FUNCTION(func, ...)
#define INIT_TIMER
#define START_TIMER
#define STOP_TIMER(name)
#endif

#endif // TIMER_H
