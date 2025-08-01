// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#include "ImGuiDrawData.h"


#if ENGINE_COMPATIBILITY_LEGACY_CLIPPING_API
void FImGuiDrawList::CopyVertexData(TArray<FSlateVertex>& OutVertexBuffer, const FTransform2D& Transform, const FSlateRotatedRect& VertexClippingRect) const
#else
void FImGuiDrawList::CopyVertexData(TArray<FSlateVertex>& OutVertexBuffer, const FTransform2D& Transform) const
#endif // ENGINE_COMPATIBILITY_LEGACY_CLIPPING_API
{
	// Reset and reserve space in destination buffer.
	OutVertexBuffer.SetNumUninitialized(ImGuiVertexBuffer.Size, false);

	// Transform and copy vertex data.
	for (int Idx = 0; Idx < ImGuiVertexBuffer.Size; Idx++)
	{
		const ImDrawVert& ImGuiVertex = ImGuiVertexBuffer[Idx];
		FSlateVertex& SlateVertex = OutVertexBuffer[Idx];

		// Final UV is calculated in shader as XY * ZW, so we need set all components.
		SlateVertex.TexCoords[0] = ImGuiVertex.uv.x;
		SlateVertex.TexCoords[1] = ImGuiVertex.uv.y;
		SlateVertex.TexCoords[2] = SlateVertex.TexCoords[3] = 1.f;

#if ENGINE_COMPATIBILITY_LEGACY_CLIPPING_API
		const FVector2D VertexPosition = Transform.TransformPoint(ImGuiInterops::ToVector2D(ImGuiVertex.pos));
		SlateVertex.Position[0] = VertexPosition.X;
		SlateVertex.Position[1] = VertexPosition.Y;
		SlateVertex.ClipRect = VertexClippingRect;
		// @SPLASH_DAMAGE_CHANGE: ben.laws@splashdamage.com - BEGIN: Fix UE5 compile
#elif ENGINE_COMPATIBILITY_LEGACY_WORLD_COORDINATES
		SlateVertex.Position = Transform.TransformPoint(ImGuiInterops::ToVector2D(ImGuiVertex.pos));
#else
		SlateVertex.Position = FVector2f{ Transform.TransformPoint(ImGuiInterops::ToVector2D(ImGuiVertex.pos)) };
#endif
		// @SPLASH_DAMAGE_CHANGE: ben.laws@splashdamage.com - END

		// Unpack ImU32 color.
		SlateVertex.Color = ImGuiInterops::UnpackImU32Color(ImGuiVertex.col);
	}
}

void FImGuiDrawList::CopyIndexData(TArray<SlateIndex>& OutIndexBuffer, const int32 StartIndex, const int32 NumElements) const
{
	// Reset buffer.
	OutIndexBuffer.SetNumUninitialized(NumElements, false);

	// Copy elements (slow copy because of different sizes of ImDrawIdx and SlateIndex and because SlateIndex can
	// have different size on different platforms).
	for (int i = 0; i < NumElements; i++)
	{
		OutIndexBuffer[i] = ImGuiIndexBuffer[StartIndex + i];
	}
}

void FImGuiDrawList::TransferDrawData(ImDrawList& Src)
{
	// Move data from source to this list.
	Src.CmdBuffer.swap(ImGuiCommandBuffer);
	Src.IdxBuffer.swap(ImGuiIndexBuffer);
	Src.VtxBuffer.swap(ImGuiVertexBuffer);

	// ImGui seems to clear draw lists in every frame, but since source list can contain pointers to buffers that
	// we just swapped, it is better to clear explicitly here.
	// @SPLASH_DAMAGE_CHANGE: liam.potter@bulkhead.com - BEGIN: Function no longer exists in ImGui 1.88.
	// Src.Clear();
	// @SPLASH_DAMAGE_CHANGE: liam.potter@bulkhead.com - END
}
