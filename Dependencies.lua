VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["stb_image"] = "%{wks.location}/VulkanAbstractionPractice/vendor/stb_image"
IncludeDir["GLFW"] = "%{wks.location}/VulkanAbstractionPractice/vendor/GLFW/include"
IncludeDir["ImGui"] = "%{wks.location}/VulkanAbstractionPractice/vendor/imgui"
IncludeDir["glm"] = "%{wks.location}/VulkanAbstractionPractice/vendor/glm"
IncludeDir["VKBootstrap"] = "%{wks.location}/VulkanAbstractionPractice/vendor/VkBootstrap/src"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"

LibraryDir = {}

LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

Library = {}

Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK}/shaderc_combinedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_combined.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"