#pragma once

struct FLactoseServiceInfo;
class ULactoseServiceSubsystem;

namespace Lactose::Debug::Services
{
	class FStatusSection : public TSharedFromThis<FStatusSection>
	{
	public:
		explicit FStatusSection(ULactoseServiceSubsystem* InServiceSubsystem);
		bool Render();

	private:
		TWeakObjectPtr<ULactoseServiceSubsystem> ServiceSubsystem;
		TSharedPtr<FLactoseServiceInfo> ServiceStatus;
		double ServiceStatusResponseTimeMs;
	};
}
