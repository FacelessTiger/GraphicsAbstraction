include "/vendor/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

workspace "GraphicsAbstraction"
	architecture "x86_64"
	startproject "GraphicsAbstraction"
	
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
group ""

group "Core"
	include "GraphicsAbstraction"
group ""