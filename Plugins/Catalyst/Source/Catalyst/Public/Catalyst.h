// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

CATALYST_API DECLARE_LOG_CATEGORY_EXTERN(LogCatalyst, Log, All);

class FCatalystModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
