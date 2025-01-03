#pragma once

#include <Misc/Optional.h>
#include "SimpShorts.generated.h"

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
template<typename T, ESPMode InMode = ESPMode::ThreadSafe>
[[nodiscard]] TSharedRef<T, InMode> MakeShared(T&& TempType)
{
	return TSharedRef<T, InMode>(new T(Forward<T>(TempType)));
}

template<typename T, ESPMode InMode = ESPMode::ThreadSafe>
[[nodiscard]] TSharedRef<T, InMode> MakeShared(const T& SourceType)
{
	return TSharedRef<T, InMode>(new T(SourceType));
}

template<typename T>
[[nodiscard]] TUniquePtr<T> MakeUnique(T&& TempType)
{
	return TUniquePtr<T>(new T(Forward<T>(TempType)));
}

template<typename T>
[[nodiscard]] TUniquePtr<T> MakeUnique(const T& SourceType)
{
	return TUniquePtr<T>(new T(SourceType));
}

template<typename TGameInstance = UGameInstance> requires std::is_base_of_v<UGameInstance, TGameInstance>
TGameInstance* GetGameInstance(const UObject& WorldContext)
{
	const UWorld* World = WorldContext.GetWorld();
	if (!World)
		return nullptr;

	UGameInstance* GameInstance = World->GetGameInstance();
	return Cast<TGameInstance>(GameInstance);
}

template<typename TGameInstance = UGameInstance> requires std::is_base_of_v<UGameInstance, TGameInstance>
TGameInstance& GetGameInstanceRef(const UObject& WorldContext)
{
	auto* FoundGameInstance = GetGameInstance<TGameInstance>(WorldContext);
	check(FoundGameInstance);
	return *FoundGameInstance;
}

template<typename TPlayerController = APlayerController, typename TPlayerContext = UObject> requires std::is_base_of_v<APlayerController, TPlayerController>
TPlayerController* GetPlayerController(const TPlayerContext& PlayerContext)
{
	if constexpr (std::is_base_of_v<APlayerState, TPlayerContext>)
	{
		APlayerController* PlayerController = PlayerContext.GetPlayerController();
		return Cast<TPlayerController>(PlayerController);
	}
	else if constexpr (std::is_base_of_v<APawn, TPlayerContext>)
	{
		AController* Controller = PlayerContext.GetController();
		return Cast<TPlayerController>(Controller);
	}
	else if constexpr (std::is_base_of_v<UActorComponent, TPlayerContext>)
	{
		const UObject* Owner = PlayerContext.GetOwner();
		if (!Owner)
			return nullptr;

		return GetPlayerController<TPlayerController>(*Owner);
	}
	else if constexpr (std::is_base_of_v<AActor, TPlayerContext>)
	{
		if (auto* OwnerAsPC = Cast<APlayerController>(&PlayerContext))
			return const_cast<APlayerController*>(OwnerAsPC);
		
		if (auto* OwnerAsPawn = Cast<APawn>(&PlayerContext))
			return GetPlayerController<TPlayerController>(*OwnerAsPawn);
		
		return nullptr;
	}
	else
	{
		return nullptr;
	}
}

#define self *this

/**
 * This struct simply exists so UHT can make a generated.h file for this file.
 * This is required so the dynamic delegates below will compile.
 */
USTRUCT()
struct FSimpGeneratedFileStub
{
	GENERATED_BODY()
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSimpDynamicMCDelegate);
DECLARE_DYNAMIC_DELEGATE(FSimpDynamicDelegate);