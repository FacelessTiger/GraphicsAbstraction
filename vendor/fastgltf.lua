project "fastgltf"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	systemversion "latest"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	http.download("https://raw.githubusercontent.com/simdjson/simdjson/v3.3.0/singleheader/simdjson.h", "fastgltf/deps/simdjson.h", { timeout = 10 })
	http.download("https://raw.githubusercontent.com/simdjson/simdjson/v3.3.0/singleheader/simdjson.cpp", "fastgltf/deps/simdjson.cpp", { timeout = 10 })

	files
	{
		"fastgltf/src/**.cpp",
		"fastgltf/include/**.hpp",
		"fastgltf/deps/**.h",
		"fastgltf/deps/**.cpp"
	}

	includedirs
	{
		"fastgltf/include",
		"fastgltf/deps"
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