#pragma once

#include <Templates/UniquePtr.h>

/**
 * Pointer:
 * Non-shareable and optional object.
 */
template<typename T>
using Ptr = TUniquePtr<T>;

template<typename T>
Ptr<T> CreatePtr()
{
	return Ptr<T>();
}

template<typename T>
Ptr<T> CreatePtr(T&& TempType)
{
	return MakeUnique<T>(Forward<T>(TempType));
}

template<typename T, typename... TArgs>
Ptr<T> CreatePtr(TArgs&&... Args)
{
	return MakeUnique<T>(Forward<TArgs>(Args)...);
}
