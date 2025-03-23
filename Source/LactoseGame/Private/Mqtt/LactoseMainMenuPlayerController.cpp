// Fill out your copyright notice in the Description page of Project Settings.


#include "LactoseMainMenuPlayerController.h"

#include <Blueprint/UserWidget.h>

#include "Simp.h"
#include "LactoseGame/LactoseGame.h"
#include "Services/ConfigCloud/LactoseConfigCloudServiceSubsystem.h"
#include "Services/Economy/LactoseEconomyServiceSubsystem.h"
#include "Services/Identity/LactoseIdentityServiceSubsystem.h"
#include "Services/Simulation/LactoseSimulationServiceSubsystem.h"

ALactoseMainMenuPlayerController::ALactoseMainMenuPlayerController()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> MainMenuWidgetClassFinder(TEXT("/Game/MainMenu/WBP_MainMenu"));
	MainMenuWidgetClass = MainMenuWidgetClassFinder.Class;
}

void ALactoseMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetInputMode(FInputModeGameAndUI());
	SetShowMouseCursor(true);

	if (LIKELY(MainMenuWidgetClass))
	{
		MainMenuWidget = CreateWidget(this, MainMenuWidgetClass);
		MainMenuWidget->AddToPlayerScreen();
	}
	else
		Log::Error(LogLactose, TEXT("Main Menu Widget Class was not set"));
}

void ALactoseMainMenuPlayerController::OpenFarm(const bool bForceStart)
{
	TArray<FString> PendingConditions;
	if (!CanStart(OUT PendingConditions))
	{
		FString PendingConditionsFlattened;
		for (const FString& Condition : PendingConditions)
			PendingConditionsFlattened.Appendf(TEXT("\n%s"), *Condition);

		Log::Log(LogLactose,
			TEXT("Game can not start because preconditions have not been met:%s"),
			*PendingConditionsFlattened);

		if (!bForceStart)
			return;

		Log::Log(LogLactose, TEXT("Force start was requested; bypassing preconditions"));
	}
	
	ConsoleCommand(FString("open FirstPersonMap"));
}

void ALactoseMainMenuPlayerController::TryLogin()
{
	auto& Identity = Subsystems::GetRef<ULactoseIdentityServiceSubsystem>(self);
	Identity.LoginUsingRefreshToken([WeakThis = MakeWeakObjectPtr(this)]
	{
		WeakThis->OnLoginUsingRefreshTokenFailed.Broadcast();
	});
}

void ALactoseMainMenuPlayerController::TryLogout()
{
	auto& Identity = Subsystems::GetRef<ULactoseIdentityServiceSubsystem>(self);
	Identity.Logout();
}

void ALactoseMainMenuPlayerController::LoginUsingBasicAuth(const FString& Email, const FString& Password)
{
	auto& Identity = Subsystems::GetRef<ULactoseIdentityServiceSubsystem>(self);
	Identity.LoginUsingBasicAuth(Email, Password);
}

void ALactoseMainMenuPlayerController::Register(
	const FString& DisplayName,
	const FString& Email,
	const FString& Password)
{
	auto& Identity = Subsystems::GetRef<ULactoseIdentityServiceSubsystem>(self);
	Identity.SignupUsingBasicAuth(DisplayName, Email, Password, [WeakThis = MakeWeakObjectPtr(this)]
	{
		WeakThis->OnSignUpFailed.Broadcast();
	});
}

bool ALactoseMainMenuPlayerController::CanStart(TArray<FString>& PendingConditions) const
{
	// Forked from ALactoseGameMode. Should probably be in its own function somewhere.
	PendingConditions.Reset();
	
	auto& Identity = Subsystems::GetRef<ULactoseIdentityServiceSubsystem>(self);

	switch (Identity.GetLoginStatus())
	{
		case ELactoseIdentityUserLoginStatus::NotLoggedIn:
			PendingConditions.Emplace(TEXT("You are not logged in"));
			break;
		case ELactoseIdentityUserLoginStatus::LoggingIn:
			PendingConditions.Emplace(TEXT("You are being logged in"));
			break;
		case ELactoseIdentityUserLoginStatus::GettingUserInfo:
			PendingConditions.Emplace(TEXT("Your user info is being retrieved"));
		default:
	}

	auto& Economy = Subsystems::GetRef<ULactoseEconomyServiceSubsystem>(self);
	
	switch (Economy.GetAllItemsStatus())
	{
		case ELactoseEconomyAllItemsStatus::None:
			PendingConditions.Emplace(TEXT("Economy items have not been loaded"));
			break;
		case ELactoseEconomyAllItemsStatus::Querying:
			PendingConditions.Emplace(TEXT("Economy items are being queried"));
			break;
		case ELactoseEconomyAllItemsStatus::Retrieving:
			PendingConditions.Emplace(TEXT("Economy items are being loaded"));
			break;
		default:
	}

	switch (Economy.GetCurrentUserItemsStatus())
	{
		case ELactoseEconomyUserItemsStatus::None:
			PendingConditions.Emplace(TEXT("Your items have not been loaded"));
			break;
		case ELactoseEconomyUserItemsStatus::Retrieving:
			PendingConditions.Emplace(TEXT("Your items are being loaded"));
			break;
		default:
	}

	auto& Simulation = Subsystems::GetRef<ULactoseSimulationServiceSubsystem>(self);

	switch (Simulation.GetAllCropsStatus())
	{
		case ELactoseSimulationCropsStatus::None:
			PendingConditions.Emplace(TEXT("Crops have not been loaded"));
			break;
		case ELactoseSimulationCropsStatus::Querying:
			PendingConditions.Emplace(TEXT("Crops are being queried"));
			break;
		case ELactoseSimulationCropsStatus::Retrieving:
			PendingConditions.Emplace(TEXT("Crops are being loaded"));
			break;
		default:
	}

	switch (Simulation.GetCurrentUserCropsStatus())
	{
		case ELactoseSimulationUserCropsStatus::None:
			PendingConditions.Emplace(TEXT("Your crops have not been loaded"));
			break;
		case ELactoseSimulationUserCropsStatus::Retrieving:
			PendingConditions.Emplace(TEXT("Your crops are being loaded"));
			break;
		default:
	}

	auto& ConfigCloud = Subsystems::GetRef<ULactoseConfigCloudServiceSubsystem>(self);

	switch (ConfigCloud.GetStatus())
	{
		case ELactoseConfigCloudStatus::None:
			PendingConditions.Emplace(TEXT("Config has not been loaded"));
			break;
		case ELactoseConfigCloudStatus::Retrieving:
			PendingConditions.Emplace(TEXT("Config is being loaded"));
			break;
		default:
	}

	return PendingConditions.IsEmpty();
}
