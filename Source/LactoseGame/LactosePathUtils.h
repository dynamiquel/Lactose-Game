#pragma once

namespace Lactose::Paths
{
	FString GetFileNameFromPackage(const FString& PackagePath);
	FString GetClassPackagePath(const FString& ShortPackagePath);
	FString GetObjectPackagePathWithSelf(const FString& ShortPackagePath);
	bool IsPackagePathAlreadyComplete(const FString& PackagePath);
}
