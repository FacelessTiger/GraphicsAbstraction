project "CobraRHIShared"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	editandcontinue "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/ImGuiLayer.cpp"
	}

	includedirs
	{
		"src",
		"%{wks.location}/include",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.GLFW}"
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