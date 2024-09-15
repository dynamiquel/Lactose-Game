#include "DebugImGuiHelpers.h"

#include "ThirdParty/ImGuiLibrary/Private/imgui_internal.h"

ImVec4 Debug::ImGui::Color(const FLinearColor& Color)
{
	return ImVec4(Color.R, Color.G, Color.B, Color.A);
}

bool Debug::ImGui::CheckBoxTristate(const char* Label, TOptional<bool>& TristateValue)
{
	bool bReturnValue;

	if (!TristateValue)
	{
		::ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, true);
		bool bTempValue = false;
		bReturnValue = ::ImGui::Checkbox(Label, &bTempValue);
		if (bReturnValue)
		{
			TristateValue = true;
		}
		::ImGui::PopItemFlag();
	}
	else
	{
		bool bTempValue = *TristateValue;
		bReturnValue = ::ImGui::Checkbox(Label, &bTempValue);
		if (bReturnValue)
		{
			TristateValue = bTempValue;
		}
	}
	
	return bReturnValue;
}

bool Debug::ImGui::DrawRangeInputField(const FString& Label, int32& Value, const int32 Step, const int32 FastStep)
{
	return ::ImGui::InputInt(STR_TO_ANSI((TEXT("##") + Label)), &Value, Step, FastStep);
}

bool Debug::ImGui::DrawRangeInputField(const FString& Label, float& Value, const float Step, const float FastStep)
{
	return ::ImGui::InputFloat(STR_TO_ANSI((TEXT("##") + Label)), &Value, Step, FastStep, "%.f");
}

bool Debug::ImGui::FSearchBox::Draw()
{
	::ImGui::Text("Search:");
	::ImGui::SameLine();
	
	bool bChanged = ::ImGui::InputText(
		STR_TO_ANSI(Label),
		Buffer.data(),
		Buffer.size(),
		InputFlags);

	if (!IsEmpty())
	{
		// Show a clear button.
		::ImGui::SameLine();
		if (::ImGui::Button(STR_TO_ANSI(FString::Printf(TEXT("X###Clear%sSearch"), *Label))))
		{
			Clear();
			bChanged = true;
		}
	}
	
	return bChanged;
}

void Debug::ImGui::FSearchBox::Clear()
{
	Buffer.fill('\0');
}

bool Debug::ImGui::FSearchBox::IsEmpty() const
{
	// Considered empty if the first character is null.
	return Buffer.front() == '\0';
}

bool Debug::ImGui::FSearchBox::PassesFilter(const FStringView CompareTo) const
{
	return IsEmpty() || CompareTo.Contains(ANSI_TO_TCHAR(Buffer.data()), ESearchCase::IgnoreCase);
}

void Debug::ImGui::Error(const char* Format, ...)
{
	ImVec4 ColorVec = Color(FLinearColor::Red);
	
	::ImGui::TextColored(ColorVec, "Error: ");
	::ImGui::SameLine();
	va_list args;
	va_start(args, Format);
	::ImGui::TextColored(ColorVec, Format, args);
	va_end(args);
}
