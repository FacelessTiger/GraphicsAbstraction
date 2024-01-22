# CobraRHI

This is an RHI I've been making mainly for my personal game engine, Cobra. But I've designed it to work generally so it can be used in other projects. 

Once it's built to use it link with CobraRHIShared and then either CobraRHIVulkan if you want vulkan, and CobraRHIDirectX if you want directx (you must also include GraphicsAbstraction/Core/DirectXExport.h in a cpp file). Currently you're also required to have glfw and glm (imgui is optional), I will work on removing these dependencies in the future.
