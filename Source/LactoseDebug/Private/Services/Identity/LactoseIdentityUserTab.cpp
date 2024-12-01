// Fill out your copyright notice in the Description page of Project Settings.


#include "Services/Identity/LactoseIdentityUserTab.h"

#include <imgui.h>

#include "DebugImGuiHelpers.h"
#include "Services/Identity/LactoseIdentityDebugApp.h"
#include "Services/Identity/LactoseIdentityServiceSubsystem.h"

ULactoseIdentityUserTab::ULactoseIdentityUserTab()
{
	SetOwningAppClass<ULactoseIdentityDebugApp>();
	SetTabName(TEXT("User"));
}

void ULactoseIdentityUserTab::Init()
{
	IdentitySubsystem = UGameInstance::GetSubsystem<ULactoseIdentityServiceSubsystem>(GetWorld()->GetGameInstance());
}

void ULactoseIdentityUserTab::Render()
{
	if (!IsValid(IdentitySubsystem))
		return Debug::ImGui::Error("Identity Subsystem could not be found");

	constexpr int32 CurrentUserHeaderFlags = ImGuiTreeNodeFlags_DefaultOpen;
	if (ImGui::CollapsingHeader("Current User"), CurrentUserHeaderFlags)
	{
		TSharedPtr<FLactoseIdentityGetUserResponse> LoggedInUser = IdentitySubsystem->GetLoggedInUserInfo();
		if (!LoggedInUser)
		{
			if (ImGui::Button("Login"))
				IdentitySubsystem->Login();
		}
		else
		{
			if (ImGui::Button("Logout"))
				IdentitySubsystem->Logout();
			
			ImGui::Text("User ID: %s", STR_TO_ANSI(LoggedInUser->Id));
			ImGui::Text("User Display Name: %s", STR_TO_ANSI(LoggedInUser->DisplayName));
			ImGui::Text("User Time Created: %s", STR_TO_ANSI(LoggedInUser->TimeCreated.ToString()));
			ImGui::Text("User Time Last Logged In: %s", STR_TO_ANSI(LoggedInUser->TimeLastLoggedIn.ToString()));
			ImGui::Text("Roles:");
			ImGui::Indent();
			for (const FString& UserRole : LoggedInUser->Roles)
				ImGui::Text("%s", STR_TO_ANSI(UserRole));
			ImGui::Unindent();
		}
	}
}
