VULKAN_SDK = os.getenv("VULKAN_SDK")
VULKAN_EXTENSION_LAYERS = os.getenv("VK_LAYER_PATH");

IncludeDir = {}
IncludeDir["stb_image"] = "%{wks.location}/vendor/stb_image"
IncludeDir["GLFW"] = "%{wks.location}/vendor/GLFW/include"
IncludeDir["ImGui"] = "%{wks.location}/vendor/imgui"
IncludeDir["glm"] = "%{wks.location}/vendor/glm"
IncludeDir["spdlog"] = "%{wks.location}/vendor/spdlog/include"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["vma"] = "%{wks.location}/vendor/vma/include"
IncludeDir["DXC"] = "%{wks.location}/vendor/DXC/include"
IncludeDir["xxHash"] = "%{wks.location}/vendor/xxHash"
IncludeDir["fastgltf"] = "%{wks.location}/vendor/fastgltf/include"
IncludeDir["AgilitySDK"] = "%{wks.location}/vendor/AgilitySDK/include"

LibraryDir = {}

LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["DXC"] = "%{wks.location}/vendor/DXC/lib"

Library = {}

Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["DirectX"] = "d3d12.lib"
Library["DXGI"] = "dxgi.lib"

Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_combined.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"

Library["DXCompiler"] = "%{LibraryDir.DXC}/dxcompiler.lib"

-- Windows
Library["WinSock"] = "Ws2_32.lib"
Library["WinMM"] = "Winmm.lib"
Library["WinVersion"] = "Version.lib"
Library["BCrypt"] = "BCrypt.lib"
Library["dbghelp"] = "dbghelp.lib"