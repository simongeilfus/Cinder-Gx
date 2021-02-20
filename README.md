## Cinder-Gx

Clone the repository recursively into Cinder's blocks folder.

#### Build Instructions

Build **DiligentCore** and **DiligentTools** with the following. *Vulkan_LIBRARY* and *Vulkan_INCLUDE_DIR* have to point to your local [*VulkanSDK*](https://vulkan.lunarg.com/) installation. Recent VulkanSDK and Windows SDK should be installed for recent features to work (ex. MeshShaders and Raytracing samples might not work without).

```shell
cmake -S . -B ./build -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release -DVulkan_INCLUDE_DIR="D:\VulkanSDK\1.2.162.1\Include" -DVulkan_LIBRARY="D:\VulkanSDK\1.2.162.1\Lib\vulkan-1.lib" -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>" -DCMAKE_POLICY_DEFAULT_CMP0091:STRING=NEW
cmake --build build --config Release
cmake --build build --config Debug
```

