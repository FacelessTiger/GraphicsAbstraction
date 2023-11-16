#pragma once

#include <GraphicsAbstraction/Core/PlatformDetection.h>

#ifdef GA_DEBUG
	#define GA_ENABLE_ASSERTS
#endif

#if defined(GA_PLATFORM_WINDOWS)
	#define GA_DEBUGBREAK() __debugbreak()
#elif defined (GA_PLATFORM_LINUX)
	#include <signal.h>
	#define GA_DEBUGBREAK() raise(SIGTRAP)
#else
	#error "Platform doesn't support debugbreak yet!"
#endif

#define GA_EXPAND_MACRO(x) x
#define GA_STRINGIFY_MACRO(x) #x

#define GA_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#include <GraphicsAbstraction/Core/Log.h>
#include <GraphicsAbstraction/Core/Assert.h>