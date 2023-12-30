include "/vendor/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

newoption {
	trigger = "with-vulkan",
	description = "Force the use of Vulkan for rendering, regardless of platform"
}

workspace "GraphicsAbstraction"
	architecture "x86_64"
	startproject "RendererExample"
	
	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "vendor/premake"
	include "GraphicsAbstraction/vendor/GLFW"
	include "GraphicsAbstraction/vendor/imgui"
	include "GraphicsAbstraction/vendor/tracy"
	include "GraphicsAbstraction/vendor/fastgltf.lua"
group ""

group "Core"
	include "GraphicsAbstraction"
	include "RendererExample"
group ""