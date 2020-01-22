![Ragyen](/assets/engine-data/logo.png?style=centerme "Raygen")

# Raygen

Raygen is a graphics engine focused on exploring real-time use cases of ray tracing.
The engine is written in modern C++ and uses the Vulkan API. 


## Getting started

```
 git clone --recursive --shallow-submodules https://github.com/Renoras/Raygen
 cd Raygen
 mkdir build
 cd build
 cmake ..
 ```

If you are using the .sln to build remember to change the startup project to Raygen.

## Dependencies (included as submodules)

* [glm](https://github.com/g-truc/glm)
* [ImGui](https://github.com/ocornut/imgui)
* [spdlog](https://github.com/gabime/spdlog)
* [stb](https://github.com/nothings/stb)
* [tinygltf](https://github.com/syoyo/tinygltf)
* [nlohmann/json](https://github.com/nlohmann/json)
* [magic_enum](https://github.com/Neargye/magic_enum)

## Authors

| Info | Role |
| ------|-----|
|**John Moschos**, [Renoras](https://github.com/Renoras)| Co-Founder, Programmer |
|**Harry Katagis**, [katagis](https://github.com/katagis)| Co-Founder, Programmer |
