project "CobraRHIDirectX"
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
		"%{IncludeDir.AgilitySDK}",

		"%{IncludeDir.glm}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.xxHash}"
	}

	links
	{
		"CobraRHIShared",
		"%{Library.DirectX}",
		"%{Library.DXGI}",
		"%{Library.DXCompiler}"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "GA_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "GA_RELEASE"

	filter "configurations:Dist"
		defines "GA_DIST"

	filter "configurations:Release or Dist"
		runtime "Release"
		optimize "on"