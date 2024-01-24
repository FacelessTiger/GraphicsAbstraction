project "CobraRHIShared"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/ImGuiLayer.cpp",
		"src/CobraIncludeHandler.h",
		"src/DxcCompiler.h",
		"src/DxcCompiler.cpp"
	}

	includedirs
	{
		"src",
		"%{wks.location}/include",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.DXC}"
	}

	links
	{
		"%{Library.DXCompiler}"
	}

	filter "system:windows"
		systemversion "latest"

		files
		{
			"src/WindowsWindow.cpp"
		}

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