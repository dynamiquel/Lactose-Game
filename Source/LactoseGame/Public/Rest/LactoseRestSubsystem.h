#pragma once

#include "CoreMinimal.h"
#include "Simp.h"
#include "HttpModule.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ILactoseRestRequest.h"

#include "LactoseRestSubsystem.generated.h"

UCLASS()
class LACTOSEGAME_API ULactoseRestSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Begin override UGameInstanceSubsystem
	void Initialize(FSubsystemCollectionBase& Collection) override;
	// End override UGameInstanceSubsystem

	const TOptional<FString>& GetRefreshToken() const { return RefreshToken; }
	const TOptional<FString>& GetAccessToken() const { return AccessToken; }

	template<typename TRequest> requires (std::is_base_of_v<Lactose::Rest::IRequest, TRequest>)
	Sr<TRequest> CreateRequest()
	{
		FHttpModule& HttpModule = FHttpModule::Get();
		Sr<IHttpRequest> HttpRequest = HttpModule.CreateRequest();
		HttpRequest->SetVerb(Lactose::Rest::Verbs::GET);
		Sr<TRequest> LactoseRequest = CreateSr<TRequest>(this, HttpRequest);
		return LactoseRequest;
	}

	bool SendRequest(const Sr<Lactose::Rest::IRequest>& Request);
	void RemoveRequest(const Sr<Lactose::Rest::IRequest>& Request);

	void AddAuthorization(const FString& InAccessToken, const FString* InRefreshToken);
	void RemoveAuthorization();

protected:
	void LoadRefreshToken();
	void SaveRefreshToken();
	void DeleteRefreshToken();

protected:
	UPROPERTY(EditDefaultsOnly)
	FString RefreshTokenFilePath = TEXT("accesstoken.txt");

private:
	// Ensures Requests stay alive while they are being processed.
	TSet<Sr<Lactose::Rest::IRequest>> PendingRequests;

	FCriticalSection PendingRequestsLock;

	TOptional<FString> RefreshToken;
	TOptional<FString> AccessToken;
};

namespace Lactose::Rest
{
	DECLARE_MULTICAST_DELEGATE_OneParam(FRestRequestDelegate,
		const Sr<Lactose::Rest::IRequest>& Request);

	inline FRestRequestDelegate OnRequestFailed;
}
