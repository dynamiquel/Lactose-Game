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
				auto* GameInstance = GetGameInstance(WorldContext);
				if (!GameInstance)
					return nullptr;

				return Get<TSubsystem>(*GameInstance);
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

	template<Subsystem TSubsystem, Object TWorldContext = UObject>
	TSubsystem* Get(const TWorldContext* WorldContext)
	{
		if constexpr (EngineSubsystem<TSubsystem>)
		{
			return Engine::Get<TSubsystem>();
		}
		else if (WorldContext)
		{
			return Get<TSubsystem>(*WorldContext);
		}
		else
		{
			UE_LOG(LogSubsystemCollection, Warning, TEXT("A World Context is required for retrieving a non-Engine Subsystem"));
			return nullptr;
		}
	}
	
	template<Subsystem TSubsystem, Object TWorldContext = UObject>
	TSubsystem* Get(const TWorldContext& WorldContext)
	{
		if constexpr (EngineSubsystem<TSubsystem>)
		{
			return Engine::Get<TSubsystem>();
		}
		else if constexpr (GameInstanceSubsystem<TSubsystem>)
		{
			return GameInstance::Get<TSubsystem>(WorldContext);
		}
		else if constexpr (WorldSubsystem<TSubsystem>)
		{
			return World::Get<TSubsystem>(WorldContext);
		}
		else
		{
			return nullptr;
		}
	}

	template<Subsystem TSubsystem, Object TWorldContext = UObject>
	TSubsystem& GetRef(const TWorldContext* WorldContext)
	{
		auto* FoundSubsystem = Get<TSubsystem>(WorldContext);
		check(FoundSubsystem);
		return *FoundSubsystem;
	}

	template<Subsystem TSubsystem, Object TWorldContext = UObject>
	TSubsystem& GetRef(const TWorldContext& WorldContext)
	{
		auto* FoundSubsystem = Get<TSubsystem>(WorldContext);
		check(FoundSubsystem);
		return *FoundSubsystem;
	}
}