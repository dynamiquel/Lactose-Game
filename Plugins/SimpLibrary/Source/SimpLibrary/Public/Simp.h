#pragma once

/*
 * Simp is a single-header library that is used to help reduce the verbosity for common
 * C++ operations.
 *
 * Maybe you like it or maybe you don't. Anyways, I do. - dynamiquel
 */

/**
 * Optional:
 * Optional object.
 */
template<typename T>
using Opt = TOptional<T>;

/**
 * Pointer:
 * Non-shareable and optional object.
 */
template<typename T>
using Ptr = TUniquePtr<T>;

/**
 * Shared Reference:
 * Shareable ref-counted object.
 */
template<typename T>
using Sr = TSharedRef<T>;

/**
 * Shared Pointer:
 * Shareable ref-counted optional object.
 */ 
template<typename T>
using Sp = TSharedPtr<T>;

/**
 * Weak Pointer:
 * Weak reference to a shareable object.
 */
template<typename T>
using Wp = TWeakPtr<T>;

namespace Simp
{
	enum class ERefCountMode : uint8
	{
		NotAtomic,
		Atomic
	};
	
	/**
	 * Simple ref-counted smart reference because Unreal doesn't seem to have one.
	 *
	 * - Cheaper than Shared Pointers and safer than raw pointers.
	 * - Never null.
	 * - References are released upon destruction.
	 * - Supports atomic operations for thread-safety or not for performance.
	 * - Non-intrusive (works on anything!)
	 */
	template<typename T, ERefCountMode Mode>
	class TRefCounted
	{
		class Rc_Internal
		{
			using RefCountType = std::conditional_t<Mode == ERefCountMode::Atomic, std::atomic<uint32>, uint32>;

			explicit Rc_Internal(Ptr<T> InPtr)
				: Ptr(MoveTemp(InPtr))
				, RefCount(1)
			{}

			int32 IncrementRefCount()
			{
				if constexpr (Mode == ERefCountMode::Atomic)
					return RefCount.fetch_add(1);
				else
					return RefCount += 1;
			}

			int32 DecrementRefCount()
			{
				if constexpr (Mode == ERefCountMode::Atomic)
					return RefCount.fetch_sub(1);
				else
					return RefCount -= 1;
			}

			int32 GetRefCount() const
			{
				if constexpr (Mode == ERefCountMode::Atomic)
					return RefCount.load(std::memory_order_relaxed);
				else
					return RefCount;
			}

			Ptr<T> Ptr;
			RefCountType RefCount;
		};

	public:
		explicit TRefCounted(Ptr<T> InPtr)
			: Reference(Rc_Internal(MoveTemp(InPtr)))
		{
			CheckInitialisedCorrectly();
		}

		TRefCounted(const TRefCounted& InRc)
		{
			Reference = InRc.Reference;
			CheckInitialisedCorrectly();

			Reference->IncrementRefCount();
		}

		explicit TRefCounted(TRefCounted&& MovedRc)
		{
			Reference = MovedRc.Reference;
			MovedRc.Reference = nullptr;
			// No need to update ref count.
			CheckInitialisedCorrectly();
		}
		
		~TRefCounted()
		{
			if (Reference && Reference->DecrementRefCount() == 0)
				delete Reference;
		}

		int32 GetRefCount() const
		{
			return Reference ? Reference->GetRefCount() : 0;
		}

		T& Get()
		{
			check(Reference);
			check(Reference->InternalPtr);
			return *Reference->InternalPtr;
		}

		const T& Get() const
		{
			return const_cast<TRefCounted*>(this)->Get();
		}

		T* operator->() const
		{
			return &Get();
		}

	private:
		void CheckInitialisedCorrectly() const
		{
			checkf(Reference->Ptr.IsValid(), TEXT("Rc must always be initialised to a valid object"));
			check(Reference->GetRefCount() > 0);
		}

	private:
		Rc_Internal* Reference;
	};
}

/**
 * Simple ref-counted object.
 *
 * - Cheaper than Shared Pointers and safer than raw pointers.
 * - Never null.
 * - References are released upon destruction.
 */
template<typename T>
using Rc = Simp::TRefCounted<T, Simp::ERefCountMode::NotAtomic>;

/**
 * Simple atomic ref-counted object.
 *
 * - Cheaper than Shared Pointers and safer than raw pointers.
 * - Never null.
 * - References are released upon destruction.
 */
template<typename T>
using Arc = Simp::TRefCounted<T, Simp::ERefCountMode::Atomic>;