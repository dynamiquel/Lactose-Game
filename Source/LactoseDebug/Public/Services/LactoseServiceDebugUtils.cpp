#include "LactoseServiceDebugUtils.h"

#include <imgui.h>

#include "DebugImGuiHelpers.h"
#include "Services/LactoseServiceSubsystem.h"

Lactose::Debug::Services::FStatusSection::FStatusSection(ULactoseServiceSubsystem* InServiceSubsystem)
	: ServiceSubsystem(InServiceSubsystem)
	, ServiceStatusResponseTimeMs(0)
{
}

bool Lactose::Debug::Services::FStatusSection::Render()
{
	ULactoseServiceSubsystem* ServiceSubsystemPinned = ServiceSubsystem.Get();
	if (!ServiceSubsystemPinned)
	{
		::Debug::ImGui::Error("Identity Subsystem could not be found");
		return false;
	}

	if (ImGui::Button("Refresh"))
	{
		ServiceStatusResponseTimeMs = 0.;
		
		ServiceSubsystemPinned->GetServiceInfo().Next([ThisWeak = AsWeak()]
			(TSharedPtr<FGetServiceStatusRequest::FResponseContext> ResponseContext)
		{
			auto This = ThisWeak.Pin();
			if (!This)
				return;
			
			This->ServiceStatus = ResponseContext->ResponseContent;
			This->ServiceStatusResponseTimeMs = ResponseContext->GetLatencyMs();
		});
	}

	if (ServiceStatus)
	{
		if (ServiceStatusResponseTimeMs > 0)
		{
			ImGui::SameLine();
			ImGui::Text("Latency: %0.0f ms", ServiceStatusResponseTimeMs);
		}
		
		constexpr int32 Columns = 2;
		constexpr int32 Flags = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_SizingStretchProp;
		if (ImGui::BeginTable("IdentityStatusTable", Columns, Flags))
		{
			constexpr int32 NameColumnFlags = ImGuiTableColumnFlags_WidthFixed;
			constexpr int32 NameColumnWidth = 100.f;
			ImGui::TableSetupColumn("Name", NameColumnFlags, NameColumnWidth);
			ImGui::TableSetupColumn("Value");

			ImGui::TableNextColumn();
			ImGui::Text("Name");
			ImGui::TableNextColumn();
			ImGui::Text("%s", STR_TO_ANSI(ServiceStatus->Name));
			ImGui::TableNextColumn();
			ImGui::Text("Description");
			ImGui::TableNextColumn();
			ImGui::Text("%s", STR_TO_ANSI(ServiceStatus->Description));
			ImGui::TableNextColumn();
			ImGui::Text("Dependencies");
			ImGui::TableNextColumn();
			for (const FString& Dependency : ServiceStatus->Dependencies)
				ImGui::Text("%s", STR_TO_ANSI(Dependency));
			ImGui::TableNextColumn();
			ImGui::Text("Version");
			ImGui::TableNextColumn();
			ImGui::Text("%s", STR_TO_ANSI(ServiceStatus->Version));
			ImGui::TableNextColumn();
			ImGui::Text("Build Time");
			ImGui::TableNextColumn();
			ImGui::Text("%s", STR_TO_ANSI(ServiceStatus->BuildTime.ToString()));
			ImGui::TableNextColumn();
			ImGui::Text("Status");
			ImGui::TableNextColumn();
			ImGui::Text("%d", ServiceStatus->Status);
			ImGui::TableNextColumn();
			ImGui::Text("Runtime");
			ImGui::TableNextColumn();
			ImGui::Text("%s", STR_TO_ANSI(ServiceStatus->Runtime));
			ImGui::TableNextColumn();
			ImGui::Text("OS");
			ImGui::TableNextColumn();
			ImGui::Text("%s", STR_TO_ANSI(STR_TO_ANSI(ServiceStatus->OperatingSystem)));
			ImGui::TableNextColumn();
			ImGui::Text("Start Time");
			ImGui::TableNextColumn();
			ImGui::Text("%s", STR_TO_ANSI(ServiceStatus->StartTime.ToString()));
			ImGui::TableNextColumn();
			ImGui::Text("Uptime");
			ImGui::TableNextColumn();
			ImGui::Text("%s", STR_TO_ANSI(ServiceStatus->Uptime));
			
			ImGui::EndTable();
		}
	}

	return true;
}
