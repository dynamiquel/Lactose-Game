// Fill out your copyright notice in the Description page of Project Settings.


#include "Rest/LactoseRestSendDebugTab.h"

#include <imgui.h>

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
	// TODO: Format this better.
	// Table for columns
	// Wrapped output
	// Multiline and wrapped input
	// Show status for sending message.
	// Show how long current request is taking.
	// Show how long a response took.
	// Allow users to create and save requests.
	
	Super::Render();
	
	ImGui::Combo("Verb", &SelectedVerbIndex, Verbs.data(), Verbs.size());
	ImGui::InputText("URL", UrlBuffer.data(), UrlBuffer.size());
	ImGui::InputText("Body", PayloadBuffer.data(), PayloadBuffer.size());

	const bool bResponsePending = FutureResponseContext.IsValid() && !FutureResponseContext.IsReady();

	if (!bResponsePending && ImGui::Button("Send"))
	{
		auto RestSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<ULactoseRestSubsystem>();
		if (!RestSubsystem)
			return;

		if (SelectedVerbIndex < 0 || SelectedVerbIndex >= Verbs.size())
			return;

		LastResponseContext.Reset();

		const FString Verb = ANSI_TO_TCHAR(Verbs[SelectedVerbIndex]);

		auto Request = Lactose::Rest::FRequest::Create(*RestSubsystem);
		Request->SetVerb(Verb);
		Request->SetUrl(ANSI_TO_TCHAR(UrlBuffer.data()));

		if (PayloadBuffer[0] != '\0')
		{
			TArray<uint8> Content;
			Content.Append(reinterpret_cast<uint8*>(PayloadBuffer.data()), PayloadBuffer.size());
			Request->SetContent(MoveTemp(Content));
		}
		
		Request->GetOnResponseReceived().AddUObject(this, &ThisClass::OnResponseReceived);
		FutureResponseContext = Request->Send();
	}

	if (!LastResponseContext)
		return;

	if (!LastResponseContext->HttpResponse)
		return;
	
	ImGui::Text("Code: %d", LastResponseContext->HttpResponse->GetResponseCode());
	ImGui::Text("%s", TCHAR_TO_ANSI(*LastResponseContext->HttpResponse->GetContentAsString()));
}

void ULactoseRestSendDebugTab::OnResponseReceived(TSharedRef<Lactose::Rest::IRequest::FResponseContext> Context)
{
	LastResponseContext = Context;
}
