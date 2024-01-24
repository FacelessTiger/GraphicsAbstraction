include "/vendor/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

newoption {
	trigger = "vulkan",
	description = "Force the use of Vulkan for rendering, regardless of platform"
}

newoption {
	trigger = "directx",
	description = "Force the use of DirectX for rendering, regardless of platform"
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
	include "vendor/imgui"
	include "vendor/fastgltf.lua"
group ""

group "Core"
	include "CobraRHIDirectX"
	include "CobraRHIVulkan"
	include "CobraRHIShared"
	include "includeFiles.lua"
	include "RendererExample"
group ""