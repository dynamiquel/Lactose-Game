// Fill out your copyright notice in the Description page of Project Settings.


#include "Services/Config/LactoseConfigTab.h"

#include "DebugImGuiHelpers.h"
#include "Services/Config/LactoseConfigDebugApp.h"
#include "Services/ConfigCloud/LactoseConfigCloudServiceSubsystem.h"

ULactoseConfigTab::ULactoseConfigTab()
{
	SetOwningAppClass<ULactoseConfigDebugApp>();
	SetTabName(TEXT("Config"));
}

void ULactoseConfigTab::Init()
{
	ConfigSubsystem = UGameInstance::GetSubsystem<ULactoseConfigCloudServiceSubsystem>(GetWorld()->GetGameInstance());
}

void ULactoseConfigTab::Render()
{
	if (!IsValid(ConfigSubsystem))
		return Debug::ImGui::Error("Config Cloud Subsystem could not be found");
	
	ImGui::Text("Status: ");
	ImGui::SameLine();
	
	switch (ConfigSubsystem->GetStatus())
	{
		case ELactoseConfigCloudStatus::None:
			ImGui::Text("None");
			if (ImGui::Button("Load Config"))
				ConfigSubsystem->LoadConfig();
			break;
		case ELactoseConfigCloudStatus::Retrieving:
			ImGui::Text("Retrieving");
			break;
		case ELactoseConfigCloudStatus::Loaded:
			ImGui::Text("Loaded");
			if (ImGui::Button("Load Config"))
				ConfigSubsystem->LoadConfig();
			break;
		default:
			ImGui::Text("Unknown");
	}

	ImGui::Spacing();

	TSharedPtr<const FLactoseConfigCloudConfig> Config = ConfigSubsystem->GetConfig();
	if (!Config)
		return;

	ConfigSearchBox.Draw();

	ImGui::BeginChild("ConfigScrollingArea", ImVec2(0, 0), /* bBorder */ true);
	
	ImGui::Spacing();

	for (const TTuple<FString, TSharedRef<FLactoseConfigCloudEntry>>& Entry : Config->GetEntries())
	{
		if (!ConfigSearchBox.PassesFilter(Entry.Key))
			continue;

		if (!ImGui::CollapsingHeader(STR_TO_ANSI(Entry.Key)))
			continue;

		ImGui::Indent();
		ImGui::TextWrapped("%s", STR_TO_ANSI(Entry.Value->GetString()));
		if (auto* ScriptStruct = Entry.Value->GetCachedScriptStruct())
			ImGui::Text("Cached Script Struct: %s", STR_TO_ANSI(ScriptStruct->GetName()));
		ImGui::Unindent();
	}

	ImGui::EndChild();

}
