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
	if (ImGui::BeginTable("###RestClientTable", Columns, Flags))
	{
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

	if (!SavedRequests.Requests.IsEmpty())
		if (ImGui::Button("Clear"))
			ClearRequests();
	
	ImGui::Spacing();

	constexpr int32 Columns = 3;
	constexpr int32 Flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;
	if (ImGui::BeginTable("Requests Table", Columns, Flags))
	{
		ImGui::TableSetupColumn("###RequestNameColumn");

		constexpr int32 ButtonColumnFlags = ImGuiTableColumnFlags_WidthFixed;
		ImGui::TableSetupColumn("###SendRequestColumn", ButtonColumnFlags);
		ImGui::TableSetupColumn("###DeleteRequestColumn", ButtonColumnFlags);

		for (int32 RequestIdx = 0; RequestIdx < SavedRequests.Requests.Num(); RequestIdx++)
		{
			const FString UniqueRequestId = TEXT("###") + LexToString(RequestIdx);
			ImGui::TableNextColumn();
		
			const bool bSelected = SelectedRequestIndex == RequestIdx;
			const FString& RequestName = SavedRequests.Requests[RequestIdx].Name;
			const FString Label = RequestName + UniqueRequestId;

			if (ImGui::Selectable(STR_TO_ANSI(Label), bSelected))
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
}

void ULactoseRestSendDebugTab::DrawSelectedRequestSection()
{
	if (!SavedRequests.Requests.IsValidIndex(SelectedRequestIndex))
		return;
	
	FLactoseRestDebugRequest& SelectedRequest = SavedRequests.Requests[SelectedRequestIndex];

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
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	if (ImGui::InputText("###Name", &Name))
	{
		SelectedRequest.Name = ANSI_TO_TCHAR(Name.c_str());
		bSelectedRequestDirty = true;
	}

	ImGui::Text("Verb:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	if (ImGui::Combo("###Verb", &SelectedRequest.Verb, Verbs.data(), Verbs.size()))
	{
		bSelectedRequestDirty = true;
	}

	ImGui::Text("URL:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	if (ImGui::InputText("###URL", &RequestUrl))
	{
		SelectedRequest.Url = ANSI_TO_TCHAR(RequestUrl.c_str());
		bSelectedRequestDirty = true;
	}
	
	if (ImGui::Button("Send###SendRequest"))
		SendRequest(SelectedRequestIndex);

	ImGui::Text("Content");

	constexpr ImVec2 RequestContentSectionSize = ImVec2();
	constexpr bool bRequestContentSectionBorder = false;
	constexpr int32 RequestContentSectionFlags = ImGuiWindowFlags_HorizontalScrollbar;
	if (ImGui::BeginChild("RequestContentSection", RequestContentSectionSize, bRequestContentSectionBorder, RequestContentSectionFlags))
	{
		const ImVec2 TextBoxSize = ImVec2(-std::numeric_limits<float>::min(), ImGui::GetContentRegionAvail().y);
		constexpr int32 TextBoxFlags = ImGuiInputTextFlags_AllowTabInput;

		if (ImGui::InputTextMultiline("###Content", &Content, TextBoxSize, TextBoxFlags))
		{
			SelectedRequest.Content = ANSI_TO_TCHAR(Content.c_str());
			bSelectedRequestDirty = true;
		}

		ImGui::EndChild();
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
		ImGui::Text("Waiting for Response: %0.0f ms", WaitingTime);
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
	ImGui::Text("Response Time: %0.0f ms", ResponseTimeMs);

	ImGui::Spacing();

	const TArray<uint8>& ContentBytes = HttpResponse->GetContent();

	if (!ContentBytes.IsEmpty())
	{
		ImGui::Text("Content");
		
		constexpr ImVec2 ResponseContentSectionSize = ImVec2();
		constexpr bool bResponseContentSectionBorder = false;
		constexpr int32 ResponseContentSectionFlags = ImGuiWindowFlags_HorizontalScrollbar;
		if (ImGui::BeginChild("ResponseContentSection", ResponseContentSectionSize, bResponseContentSectionBorder, ResponseContentSectionFlags))
		{
			// Content is converted to a new string because it is still possible for a read-only text box to be
			// modified. We don't want some crash due to accidental modification of the const content data.
			auto ContentString = std::string(reinterpret_cast<const char*>(ContentBytes.GetData()), ContentBytes.Num());
			
			const ImVec2 TextBoxSize = ImVec2(-std::numeric_limits<float>::min(), ImGui::GetContentRegionAvail().y);
			constexpr int32 TextBoxFlags = ImGuiInputTextFlags_ReadOnly;
			ImGui::InputTextMultiline("###ResponseContent", &ContentString, TextBoxSize, TextBoxFlags);

			ImGui::EndChild();
		}
	}
}

void ULactoseRestSendDebugTab::CreateNewRequest()
{
	auto& NewRequest = SavedRequests.Requests.AddDefaulted_GetRef();
	NewRequest.Name = TEXT("New Request");
	NewRequest.Url = TEXT("https://");
}

void ULactoseRestSendDebugTab::ClearRequests()
{
	SavedRequests.Requests.Empty();
}

void ULactoseRestSendDebugTab::DeleteRequest(const int32 SavedRequestIdx)
{
	if (SavedRequests.Requests.IsValidIndex(SavedRequestIdx))
		SavedRequests.Requests.RemoveAt(SavedRequestIdx);
}

void ULactoseRestSendDebugTab::SendRequest(const int32 SavedRequestIdx)
{
	if (!SavedRequests.Requests.IsValidIndex(SavedRequestIdx))
		return;

	FLactoseRestDebugRequest& SelectedRequest = SavedRequests.Requests[SavedRequestIdx];

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

void ULactoseRestSendDebugTab::OnResponseReceived(Sr<Lactose::Rest::IRequest::FResponseContext> Context)
{
	RecentResponse.TimeReceived = FDateTime::UtcNow();
	RecentResponse.ResponseContext = Context;
}
