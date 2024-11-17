#include "Identity/LactoseIdentityTestTab.h"

#include "DebugImGuiHelpers.h"
#include "Identity/LactoseIdentityDebugApp.h"
#include "Services/Identity/LactoseIdentityServiceSubsystem.h"

ULactoseIdentityTestTab::ULactoseIdentityTestTab()
{
	SetOwningAppClass<ULactoseIdentityDebugApp>();
	SetTabName(TEXT("Test"));
}

void ULactoseIdentityTestTab::Init()
{
	IdentitySubsystem = UGameInstance::GetSubsystem<ULactoseIdentityServiceSubsystem>(GetWorld()->GetGameInstance());
}

void ULactoseIdentityTestTab::Render()
{
	if (!IsValid(IdentitySubsystem))
	{
		return Debug::ImGui::Error("Identity Subsystem could not be found");
	}

	if (ImGui::CollapsingHeader("Connection Tests"))
	{
		if (ImGui::Button("Ping"))
			IdentitySubsystem->GetServiceInfo();

		if (ImGui::Button("Ping2"))
			IdentitySubsystem->GetServiceInfo2();

		if (ImGui::Button("Ping3"))
			IdentitySubsystem->GetServiceInfo3();
	}
}
