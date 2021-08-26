#pragma once

#include "engine/MulticastEvent.h"

namespace Event {
// int32 width, int32 height
inline MulticastEvent<int32, int32> OnWindowResize; // @1: width, @2: height
inline MulticastEvent<> OnViewportUpdated;          //
inline MulticastEvent<bool> OnWindowFocus;          // @1: newIsFocused
inline MulticastEvent<bool> OnWindowMinimize;       // @1: newIsMinimized
inline MulticastEvent<bool> OnWindowMaximize;       // @1: newIsMaximized
inline MulticastEvent<> OnViewerUpdated;            //

} // namespace Event
