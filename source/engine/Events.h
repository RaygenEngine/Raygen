#pragma once

#include "engine/MulticastEvent.h"

class Node;
class CameraNode;
namespace Event {
// int32 width, int32 height
inline MulticastEvent<int32, int32> OnWindowResize; // @1: width, @2: height
inline MulticastEvent<> OnViewportUpdated;          //
inline MulticastEvent<bool> OnWindowFocus;          // @1: newIsFocused
inline MulticastEvent<bool> OnWindowMinimize;       // @1: newIsMinimized

inline MulticastEvent<> OnWorldLoaded;
inline MulticastEvent<Node*> OnWorldNodeAdded;
inline MulticastEvent<Node*> OnWorldNodeRemoved;
inline MulticastEvent<CameraNode*> OnWorldActiveCameraChanged;
} // namespace Event
