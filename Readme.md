![Kaleido](/assets/engine-data/logo.png?style=centerme "Kaleido")

# Kaleido

Kaleido is a graphics engine focused on renderer extensibility, written in modern C++17.
It's primary focus is to support the creation of multiple renderers that can be hot-swapped in real-time, even while the scene is updating.

## Features

* Real-time renderer swapping while scenes are updating
* Sample PBR OpenGL Forward & Deferred renderers
* Dynamic & multi-threaded asset loading & automated caching
* Reflection of world entities with support for enums
* Drag & Drop Scene editor & auto-generated entity editor
* Generic JSON serialization based on reflection

## Features (Alt)


### Rendering
* Hot-swappable renderers of different contexts
* Sample OpenGL Forward PBR & Deferred PBR renderers
* Stereo VR Renderer
* Directional, omni & spot lights with shadows
* Basic post-process
* Frustum culling

### World & Editor
* Extensible type-system with reflection support for user types
* GUI Real-time drag & drop scene editor
* Auto-generated GUI property editor for user types
* JSON Serialization of user types through reflection
* Scene saving and loading 

### Assets
* Runtime multi-threaded asset loading
* Dynamic loading, unloading & automated caching
* Extensible asset loaders
* Support for GLTF, Generic JSON, PNG, JPG, BMP

## Screenshots




## Requirements

The development is done mostly on MSVC 19.22. Clang-cl v9 is also regularly tested. 
CMake 3.11 is required.

## Getting started
[//]: # (TODO: Test --shallow-submodules)

```
 git clone --recursive --shallow-submodules https://github.com/renoras/kaleido
 cd kaleido
 mkdir build
 cd build
 cmake ..
 ```
If you are using the .sln to build remember to change the startup project to Kaleido.
Then just build & run. Everything default assets are included for the preview scene.

## Dependencies (included as submodules)

* [glm](https://github.com/g-truc/glm)
* [ImGui](https://github.com/ocornut/imgui)
* [spdlog](https://github.com/gabime/spdlog)
* [stb](https://github.com/nothings/stb)
* [tinygltf](https://github.com/syoyo/tinygltf)
* [nlohmann/json](https://github.com/nlohmann/json)
* [magic_enum](https://github.com/Neargye/magic_enum)
* glad
* LibOVR

## Authors

| Info | Role | Focus |
| ------|-----|-----|
|**John Moschos**, [Renoras](https://github.com/Renoras)| Co-Founder, Programmer | Graphics & rendering |
|**Harry Katagis**, [katagis](https://github.com/katagis)| Co-Founder, Programmer | Engine Subsystems & Modules |
