#include "Services/Economy/LactoseEconomyStatusTab.h"

#include "DebugImGuiHelpers.h"
#include "Services/LactoseServiceDebugUtils.h"
#include "Services/Economy/LactoseEconomyDebugApp.h"
#include "Services/Economy/LactoseEconomyServiceSubsystem.h"

ULactoseEconomyStatusTab::ULactoseEconomyStatusTab()
{
	SetOwningAppClass<ULactoseEconomyDebugApp>();
	SetTabName(TEXT("Status"));
}

void ULactoseEconomyStatusTab::Init()
{
	EconomySubsystem = UGameInstance::GetSubsystem<ULactoseEconomyServiceSubsystem>(GetWorld()->GetGameInstance());
	StatusSection = CreateSr<Lactose::Debug::Services::FStatusSection>(EconomySubsystem);
}

void ULactoseEconomyStatusTab::Render()
{
	if (!IsValid(EconomySubsystem))
		return Debug::ImGui::Error("Economy Subsystem could not be found");
	
	if (StatusSection)
		StatusSection->Render();
}
