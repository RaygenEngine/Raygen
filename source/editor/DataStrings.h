#pragma once
// DOC:

// For editor use only, contains the actual text pieces for the editor help and other stuff.

inline const char* txt_about = R"(
License: TBD
Copyright (c) 2019-2020 John Moschos, Harry Katagis

)";

// this text autowraps around 70 chars for optimal reading
inline const char* txt_help = R"(
The editor is designed primarily for debugging and advanced users. Multiple properties can be set to "incorrect" values and rarely some may crash your system. You can for example set a negative texture size, or a 128k by 128k one so use responsibly.
Also note that the editor has not been thoroughly profiled and may introduce slight performance penalties while the windows are open and visible. The editor can be toggled at any time using the ` key.

Scene saving and loading is available and the scene format is in json.
All the reflected properties of nodes are automatically serialized and restored upon loading. Any asset referenced in the scene file is stored relative to the Raygen/assets/ folder and the scene will fail to load if it is not found.
Editing the json files by hand is possible but not recommended.
)";

inline const char* help_Outliner = R"(
Outliner provides an outline of the current world hierarchy.

Select nodes by clicking to open their property editor.
Right click nodes to open their context menu.
Drag-n-drop a node onto another to change the hierarchy.
)";

inline const char* help_AssetView = R"(
Assets panel provides an overview of all the currently registered assets.

The data residing in memory can be unloaded and reloaded at will.
)";

inline const char* help_EditorCamera = R"(
Editor camera is a special node managed by the editor to allow for editor specific extra functionality.

Keybinds for editor camera are:
W,A,S,D,Q,E movement
Right-click drag: facing
Shift, Ctrl: Speed up, speed down
X: toggle between axis aligned, view based movement
C: reset orientation (resets roll too)
F: Focus currently selected node
Shift+F: Toggle Piloting selected node.

The editor camera differs from other nodes in the following ways:

* It is not saved or loaded in scene files
* It updates even when the world is not updating
* It is automatically removed or added when toggling world updates
* It cannot have children
* It automatically moves its parent
* Context actions are disabled
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
Unloads all currently filtered assets. 
Some assets may reload instantly because they are accessed.
)";

inline const char* help_AssetReloadUnloaded = R"(
Reloads any filtered assets currently unloaded.
)";

inline const char* help_AssetReloadAll = R"(
Reloads all currently filtered assets.
)";

inline const char* help_AssetFilter = R"(
Filter assets, can accept multiple terms by ',' and negative matches by '-'
eg: '-mat, .gltf'
)";


inline const char* help_GltfWindowRefresh = R"(
This window contains all .gtlf files found in assets/ folders.
Duplicate names are shown once.

Contents of this window are cached, refresh to detect new files / changes in the filesystem.
)";

inline const char* help_PropMassEditMats = R"(
Toggles parallel editing of materials in this list by editing any one of them.
Only the edited properties will propagate to the rest of the materials.
)";

inline const char* help_PropMassRestoreMats = R"(
Mass reloads the materials to restore their original state after mass editing.
)";
