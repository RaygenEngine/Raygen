
# Engine



# Modules Quick View
## Engine Object

* Hosts the available engine modules and manages their lifetimes
* Provides public accessors for inter-module communication
* Supports utilities and actions that require cross module state

*Comment: To avoid boilerplate we decided to make the engine __()*

## CPU Asset Manager

* Unifies and handles disk assets and cpu generated assets
* Stores a unique hash of the virtual asset path in order to achieve automatic caching
* Performing queries that resolve to an existsing asset path 
* Assets are immutable pods whose data can be accessed by other modules through handles
* Pod handles __provide a type-safe approach to access potentially unloaded pods
* Multiple handles can point to the same asset entry 


## World

* Contains the scene representation (scene-graph)
* World nodes use an update system based on generic and type-specific dirty flags
* Provides public access (through iterators) to any type-specific subset of the scene-graph's nodes (eg: __)
* Saving and loading is done by serialization and deserialization of each node's properties
* Node transformation and bounding box data is calculated and cached automatically

*Comment: The world module is designed to be used by any renderer. Nodes do not implement any rendering logic. Every node is a child of an implicit transform node. (For the time being this approach reduces runtime costs and provides a type-independant way of calculating world math.) The scene-graph at the time of writing does not support automatic instancing.*

## Renderer Interface

* Provides a generic interface for rendering
* A renderer's lifetime is managed by the engine
* The basic functions of the renderer interface are Init and Render

*Comment: The renderer designer has the freedom to refrain form supporting any engine feature. (eg: A renderer can use a snapshot of the current state of the world to create a static scene.)*

### Observer Renderer

* Offers an easy approach to construct dynamic renderers that track world state changes
* Automatically handles and updates renderer objects/node observers that correspond to world nodes (1 to 1)
* A node observer keeps a reference to a world node and updates its state based on the same dirty flag system that nodes use

*Comment: This is the recommended way of writing renderers as by default adding and removing nodes is handled automatically.*

## Editor

* ImGui based UI
* Can be toggled during runtime.
* Provides visualization and editing of reflected member variables for nodes and assets
* Adding, removing and editing the hierarchy of scenes is supported.

*Comment: ImGui provides an intermediate drawing format. The renderer designer can opt-in and implement it. Major rendering APIs and specifications can use similar implementation of ImGui drawing (see GLEditorRenderer).*

