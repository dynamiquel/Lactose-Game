#pragma once

#include <array>
#include "CoreMinimal.h"
#include "Math/Range.h"
#include "ImGuiModule.h"

/**
 * Converts a TChar string to an ANSI string.
 * TCHAR_TO_ANSI is deprecated on Linux but the new StringCast<ANSICHAR>(TCHAR).Get() is annoying to use.
 * Requires to be a struct since Unreal's string conversion utilises stack allocation.
 */
#ifndef STR_TO_ANSI
	#define STR_TO_ANSI(String) StringCast<ANSICHAR>(*(static_cast<FString>(String))).Get()
#endif

namespace Debug::ImGui
{
	template<typename TVectorDestination, typename TVectorSource>
	TVectorDestination ConvertVector2D(const TVectorSource& SourceVector)
	{
		return TVectorDestination(SourceVector.X, SourceVector.Y);
	}

	DEBUGOVERLAY_API ImVec4 Color(const FLinearColor& Color);

	template<typename TNumber>
	constexpr ImGuiDataType_ GetNumericDataType()
	{
		if constexpr (std::is_same_v<TNumber, double>)
		{
			return ImGuiDataType_Double;
		}
		else if constexpr (std::is_same_v<TNumber, float>)
		{
			return ImGuiDataType_Float;
		}
		else if constexpr (std::is_same_v<TNumber, int64>)
		{
			return ImGuiDataType_S64;
		}
		else if constexpr (std::is_same_v<TNumber, int32>)
		{
			return ImGuiDataType_S32;
		}
		else if constexpr (std::is_same_v<TNumber, int16>)
		{
			return ImGuiDataType_S16;
		}
		else if constexpr (std::is_same_v<TNumber, int8>)
		{
			return ImGuiDataType_S8;
		}
		else if constexpr (std::is_same_v<TNumber, uint64>)
		{
			return ImGuiDataType_U64;
		}
		else if constexpr (std::is_same_v<TNumber, uint32>)
		{
			return ImGuiDataType_U32;
		}
		else if constexpr (std::is_same_v<TNumber, uint16>)
		{
			return ImGuiDataType_U16;
		}
		else if constexpr (std::is_same_v<TNumber, uint8>)
		{
			return ImGuiDataType_U8;
		}
		else
		{
			static_assert("Provided an invalid number");
			return ImGuiDataType_COUNT;
		}
	}
	
	DEBUGOVERLAY_API bool CheckBoxTristate(const char* Label, TOptional<bool>& TristateValue);
	DEBUGOVERLAY_API bool DrawRangeInputField(const FString& Label, int32& Value, int32 Step = 1, int32 FastStep = 1);
	DEBUGOVERLAY_API bool DrawRangeInputField(const FString& Label, float& Value, float Step = 1.f, float FastStep = 1.f);
	
	template<typename RangeType> requires (std::is_arithmetic_v<RangeType>)
	bool RangeInput(const FString& Label, TRange<RangeType>& Range, RangeType Step = 1, RangeType FastStep = 1)
	{
		RangeType Min = Range.GetLowerBoundValue();
		RangeType Max = Range.GetUpperBoundValue();
	
		::ImGui::Text("%s", STR_TO_ANSI(Label));
		::ImGui::Indent();
		
		bool bAnyChanged = false;
		
		::ImGui::Text("Min");
		::ImGui::SameLine();
		if (DrawRangeInputField(Label + "Min", Min, Step, FastStep))
		{
			bAnyChanged = true;
		}
		
		::ImGui::Text("Max");
		::ImGui::SameLine();
		if (DrawRangeInputField(Label + "Max", Max, Step, FastStep))
		{
			bAnyChanged = true;
		}
		
		::ImGui::Unindent();
		::ImGui::Spacing();

		if (bAnyChanged)
		{
			// Ensure the Min is not below the absolute min.
			if (Min < 0)
			{
				Min = 0;
			}
	
			// Ensure the Max is never below the Min.
			if (Max < Min)
			{
				Max = Min;
			}

			Range.SetLowerBoundValue(Min);
			Range.SetUpperBoundValue(Max);
		}

		return bAnyChanged;
	}

	template<typename TNumber>
	bool SliderScalar(const FString& Label, TNumber& Value, const TNumber Min = 0, const TNumber Max = 1)
	{
		constexpr ImGuiDataType_ DataType = GetNumericDataType<TNumber>();
		
		::ImGui::Text("%s", STR_TO_ANSI(Label));
		::ImGui::Indent();
		const bool bChanged = ::ImGui::SliderScalar(STR_TO_ANSI(*("##" + Label)), DataType, &Value, &Min, &Max);
		::ImGui::Unindent();

		Value = FMath::Clamp(Value, Min, Max);

		return bChanged;
	}

	/**
	 * Helper class for creating a Search Box within ImGui. It contains its own label, input buffer and clear button.
	 */
	class DEBUGOVERLAY_API FSearchBox
	{
	public:
		FSearchBox() = default;

		explicit FSearchBox(FString&& InLabel, ImGuiInputTextFlags InInputFlags = 0)
			: Label(MoveTemp(InLabel))
			, InputFlags(InInputFlags)
			, Buffer{}
		{ }

		bool Draw();
		void Clear();
		bool IsEmpty() const;
		bool PassesFilter(const FStringView CompareTo) const;

	private:
		FString Label;
		ImGuiInputTextFlags InputFlags = 0;
		
		static constexpr uint32 BufferSize = 128;
		std::array<char, BufferSize> Buffer;
	};

	DEBUGOVERLAY_API void Error(const char* Format, ...);
}