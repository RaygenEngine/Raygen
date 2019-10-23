#pragma once
// For editor use only, contains the actual text pieces for the editor help and other stuff.

inline const char* txt_about = R"(
About text can be edited at editor/DataStrings.h

Licensed under: ...
)";

// this text autowraps around 70 chars for optimal reading
inline const char* txt_help = R"(
Programming documentation can be found at docs/ in Markdown format or viewed directly on __github. In this window you can get an overview of Rayxen's features, design logic and editor basics.

Rayxen has been primarily designed with the goal of being a sandbox for renderer development. Therefore all the design logic is based on __ basic principles:
1. All rendering features are opt-in.
2. __

The tools provided by the engine for Renderer Designers are:
1. Extensible automatic asset handling with caching.
2. Integrated reflection system with a powerfull editor.
3. Hotswappable sample renderers for comparing and debugging.
__


Editor Help:
The editor is designed primarily for debugging and advanced users. Multiple properties can be set to "incorrect" values and rarely some may crash your system. You can for example set a negative texture size, or a 128k by 128k one so use responsibly.
Also note that the editor has not been thoroughly profiled and may induce slight performance penalties while the windows are open and visible. The editor can be toggled at any time using the ` key.


Scene saving and loading is available and the scene format is in json.
All the reflected properties of nodes are automatically serialized and restored upon loading. Any asset refrenced in the scene file is stored relative to the Rayxen/assets/ folder and the scene will fail to load if it is not found.
Editing the json files by hand possible but not recommended.


Some help tooltips are available in the editor covering specific functions. These are not included here __(why).

)";

inline const char* help_Outliner = R"(
outliner:
)";

inline const char* help_PropertyEditor = R"(
propeditor:
)";

inline const char* help_AssetView = R"(
assetview:
)";

inline const char* help_EditorCamera = R"(
editor camera:
)";

inline const char* help_UpdateWorld = R"(
Enables the update functions of the world nodes, and disables the editor camera.
Some editor functionality like piloting may not work because they require the editor camera.

To discard the state after running updates, use restore update state.
)";

inline const char* help_RestoreWorld = R"(
Toggles automatic reloading of the world state after 'stoping' updating.
)";

inline const char* help_AssetUnloadAll = R"(
DOC:
)";

inline const char* help_AssetReloadUnloaded = R"(
DOC:
)";

inline const char* help_AssetReloadAll = R"(
DOC:
)";

inline const char* help_AssetFilter = R"(
DOC:
)";

inline const char* help_AssetSearchGltf = R"(
Gltf Search:
)";

inline const char* help_GltfWindowRefresh = R"(
Gltf Refresh:
)";

inline const char* help_PropPodEditing = R"(

WARNING:
Editing asset pod properties is currently only partially supported.

The cpu side data are edited in real time, but even the provided renderers will not reflect the changes on most cases.
Also any changes to the pods do not get saved in scene files and will be lost when the asset reloads from disk.
)";


inline const char* help_PropMassEditMats = R"(
Toggles parallel editing of materials in this list by editing any one of them.
Only the edited properties will propagate to the rest of the materials.
)";

inline const char* help_PropMassRestoreMats = R"(
Mass reloads the materials to restore their original state after mass editing.
)";
