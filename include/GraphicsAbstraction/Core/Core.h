#pragma once

#include <cstdint>
#include <utility>
#include <memory>
#include <type_traits>

#ifndef GA_DLL_LINK
	#ifdef GA_BUILD_DLL
		#define GA_DLL_LINK __declspec(dllexport)
	#else
		#define GA_DLL_LINK __declspec(dllimport)
	#endif
#endif

#define GA_RHI_TEMPLATE(name, ...)						\
	Impl<name>* impl;									\
	virtual ~name();									\
	static Ref<name> Create(__VA_ARGS__);

namespace GraphicsAbstraction {

	template<typename T>
	struct Impl;

	template<typename T> class Ref;
	class GA_DLL_LINK RefCounted
	{
	protected:
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

		template<typename D>
		Ref(const Ref<D>& other, T* pointer)
			: m_Value(pointer)
		{
			Increment();
		}

		template<typename D>
		Ref(const Ref<D>& other, const T* pointer)
			: m_Value((T*)pointer) // TODO: basically a const_cast, very bad
		{
			Increment();
		}

		Ref() : m_Value(nullptr) { }
		Ref(std::nullptr_t) : m_Value(nullptr) { }
		Ref(const Ref& other) : m_Value(other.m_Value) { Increment(); }
		Ref(Ref&& other) : m_Value(std::exchange(other.m_Value, nullptr)) { }
		~Ref() { Decrement(); }

		inline T* Get() { return m_Value; }
		inline const T* Get() const { return m_Value; }

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

	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return Ref<T>(new T(std::forward<Args>(args)...));
	}

	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

}