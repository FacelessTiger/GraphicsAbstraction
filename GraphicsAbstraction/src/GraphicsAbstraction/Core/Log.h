#pragma once

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace GraphicsAbstraction {

	class Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
	};

}

#define GA_CORE_TRACE(...)	::GraphicsAbstraction::Log::GetCoreLogger()->trace(__VA_ARGS__);
#define GA_CORE_INFO(...)	::GraphicsAbstraction::Log::GetCoreLogger()->info(__VA_ARGS__);
#define GA_CORE_WARN(...)	::GraphicsAbstraction::Log::GetCoreLogger()->warn(__VA_ARGS__);
#define GA_CORE_ERROR(...)	::GraphicsAbstraction::Log::GetCoreLogger()->error(__VA_ARGS__);
#define GA_CORE_FATAL(...)	::GraphicsAbstraction::Log::GetCoreLogger()->critical(__VA_ARGS__);