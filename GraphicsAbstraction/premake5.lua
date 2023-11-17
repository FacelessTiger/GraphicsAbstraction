project "GraphicsAbstraction"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	editandcontinue "Off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/GraphicsAbstraction/**.h",
		"src/GraphicsAbstraction/**.cpp",
		"src/Main.cpp",
		"vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",
		"vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE"
	}

	includedirs
	{
		"src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.spdlog}"
	}

	links 
	{
		"GLFW",
		"ImGui",
	}

	filter "options:with-vulkan"
		defines "GA_RENDERER_VULKAN"

		files
		{
			"src/Platform/GraphicsAPI/Vulkan/**.h",
			"src/Platform/GraphicsAPI/Vulkan/**.cpp",
			"vendor/VkBootstrap/src/**.h",
			"vendor/VkBootstrap/src/**.cpp"
		}

		includedirs
		{
			"%{IncludeDir.VulkanSDK}",
			"%{IncludeDir.VKBootstrap}"
		}

		links
		{
			"%{Library.Vulkan}"
		}

	filter { "options:with-vulkan", "configurations:Debug" }
		links
		{
			"%{Library.ShaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}"
		}

	filter { "options:with-vulkan", "configurations:Release or Dist" }
		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}

	filter "system:windows"
		systemversion "latest"

		links
		{
			"%{Library.WinSock}",
			"%{Library.WinMM}",
			"%{Library.WinVersion}",
			"%{Library.BCrypt}",
			"%{Library.dbghelp}"
		}

		files
		{
			"src/Platform/OperatingSystem/Windows/**.h",
			"src/Platform/OperatingSystem/Windows/**.cpp"
		}

	filter "configurations:Debug"
		defines "GA_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "GA_RELEASE"

	filter "configurations:Debug or Release"
		links
		{
			"tracy"
		}

		includedirs
		{
			"%{IncludeDir.tracy}"
		}

		defines
		{
			"TRACY_ENABLE",
			"TRACY_ON_DEMAND"
		}

	filter "configurations:Dist"
		defines "GA_DIST"

	filter "configurations:Release or Dist"
		runtime "Release"
		optimize "on"