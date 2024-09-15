#pragma once

namespace Debug
{
	template<typename T>
	TArray<TSubclassOf<T>> GetClassesOfType(const bool bIncludeAbstract = false, TOptional<TFunction<bool (const T&)>> CDOFilter = {})
	{
		TArray<TSubclassOf<T>> FoundClasses;
		for (TObjectIterator<UClass> It; It; ++It)
		{
			if (/* bPassesAbstractFilter */ (bIncludeAbstract || !It->HasAnyClassFlags(CLASS_Abstract)) &&
				/* bPassesClassFilter */	It->IsChildOf<T>() &&
				/* bPassesCDOFilter */		(!CDOFilter.IsSet() || ::Invoke(*CDOFilter, *It->GetDefaultObject<T>())))
			{
				FoundClasses.Emplace(*It);
			}
		}

		return MoveTemp(FoundClasses);
	}
}
