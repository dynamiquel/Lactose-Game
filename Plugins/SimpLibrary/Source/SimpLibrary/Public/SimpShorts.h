#pragma once

#include <Misc/Optional.h>

/**
 * Optional:
 * Optional object.
 */
template<typename T>
using Opt = TOptional<T>;

/**
 * Creates an Unreal Shared Reference using a standard struct initialiser.
 * This allows for usages such as:
 * 
 * @code auto MySharedStruct = MakeShared(MyStruct{.Var1 = 1, .Var2 = 4});@endcode.
 */
template <typename T, ESPMode InMode = ESPMode::ThreadSafe>
[[nodiscard]] TSharedRef<T, InMode> MakeShared(T&& TempType)
{
	return TSharedRef<T, InMode>(new T(Forward<T>(TempType)));
}

template <typename T, ESPMode InMode = ESPMode::ThreadSafe>
[[nodiscard]] TSharedRef<T, InMode> MakeShared(const T& SourceType)
{
	return TSharedRef<T, InMode>(new T(SourceType));
}

template <typename T>
[[nodiscard]] TUniquePtr<T> MakeUnique(T&& TempType)
{
	return TUniquePtr<T>(new T(Forward<T>(TempType)));
}

template <typename T>
[[nodiscard]] TUniquePtr<T> MakeUnique(const T& SourceType)
{
	return TUniquePtr<T>(new T(SourceType));
}

