#include "DebugAppTab.h"

void UDebugAppTab::SetTabName(FString&& InTabName)
{
	check(!InTabName.IsEmpty());
	TabName = InTabName;
}
