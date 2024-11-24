#include "Services/Identity/LactoseIdentityDiscordDebugTab.h"

#include <imgui.h>

#include "DebugImGuiHelpers.h"
#include "DiscordGame/DiscordGameSubsystem.h"
#include "DiscordGame/DiscordHelpers.h"
#include "Services/Identity/LactoseIdentityDebugApp.h"

ULactoseIdentityDiscordDebugTab::ULactoseIdentityDiscordDebugTab()
{
	SetOwningAppClass<ULactoseIdentityDebugApp>();
	SetTabName(TEXT("Discord"));
}

void ULactoseIdentityDiscordDebugTab::Init()
{
	DiscordSubsystem = GEngine->GetEngineSubsystem<UDiscordGameSubsystem>();
}

void ULactoseIdentityDiscordDebugTab::Render()
{
	if (!IsValid(DiscordSubsystem))
	{
		return Debug::ImGui::Error("Discord Subsystem could not be found");
	}

	if (!DiscordSubsystem->IsDiscordSDKLoaded())
	{
		return Debug::ImGui::Error("Discord Game SDK is not loaded");
	}

	if (!DiscordSubsystem->IsDiscordRunning())
	{
		return Debug::ImGui::Error("Discord Game SDK is not running");
	}

	discord::User DiscordUser;
	if (discord::Result Result = DiscordSubsystem->GetUserManager().GetCurrentUser(&DiscordUser); Result != discord::Result::Ok)
	{
		return Debug::ImGui::Error("Could not get the current Discord User due to Result %d", Result);
	}

	ImGui::Text("Discord User ID: %ld", DiscordUser.GetId());
	ImGui::Text("Discord Username: %s", DiscordUser.GetUsername());
	ImGui::Text("Discord Discriminator: %s", DiscordUser.GetDiscriminator());
}
