#pragma once

#include "SimpConcepts.h"
#include "SimpShorts.h"

using namespace Simp;

namespace Subsystems
{
	namespace Engine
	{
		template<EngineSubsystem TSubsystem>
		TSubsystem* Get()
		{
			return GEngine->GetEngineSubsystem<TSubsystem>();
		}

		template<EngineSubsystem TSubsystem>
		TSubsystem& GetRef()
		{
			auto EngineSubsystem = Get<TSubsystem>();
			check(EngineSubsystem);
			return *EngineSubsystem;
		}
	}

	namespace GameInstance
	{
		template<GameInstanceSubsystem TSubsystem, Object TWorldContext = UObject>
		TSubsystem* Get(const TWorldContext& WorldContext)
		{
			if constexpr (std::is_base_of_v<UGameInstance, TWorldContext>)
			{
				return static_cast<const UGameInstance&>(WorldContext).GetSubsystem<TSubsystem>();
			}
			else
			{
				UGameInstance* FoundGameInstance;
				if constexpr (GameInstanceSubsystem<TWorldContext>)
				{
					FoundGameInstance = static_cast<const UGameInstanceSubsystem&>(WorldContext).GetGameInstance();
				}
				else
				{
					FoundGameInstance = GetGameInstance(WorldContext);
				}

				if (!FoundGameInstance)
					return nullptr;

				return Get<TSubsystem>(*FoundGameInstance);
			}
		}

		template<GameInstanceSubsystem TSubsystem, Object TWorldContext = UObject>
		TSubsystem& GetRef(const TWorldContext& WorldContext)
		{
			auto* FoundSubsystem = Get<TSubsystem>(WorldContext);
			check(FoundSubsystem);
			return *FoundSubsystem;
		}
	}

	namespace World
	{
		template<WorldSubsystem TSubsystem, Object TWorldContext = UObject>
		TSubsystem* Get(const TWorldContext& WorldContext)
		{
			if constexpr (std::is_base_of_v<UWorld, TWorldContext>)
			{
				return static_cast<const UWorld&>(WorldContext).GetSubsystem<TSubsystem>();
			}
			else
			{
				auto* World = WorldContext.GetWorld();
				if (!World)
					return nullptr;

				return Get<TSubsystem>(*World);
			}
		}

		template<WorldSubsystem TSubsystem, Object TWorldContext = UObject>
		TSubsystem& GetRef(const TWorldContext& WorldContext)
		{
			auto* FoundSubsystem = Get<TSubsystem>(WorldContext);
			check(FoundSubsystem);
			return *FoundSubsystem;
		}
	}

	namespace LocalPlayer
	{
		template<LocalPlayerSubsystem TSubsystem, Object TPlayerContext = UObject>
		TSubsystem* Get(const TPlayerContext& PlayerContext)
		{
			if constexpr (std::is_base_of_v<ULocalPlayer, TPlayerContext>)
			{
				return static_cast<const ULocalPlayer&>(PlayerContext).GetSubsystem<TSubsystem>();
			}
			else if constexpr (std::is_base_of_v<APlayerController, TPlayerContext>)
			{
				ULocalPlayer* FoundLocalPlayer = static_cast<const APlayerController&>(PlayerContext).GetLocalPlayer();
				if (!FoundLocalPlayer)
					return nullptr;

				return Get<TSubsystem>(*FoundLocalPlayer);
			}
			else
			{
				auto* FoundPlayerController = GetPlayerController(PlayerContext);
				if (!FoundPlayerController)
					return nullptr;

				return Get<TSubsystem>(*FoundPlayerController);
			}
		}

		template<WorldSubsystem TSubsystem, Object TPlayerContext = UObject>
		TSubsystem& GetRef(const TPlayerContext& PlayerContext)
		{
			auto* FoundSubsystem = Get<TSubsystem>(PlayerContext);
			check(FoundSubsystem);
			return *FoundSubsystem;
		}
	}

	/**
	 * Easy way to find a Subsystem by simply providing a Subsystem Type and Context.
	 *
	 * It will determine at compile-time where to look for the Subsystem based
	 * on the provided Subsystem Type and Context.
	 */
	template<Subsystem TSubsystem, Object TContext = UObject>
	TSubsystem* Get(const TContext& Context)
	{
		if constexpr (EngineSubsystem<TSubsystem>)
		{
			return Engine::Get<TSubsystem>();
		}
		else if constexpr (GameInstanceSubsystem<TSubsystem>)
		{
			return GameInstance::Get<TSubsystem>(Context);
		}
		else if constexpr (WorldSubsystem<TSubsystem>)
		{
			return World::Get<TSubsystem>(Context);
		}
		else if constexpr (LocalPlayerSubsystem<TSubsystem>)
		{
			return LocalPlayer::Get<TSubsystem>(Context);
		}
		else
		{
			return nullptr;
		}
	}

	template<Subsystem TSubsystem, Object TContext = UObject>
	TSubsystem* Get(const TContext* Context)
	{
		if constexpr (EngineSubsystem<TSubsystem>)
		{
			// Engine Subsystems don't require any kind of Context.
			return Engine::Get<TSubsystem>();
		}
		else if (Context)
		{
			return Get<TSubsystem>(*Context);
		}
		else
		{
			UE_LOG(LogSubsystemCollection, Warning, TEXT("A Context is required for retrieving a non-Engine Subsystem"));
			return nullptr;
		}
	}

	template<Subsystem TSubsystem, Object TContext = UObject>
	TSubsystem& GetRef(const TContext* Context)
	{
		auto* FoundSubsystem = Get<TSubsystem>(Context);
		check(FoundSubsystem);
		return *FoundSubsystem;
	}

	template<Subsystem TSubsystem, Object TContext = UObject>
	TSubsystem& GetRef(const TContext& World)
	{
		auto* FoundSubsystem = Get<TSubsystem>(World);
		check(FoundSubsystem);
		return *FoundSubsystem;
	}
}