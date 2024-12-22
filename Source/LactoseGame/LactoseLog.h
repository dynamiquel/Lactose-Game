#pragma once

namespace Log
{
	template<typename T>
	concept LogCategory = std::is_base_of_v<FLogCategoryBase, T>;

	template<LogCategory TCategory, ELogVerbosity::Type EVerbosity>
	constexpr bool IsLogCompiledOut()
	{
		return (EVerbosity & ELogVerbosity::VerbosityMask) <= TCategory::CompileTimeVerbosity
			&& (EVerbosity & ELogVerbosity::VerbosityMask) <= ELogVerbosity::COMPILED_IN_MINIMUM_VERBOSITY;
	}
	
	template<LogCategory TCategory, ELogVerbosity::Type EVerbosity, typename... TArgs>
	static void Log(const TCategory& Category, const TCHAR* Format, TArgs&&... Args)
	{
		static UE::Logging::Private::FStaticBasicLogDynamicData LOG_Dynamic;
		static UE::Logging::Private::FStaticBasicLogRecord LOG_Static(
			Format, nullptr, 0, EVerbosity, LOG_Dynamic);

		if constexpr ((EVerbosity & ELogVerbosity::VerbosityMask) == ELogVerbosity::Fatal)
		{
			UE::Logging::Private::BasicFatalLog(Category, &LOG_Static, std::forward<TArgs>(Args)...);
			return;
		}
	
		if constexpr (IsLogCompiledOut<TCategory, EVerbosity>())
			return;

		if (Category.IsSuppressed(EVerbosity))
			return;
	
		UE::Logging::Private::BasicLog(Category, &LOG_Static, std::forward<TArgs>(Args)...);
	
		UE_LOG(LogTemp, Log, TEXT("%s"), TEXT("Hi"));
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
		Log(LogTemp, TEXT("%s"), TEXT("Hello, World!"));
		Warning(LogTemp, TEXT("Hello, World!"));
	}
}