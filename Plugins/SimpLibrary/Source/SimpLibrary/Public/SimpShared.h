#pragma once

#include <Templates/SharedPointer.h>

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


template<typename T>
Sr<T> CreateSr()
{
	return Sr<T>();
}

template<typename T>
Sr<T> CreateSr(T&& TempType)
{
	return MakeShared(Forward<T>(TempType));
}

template<typename T, typename... TArgs>
Sr<T> CreateSr(TArgs&&... Args)
{
	return MakeShared<T>(Forward<TArgs>(Args)...);
}
