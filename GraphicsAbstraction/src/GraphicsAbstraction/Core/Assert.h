#pragma once

#include <GraphicsAbstraction/Core/Core.h>
#include <GraphicsAbstraction/Core/Log.h>

#include <filesystem>

#ifdef GA_ENABLE_ASSERTS
	#define GA_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { GA##type##ERROR(msg, __VA_ARGS__); GA_DEBUGBREAK(); } }
	#define GA_INTERNAL_ASSERT_WITH_MSG(type, check, ...) GA_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
	#define GA_INTERNAL_ASSERT_NO_MSG(type, check) GA_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", GA_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

	#define GA_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
	#define GA_INTERNAL_ASSERT_GET_MACRO(...) GA_EXPAND_MACRO( GA_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, GA_INTERNAL_ASSERT_WITH_MSG, GA_INTERNAL_ASSERT_NO_MSG) )

	// Currently accepts at least the condition and one additional parameter (the message) being optional
	#define GA_CORE_ASSERT(...) GA_EXPAND_MACRO( GA_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
	#define GA_CORE_ASSERT(...)
#endif