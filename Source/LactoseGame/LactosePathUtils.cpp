#include "LactosePathUtils.h"

FString Lactose::Paths::GetFileNameFromPackage(const FString& PackagePath)
{
	// Returns the bottom-level Filename from a Package Path.
	// "/Game/Crops/BP_Crop_Carrot" -> "BP_Crop_Carrot" 
	
	int32 NameStart = INDEX_NONE;
	if (!PackagePath.FindLastChar(TEXT('/'), OUT NameStart))
		return FString();

	const FString FileName = PackagePath.RightChop(NameStart + 1);
	return FileName;
}

FString Lactose::Paths::GetClassPackagePath(const FString& ShortPackagePath)
{
	// If the object doesn't end with _C, then it is a 'short' path that doesn't actually
	// reference the class but we want the class, so fix here.
	// "/Game/Crops/BP_Crop_Carrot" -> "/Game/Crops/BP_Crop_Carrot.BP_Crop_Carrot_C" 
	
	// Check if class suffix is already included.
	if (ShortPackagePath.EndsWith(TEXT("_C")))
		return ShortPackagePath;
	
	const FString FileName = GetFileNameFromPackage(ShortPackagePath);
	if (FileName.IsEmpty())
		return FString();

	const FString CompletePath = ShortPackagePath + FString::Printf(TEXT(".%s_C"), *FileName);
	return CompletePath;
}

FString Lactose::Paths::GetObjectPackagePathWithSelf(const FString& ShortPackagePath)
{
	if (IsPackagePathAlreadyComplete(ShortPackagePath))
		return ShortPackagePath;

	const FString FileName = GetFileNameFromPackage(ShortPackagePath);

	const FString CompletePath = ShortPackagePath + FString::Printf(TEXT(".%s"), *FileName);
	return CompletePath;
}

bool Lactose::Paths::IsPackagePathAlreadyComplete(const FString& PackagePath)
{
	// Checks whether a Package Path is already complete.
	// "/Game/Crops/BP_Crop_Carrot" -> false
	// "/Game/Crops/BP_Crop_Carrot.BP_Crop_Carrot" -> true
	
	return PackagePath.Contains(TEXT("."), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
}
