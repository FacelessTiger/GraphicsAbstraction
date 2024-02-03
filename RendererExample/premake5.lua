project "RendererExample"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

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
		"%{wks.location}/include",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.fastgltf}"
	}

	links
	{
		"CobraRHIShared",
		"fastgltf",
		"ImGui"
	}

	filter "options:vulkan"
		links
		{
			"CobraRHIVulkan"
		}

	filter "options:directx"
		links
		{
			"CobraRHIDirectX"
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