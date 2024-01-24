project "CobraRHIVulkan"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp",
		"%{wks.location}/vendor/xxHash/**.h",
		"%{wks.location}/vendor/xxHash/**.c"
	}

	defines
	{
		"GA_BUILD_DLL"
	}

	postbuildcommands
    {
    	("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/RendererExample/\"")
    }

	includedirs
	{
		"src",
		"%{wks.location}/include",
		"%{wks.location}/CobraRHIShared/src",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.vma}",

		"%{IncludeDir.glm}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.xxHash}"
	}

	links
	{
		"CobraRHIShared",
		"%{Library.Vulkan}",
		"%{Library.DXCompiler}"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "GA_DEBUG"
		runtime "Debug"
		symbols "on"

		links
		{
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}"
		}

	filter "configurations:Release or Dist"
		runtime "Release"
		optimize "on"

		links
		{
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}

	filter "configurations:Release"
		defines "GA_RELEASE"

	filter "configurations:Dist"
		defines "GA_DIST"