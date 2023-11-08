include "/vendor/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

workspace "VulkanAbstractionPractice"
	architecture "x86_64"
	startproject "VulkanAbstractionPractice"
	
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
	include "VulkanAbstractionPractice/vendor/GLFW"
	include "VulkanAbstractionPractice/vendor/imgui"
group ""

group "Core"
	include "VulkanAbstractionPractice"
group ""