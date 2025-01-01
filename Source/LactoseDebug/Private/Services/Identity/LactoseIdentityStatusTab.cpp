#include "Services/Identity/LactoseIdentityStatusTab.h"

#include "DebugImGuiHelpers.h"
#include "Services/LactoseServiceDebugUtils.h"
#include "Services/Identity/LactoseIdentityDebugApp.h"
#include "Services/Identity/LactoseIdentityServiceSubsystem.h"

ULactoseIdentityStatusTab::ULactoseIdentityStatusTab()
{
	SetOwningAppClass<ULactoseIdentityDebugApp>();
	SetTabName(TEXT("Status"));
}

void ULactoseIdentityStatusTab::Init()
{
	IdentitySubsystem = Subsystems::Get<ULactoseIdentityServiceSubsystem>(self);
	StatusSection = CreateSr<Lactose::Debug::Services::FStatusSection>(IdentitySubsystem);
}

void ULactoseIdentityStatusTab::Render()
{
	if (!IsValid(IdentitySubsystem))
		return Debug::ImGui::Error("Identity Subsystem could not be found");
	
	if (StatusSection)
		StatusSection->Render();
}
