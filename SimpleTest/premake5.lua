project "SimpleExample"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	editandcontinue "Off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	disablewarnings { "4996" }
	linkoptions { "-IGNORE:4099" }

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"src",
		"%{wks.location}/GraphicsAbstraction/src",
		"%{wks.location}/GraphicsAbstraction/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.fastgltf}"
	}

	links
	{
		"GLFW",
		"GraphicsAbstraction",
		"fastgltf"
	}

	filter "configurations:Debug"
		defines "GA_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "GA_RELEASE"

	filter "configurations:Debug or Release"
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