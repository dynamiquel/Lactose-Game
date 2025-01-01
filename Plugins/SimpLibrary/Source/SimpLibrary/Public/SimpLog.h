#pragma once

/**
 * Replacement for Unreal's Logging Macros by relying on C++ templates
 * and compiler optimisations instead.
 * 
 * It's designed to be an easier to read and more standardised API.
 */
namespace Log
{
	/**
	 * Alternative approach to creating new Log Categories.
	 * It only needs to be defined in the header.
	 *
	 * Example usage:
	 * @code static inline TCategory<> MyNewLogCategory("LogMyNewCategory");@endcode.
	 * @note This approach does create multiple instances of the specified Log Category
	 * but you probably don't need to worry about it.
	 */
	template<ELogVerbosity::Type DefaultVerbosity = ELogVerbosity::Type::Log, ELogVerbosity::Type CompiledVerbosity = ELogVerbosity::Type::All>
	using TCategory = FLogCategory<DefaultVerbosity, CompiledVerbosity>;
	
	template<typename T>
	concept LogCategory = std::is_base_of_v<FLogCategoryBase, T>;

	template<LogCategory TCategory, ELogVerbosity::Type EVerbosity>
	constexpr bool IsCompiledOut()
	{
		return (EVerbosity & ELogVerbosity::VerbosityMask) > TCategory::CompileTimeVerbosity
			|| (EVerbosity & ELogVerbosity::VerbosityMask) > ELogVerbosity::COMPILED_IN_MINIMUM_VERBOSITY;
	}

	template<ELogVerbosity::Type EVerbosity, LogCategory TCategory>
	bool IsActive(const TCategory& Category)
	{
		if constexpr (IsCompiledOut<TCategory, EVerbosity>())
			return false;

		return !Category.IsSuppressed(EVerbosity);
	}
	
	template<LogCategory TCategory, ELogVerbosity::Type EVerbosity, typename... TArgs>
	static void Log(const TCategory& Category, const TCHAR* Format, TArgs&&... Args)
	{
		static_assert((TIsValidVariadicFunctionArg<TArgs>::Value && ...), "Invalid argument(s) passed to Log");

#if NO_LOGGING
	return;
#else
		static UE::Logging::Private::FStaticBasicLogDynamicData LOG_Dynamic;
		static UE::Logging::Private::FStaticBasicLogRecord LOG_Static(
			Format, __builtin_FILE(), __builtin_LINE(), EVerbosity, LOG_Dynamic);
		
		if constexpr ((EVerbosity & ELogVerbosity::VerbosityMask) == ELogVerbosity::Fatal)
		{
			UE::Logging::Private::BasicFatalLog(Category, &LOG_Static, std::forward<TArgs>(Args)...);
			return;
		}
		
		if (!IsActive<EVerbosity>(Category))
			return;
		
		UE::Logging::Private::BasicLog(Category, &LOG_Static, std::forward<TArgs>(Args)...);
#endif
	}

	template<LogCategory TCategory, typename... TArgs>
	static void Error(const TCategory& Category, const TCHAR* Format, TArgs&&... Args)
	{
		Log<TCategory, ELogVerbosity::Type::Error>(Category, Format, std::forward<TArgs>(Args)...);
	}

	template<LogCategory TCategory, typename... TArgs>
	static void Warning(const TCategory& Category, const TCHAR* Format, TArgs&&... Args)
	{
		Log<TCategory, ELogVerbosity::Type::Warning>(Category, Format, std::forward<TArgs>(Args)...);
	}

	template<LogCategory TCategory, typename... TArgs>
	static void Log(const TCategory& Category, const TCHAR* Format, TArgs&&... Args)
	{
		Log<TCategory, ELogVerbosity::Type::Log>(Category, Format, std::forward<TArgs>(Args)...);
	}

	template<LogCategory TCategory, typename... TArgs>
	static void Verbose(const TCategory& Category, const TCHAR* Format, TArgs&&... Args)
	{
		Log<TCategory, ELogVerbosity::Type::Verbose>(Category, Format, std::forward<TArgs>(Args)...);
	}

	template<LogCategory TCategory, typename... TArgs>
	static void VeryVerbose(const TCategory& Category, const TCHAR* Format, TArgs&&... Args)
	{
		Log<TCategory, ELogVerbosity::Type::VeryVerbose>(Category, Format, std::forward<TArgs>(Args)...);
	}

	inline void Hello()
	{
		Log(LogTemp, TEXT("Hello, World!\nThe time is: %s"), *FDateTime::Now().ToString());
	}
}

/**
 * Shorter way of declaring a Log Category.
 * 
 * Equivalent to @code DECLARE_LOG_CATEGORY_EXTERN(CategoryName, Log, All)@endcode.
 */
#define DECLARE_BASIC_LOG(CategoryName) DECLARE_LOG_CATEGORY_EXTERN(CategoryName, Log, All)

/**
 * Quickest way to create a simple log. It only needs to be defined in the header.
 * 
 * @note This approach does create multiple instances of the specified Log Category
 * but you probably don't need to worry about it.
 */
#define CREATE_BASIC_LOG(CategoryName) static inline Log::TCategory<> CategoryName(TEXT(#CategoryName));