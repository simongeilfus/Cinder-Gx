## Cinder-Gx

So far the philosophy has been to wrap as little as possible. The `Diligent` namespace has been aliased inside the `cinder::graphics` namespace for convenience and some of the common interfaces have been typedefed. Apps created with the `RendererGx` renderer have a default immediate context (Direct3D11 style) that has been conveniently exposed as global functions, which allows easy access to things such as `gx::clear`, `gx::draw`, etc... Most of that work lives inside `cinder/graphics/wrapper.h`

Just like Vulkan and DX12, `Diligent` functions take all sort of very verbose `Desc` and `CreateInfo` objects which tends to make any initialization long and sometimes tedious. Those classed have been lightly wrapped through inheritance to give them a more familiar "named parameter" interface. Inheritance allows compatibility with raw `Diligent` and wrapping those classes gives us more flexibility on what we think are reasonnable default values (which seems to be already the case in Diligent most of the time). 

#### Build Instructions

Clone the repository recursively into Cinder's blocks folder.

Build **DiligentCore** and **DiligentTools** with the following. *Vulkan_LIBRARY* and *Vulkan_INCLUDE_DIR* have to point to your local [*VulkanSDK*](https://vulkan.lunarg.com/) installation. Recent VulkanSDK and Windows SDK should be installed for recent features to work (ex. MeshShaders and Raytracing samples might not work without).

```shell
cmake -S . -B ./build -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release -DVulkan_INCLUDE_DIR="D:\VulkanSDK\1.2.162.1\Include" -DVulkan_LIBRARY="D:\VulkanSDK\1.2.162.1\Lib\vulkan-1.lib" -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>" -DCMAKE_POLICY_DEFAULT_CMP0091:STRING=NEW
cmake --build build --config Release
cmake --build build --config Debug
```

#### Tinderbox

The block is compatible with tinderbox and has a basic template that can be used instead of the OpenGL template.

#### RendererGx / prepareEngine

`RendererGx` exposes a few of the underlying structures through both member and free functions such as `getImmediateContext`, `getRenderDevice` or `getSwapChain`. Hopefully this would be for more advanced use cases and convenience paths should take care of frequent uses.

Currently there's very little options exposed to `RendererGx` apart from the API backend. I've added a `prepareEngine` callback to access the engine option after the API choice has been done. I think this is to be able to react to potential fallback on platforms that don't necessarely support VK or DX12. It currently has to be implemented as a free or static function just like `prepareSettings`.

#### DearImGui

ImGui is supported via `#include "cinder/CinderDiligentImGui.h"` and its own `ImGui::DiligentInitialize();`. Textures can be displayed just like `gl::TextureRef` can in the base cinder implementation.

#### Textures

Very basic and incomplete support for texture loading from `Surface`, `Channel` and `ImageSource`. Currently adding to this on a per-need basis. Could definitely benefit some more love.

#### PipelineState

As this becomes the main way of describing any graphic task, the wrapper for this is a bit heavier than elsewhere, maybe too heavy? I think it would be great to have some sort of system to ease even more the creation of pipelines. Maybe be a series of default common pipelines or a system *à la* `ShaderDef`.

#### DrawContext

`DrawContext` is a small prototype of convenience drawing functions. It is meant to bring the same interface as `cinder/gl/draw.h` and part of `cinder/gl/wrapper.h` in a non *immediate-mode* context. This is very much a work in progress but I believe has an already strong base. More on this soon. There's an app using it in the `tests` folder.

#### Mesh & Batch

Prototypes of `cinder::gl::VboMesh/Batch` equivalent interface. Obviously `Batch` responsability becomes much larger as shaders are not linked at a program level but at a pipeline level. Which clearly brings tons of other state related things along. `Batch` also owns a `ShaderResourceBinding` object which is `Diligent`'s way of dealing with descriptors. Hopefully `Batch` brings some balance between easy to use but still complete and flexible for more advanced tasks.

More soon!
