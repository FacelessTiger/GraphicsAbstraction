project "D3D12MA"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	systemversion "latest"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"D3D12MA/src/D3D12MemAlloc.cpp",
		"D3D12MA/include/D3D12MemAlloc.h"
	}

	includedirs
	{
		"D3D12MA/include"
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		runtime "Release"
		optimize "on"
        symbols "off"