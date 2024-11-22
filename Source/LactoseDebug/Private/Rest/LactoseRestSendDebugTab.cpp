// Fill out your copyright notice in the Description page of Project Settings.


#include "Rest/LactoseRestSendDebugTab.h"

#include <imgui.h>
#include <imgui_stdlib.h>

#include "DebugImGuiHelpers.h"
#include "Rest/LactoseRestDebugApp.h"
#include "Rest/LactoseRestRequest.h"
#include "Rest/LactoseRestSubsystem.h"

ULactoseRestSendDebugTab::ULactoseRestSendDebugTab()
{
	SetOwningAppClass<ULactoseRestDebugApp>();
	SetTabName(TEXT("Send"));
}

void ULactoseRestSendDebugTab::Render()
{
	constexpr int32 Columns = 3;
	constexpr int32 Flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersV | ImGuiTableFlags_SizingStretchProp;
	if (!ImGui::BeginTable("###RestClientTable", Columns, Flags))
		return;

	constexpr int32 ColumnFlags = 0;
	//constexpr float RequestsColumnWidth = 100.0f;
	ImGui::TableSetupColumn("Requests", ColumnFlags, 1);
	ImGui::TableSetupColumn("Selected Request", ColumnFlags, 2);
	ImGui::TableSetupColumn("Response", ColumnFlags, 2);

	ImGui::TableNextColumn();
	DrawRequestsSection();
	ImGui::TableNextColumn();
	DrawSelectedRequestSection();
	ImGui::TableNextColumn();
	DrawResponseSection();

	ImGui::EndTable();
}

void ULactoseRestSendDebugTab::DrawRequestsSection()
{
	// TODO:
	// Content horizontal and vertical scrolling.
	// Let requests be in groups (collections).
	
	ImGui::Text("Requests");

	if (ImGui::Button("Create"))
		CreateNewRequest();

	ImGui::SameLine();

	if (!SavedRequests.IsEmpty())
		if (ImGui::Button("Clear"))
			ClearRequests();
	
	ImGui::Spacing();

	constexpr int32 Columns = 3;
	constexpr int32 Flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;
	if (!ImGui::BeginTable("Requests Table", Columns, Flags))
		return;

	ImGui::TableSetupColumn("###RequestNameColumn");

	constexpr int32 ButtonColumnFlags = ImGuiTableColumnFlags_WidthFixed;
	ImGui::TableSetupColumn("###SendRequestColumn", ButtonColumnFlags);
	ImGui::TableSetupColumn("###DeleteRequestColumn", ButtonColumnFlags);

	for (int32 RequestIdx = 0; RequestIdx < SavedRequests.Num(); RequestIdx++)
	{
		const FString UniqueRequestId = TEXT("###") + LexToString(RequestIdx);
		ImGui::TableNextColumn();
		
		const bool bSelected = SelectedRequestIndex == RequestIdx;
		const FString& RequestName = SavedRequests[RequestIdx].Name;
		const FString Label = RequestName + UniqueRequestId;

		if (ImGui::Selectable(STR_TO_ANSI(Label), bSelected, ImGuiSelectableFlags_SpanAllColumns))
			SelectedRequestIndex = RequestIdx;

		ImGui::TableNextColumn();

		const FString SendButtonLabel = TEXT("->") + UniqueRequestId;
		if (ImGui::Button(STR_TO_ANSI(SendButtonLabel)))
			SendRequest(RequestIdx);

		ImGui::TableNextColumn();

		const FString DeleteButtonLabel = TEXT("X") + UniqueRequestId;
		if (ImGui::Button(STR_TO_ANSI(DeleteButtonLabel)))
			DeleteRequest(RequestIdx);
	}

	ImGui::EndTable();
}

void ULactoseRestSendDebugTab::DrawSelectedRequestSection()
{
	if (!SavedRequests.IsValidIndex(SelectedRequestIndex))
		return;
	
	FLactoseRestDebugRequest& SelectedRequest = SavedRequests[SelectedRequestIndex];

	std::string Name = STR_TO_ANSI(SelectedRequest.Name);
	std::string RequestUrl = STR_TO_ANSI(SelectedRequest.Url);
	std::string Content = STR_TO_ANSI(SelectedRequest.Content);

	ImGui::Text("Selected Request");

	if (bSelectedRequestDirty)
		if (ImGui::Button("Save###SaveRequest"))
			SaveRequests();

	ImGui::SameLine();
	
	if (ImGui::Button("Delete###DeleteRequest"))
		DeleteRequest(SelectedRequestIndex);
	
	ImGui::Spacing();

	ImGui::Text("Name:");
	ImGui::SameLine();
	if (ImGui::InputText("###Name", &Name))
	{
		SelectedRequest.Name = ANSI_TO_TCHAR(Name.c_str());
		bSelectedRequestDirty = true;
	}

	ImGui::Text("Verb:");
	ImGui::SameLine();
	if (ImGui::Combo("###Verb", &SelectedRequest.Verb, Verbs.data(), Verbs.size()))
	{
		bSelectedRequestDirty = true;
	}

	ImGui::Text("URL:");
	ImGui::SameLine();
	if (ImGui::InputText("###URL", &RequestUrl))
	{
		SelectedRequest.Url = ANSI_TO_TCHAR(RequestUrl.c_str());
		bSelectedRequestDirty = true;
	}
	
	if (ImGui::Button("Send###SendRequest"))
		SendRequest(SelectedRequestIndex);

	constexpr ImVec2 TextBoxSize = ImVec2(200, 400);
	constexpr int32 TextBoxFlags = ImGuiInputTextFlags_AllowTabInput;

	ImGui::Text("Content");
	if (ImGui::InputTextMultiline("###Content", &Content, TextBoxSize, TextBoxFlags))
	{
		SelectedRequest.Content = ANSI_TO_TCHAR(Content.c_str());
		bSelectedRequestDirty = true;
	}
}

void ULactoseRestSendDebugTab::DrawResponseSection()
{
	const bool bResponsePending = SentRequest.FutureResponseContext.IsValid() && !SentRequest.FutureResponseContext.IsReady();
	const bool bResponseReceived = RecentResponse.ResponseContext.IsValid();

	if (!bResponsePending && !bResponseReceived)
		return;
	
	ImGui::Text("Response");

	if (bResponsePending)
	{
		const double WaitingTime = (FDateTime::UtcNow() - SentRequest.TimeSent).GetTotalMilliseconds();
		ImGui::Text("Waiting for Response: %fms", WaitingTime);
		return;
	}

	const FHttpResponsePtr HttpResponse = RecentResponse.ResponseContext->HttpResponse;
	if (!HttpResponse)
	{
		ImGui::Text("Response is invalid");
		return;
	}

	ImGui::Text("Code: %d", HttpResponse->GetResponseCode());

	const double ResponseTimeMs = (RecentResponse.TimeReceived - SentRequest.TimeSent).GetTotalMilliseconds();
	ImGui::Text("Response Time: %fms", ResponseTimeMs);

	ImGui::Spacing();

	ImGui::Text("Content");

	const TArray<uint8>& ContentBytes = HttpResponse->GetContent();

	// Don't worry about the const case as this text box is going to be read only.
	char* ContentBytesRaw = reinterpret_cast<char*>(const_cast<uint8*>(ContentBytes.GetData()));

	constexpr int32 TextBoxFlags = ImGuiInputTextFlags_ReadOnly;
	constexpr ImVec2 TextBoxSize = ImVec2(200, 400);
	ImGui::InputTextMultiline("###ResponseContent", ContentBytesRaw, ContentBytes.Num(), TextBoxSize, TextBoxFlags);
}

void ULactoseRestSendDebugTab::CreateNewRequest()
{
	auto& NewRequest = SavedRequests.AddDefaulted_GetRef();
	NewRequest.Name = TEXT("New Request");
	NewRequest.Url = TEXT("https://");
}

void ULactoseRestSendDebugTab::ClearRequests()
{
	SavedRequests.Empty();
}

void ULactoseRestSendDebugTab::DeleteRequest(const int32 SavedRequestIdx)
{
	if (SavedRequests.IsValidIndex(SavedRequestIdx))
		SavedRequests.RemoveAt(SavedRequestIdx);
}

void ULactoseRestSendDebugTab::SendRequest(const int32 SavedRequestIdx)
{
	if (!SavedRequests.IsValidIndex(SavedRequestIdx))
		return;

	FLactoseRestDebugRequest& SelectedRequest = SavedRequests[SavedRequestIdx];

	auto RestSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
	if (!RestSubsystem)
		return;

	auto Request = Lactose::Rest::FRequest::Create(*RestSubsystem);
	Request->SetVerb(ANSI_TO_TCHAR(Verbs[SelectedRequest.Verb]));
	Request->SetUrl(SelectedRequest.Url);

	if (!SelectedRequest.Content.IsEmpty())
	{
		TArray<uint8> Content = Lactose::Serialisation::Json::StringToBytes(SelectedRequest.Content);
		Request->SetContent(MoveTemp(Content));
	}
		
	Request->GetOnResponseReceived().AddUObject(this, &ThisClass::OnResponseReceived);

	RecentResponse.ResponseContext.Reset();
	SentRequest.TimeSent = FDateTime::UtcNow();
	SentRequest.FutureResponseContext = Request->Send();

	SaveRequests();
}

void ULactoseRestSendDebugTab::SaveRequests()
{
	UpdateProjectUserConfigFile();
	bSelectedRequestDirty = false;
}

void ULactoseRestSendDebugTab::OnResponseReceived(TSharedRef<Lactose::Rest::IRequest::FResponseContext> Context)
{
	RecentResponse.TimeReceived = FDateTime::UtcNow();
	RecentResponse.ResponseContext = Context;
}
