#include "LactoseMotdsSubsystem.h"

#include "LactoseGame/LactoseGame.h"
#include "Services/ConfigCloud/LactoseConfigCloudServiceSubsystem.h"

void ULactoseMotdsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency<ULactoseConfigCloudServiceSubsystem>();

	Lactose::Config::Events::OnConfigLoaded.AddUObject(this, &ThisClass::OnConfigCloudLoaded);
}

void ULactoseMotdsSubsystem::OnConfigCloudLoaded(
	const ULactoseConfigCloudServiceSubsystem& Sender,
	Sr<FLactoseConfigCloudConfig> Config)
{
	Sp<FLactoseConfigCloudEntry> FoundMotds = Config->FindEntry(TEXT("motds"));
	if (!FoundMotds)
	{
		return Log::Error(LogLactose, TEXT("Couldn't find Message of the Days (motds) in the Config Cloud"));
	}

	const FLactoseMotds* MotdsConverted = FoundMotds->Get<FLactoseMotds>();
	if (!MotdsConverted)
	{
		return Log::Error(LogLactose,
			TEXT("Was unable to convert Message of the Days into FLactoseMotds. Json:%s"),
			*FoundMotds->GetString());
	}

	Motds = CreateSr(*MotdsConverted);
}

FLactoseMotds ULactoseMotdsSubsystem::GetMotdsCopy() const
{
	if (!Motds)
		return {};

	return *Motds;
}
