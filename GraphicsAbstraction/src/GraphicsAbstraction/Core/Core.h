#pragma once

#include <GraphicsAbstraction/Core/PlatformDetection.h>
#include <cstdint>
#include <utility>
#include <type_traits>

#ifndef GA_DIST
	#define GA_ENABLE_ASSERTS
#endif

#if defined(GA_PLATFORM_WINDOWS)
	#define GA_DEBUGBREAK() __debugbreak()
#elif defined (GA_PLATFORM_LINUX) || defined (GA_PLATFORM_ANDROID)
	#include <signal.h>
	#define GA_DEBUGBREAK() raise(SIGTRAP)
#else
	#error "Platform doesn't support debugbreak yet!"
#endif

#define GA_EXPAND_MACRO(x) x
#define GA_STRINGIFY_MACRO(x) #x

#define GA_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

namespace GraphicsAbstraction {

	template<typename T> class Ref;
	class RefCounted
	{
	private:
		template<typename T> friend class Ref;
		mutable uint32_t m_RefCount = 0;
	};

	template<typename T>
	class Ref
	{
	public:
		static_assert(std::has_virtual_destructor_v<T>, "Template parameter must have a virtual destructor");
		template<typename D> friend class Ref;

		Ref(T* pointer) : m_Value(pointer)
		{
			static_assert(std::is_base_of_v<RefCounted, T>, "Template parameter must inherit from GraphicsAbstraction::RefCounted");
			Increment();
		}

		template<typename D>
		Ref(const Ref<D>& other)
			: m_Value(other.m_Value)
		{
			static_assert(std::is_base_of_v<T, D>, "Type must derive from target type");
			Increment();
		}

		Ref() : m_Value(nullptr) { }
		Ref(std::nullptr_t) : m_Value(nullptr) { }
		Ref(const Ref& other) : m_Value(other.m_Value) { Increment(); }
		Ref(Ref&& other) : m_Value(std::exchange(other.m_Value, nullptr)) { }
		~Ref() { Decrement(); }

		Ref& operator=(std::nullptr_t)
		{
			Decrement();

			m_Value = nullptr;
			return *this;
		}

		Ref& operator=(const Ref& other)
		{
			other.Increment();
			Decrement();
			
			m_Value = other.m_Value;
			return *this;
		}

		Ref& operator=(Ref&& other)
		{
			Decrement();
			
			m_Value = std::exchange(other.m_Value, nullptr);
			return *this;
		}

		T* operator->() { return m_Value; }
		const T* operator->() const { return m_Value; }

		T& operator*() { return *m_Value; }
		const T& operator*() const { return *m_Value; }

		bool operator==(Ref other) const { return m_Value == other.m_Value; }
		bool operator!=(Ref other) const { return m_Value != other.m_Value; }

		operator T* () { return m_Value; }
		operator const T* () const { return m_Value; }
		operator bool() const { return m_Value; }
	private:
		inline void Increment() const { if (m_Value) m_Value->m_RefCount++; }

		inline void Decrement() const
		{ 
			if (m_Value)
			{
				if (!--m_Value->m_RefCount) delete m_Value;
			}
		}
	private:
		T* m_Value;
	};

	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return Ref<T>(new T(std::forward<Args>(args)...));
	}

	template<typename T, typename D>
	constexpr Ref<D> StaticRefCast(const Ref<T>& other)
	{
		return Ref<D>(other);
	}

}

#include <GraphicsAbstraction/Core/Log.h>
#include <GraphicsAbstraction/Core/Assert.h>