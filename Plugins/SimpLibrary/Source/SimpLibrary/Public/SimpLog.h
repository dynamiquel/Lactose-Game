#pragma once

/**
 * Reimplementation of Unreal's Logging Macros using C++ templates instead.
 * It's designed to be an easier to read and more standardised API.
 */
namespace Log
{
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
		Log(LogTemp, TEXT("Hello"));
	}
}