#pragma once

namespace Simp
{
	template<typename T>
	concept Object = std::is_base_of_v<T, UObject>;

	template<typename T>
	concept Actor = std::is_base_of_v<T, AActor>;

	template<typename T>
	concept Component = std::is_base_of_v<T, UActorComponent>;

	template<typename T>
	concept Subsystem = std::is_base_of_v<USubsystem, T>;

	template<typename T>
	concept EngineSubsystem = std::is_base_of_v<UEngineSubsystem, T>;

	template<typename T>
	concept GameInstanceSubsystem = std::is_base_of_v<UGameInstanceSubsystem, T>;

	template<typename T>
	concept WorldSubsystem = std::is_base_of_v<UWorldSubsystem, T>;

	template<typename T>
	concept LocalPlayerSubsystem = std::is_base_of_v<ULocalPlayerSubsystem, T>;
}