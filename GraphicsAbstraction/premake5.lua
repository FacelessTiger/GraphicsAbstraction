project "GraphicsAbstraction"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	editandcontinue "Off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	disablewarnings { "4996" }
	linkoptions { "-IGNORE:4099", "-IGNORE:4006" }

	files
	{
		"src/GraphicsAbstraction/**.h",
		"src/GraphicsAbstraction/**.cpp",
		"vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",
		"vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl",
		"vendor/xxHash/**.h",
		"vendor/xxHash/**.c"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
		"SPDLOG_WCHAR_TO_UTF8_SUPPORT"
	}

	includedirs
	{
		"src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.xxHash}"
	}

	links 
	{
		"GLFW",
		"ImGui",
	}

	filter "options:vulkan"
		defines "GA_RENDERER_VULKAN"

		files
		{
			"src/Platform/GraphicsAPI/Vulkan/**.h",
			"src/Platform/GraphicsAPI/Vulkan/**.cpp"
		}

		includedirs
		{
			"%{IncludeDir.VulkanSDK}",
			"%{IncludeDir.DXC}",
			"%{IncludeDir.vma}"
		}

		links
		{
			"%{Library.Vulkan}",
			"%{Library.DXCompiler}"
		}

	filter "options:directx"
		defines "GA_RENDERER_DIRECTX"

		files
		{
			"src/Platform/GraphicsAPI/D3D12/**.h",
			"src/Platform/GraphicsAPI/D3D12/**.cpp"
		}

		includedirs
		{
			"%{IncludeDir.DXC}"
		}

		links
		{
			"%{Library.DirectX}",
			"%{Library.DXGI}",
			"%{Library.DXCompiler}"
		}

	filter { "options:vulkan", "configurations:Debug" }
		links
		{
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}"
		}

	filter { "options:vulkan", "configurations:Release or Dist" }
		links
		{
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