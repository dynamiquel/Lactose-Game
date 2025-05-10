#include "Rest/LactoseRestSubsystem.h"

#include "Rest/LactoseRestLog.h"


void ULactoseRestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	LoadRefreshToken();
}

bool ULactoseRestSubsystem::SendRequest(const Sr<Lactose::Rest::IRequest>& Request)
{
	if (Request->GetInternal()->ProcessRequest())
	{
		FScopeLock Lock(&PendingRequestsLock);
		PendingRequests.Add(Request);

		Log::Verbose(LogLactoseRest
			, TEXT("Sent Request to '%s'. Content:\n%s"),
			*Request->GetInternal()->GetURL(),
			*Request->GetContentString());
		
		return true;
	}

	return false;
}

void ULactoseRestSubsystem::RemoveRequest(const Sr<Lactose::Rest::IRequest>& Request)
{
	FScopeLock Lock(&PendingRequestsLock);
	PendingRequests.Remove(Request);
}

void ULactoseRestSubsystem::AddAuthorization(const FString& InAccessToken, const FString* InRefreshToken)
{
	FHttpModule& HttpModule = FHttpModule::Get();

	AccessToken = InAccessToken;
	
	HttpModule.AddDefaultHeader(
		TEXT("Authorization"),
		FString::Printf(TEXT("Bearer %s"), *InAccessToken));

	if (InRefreshToken)
	{
		RefreshToken = *InRefreshToken;
		SaveRefreshToken();
	}
	else
	{
		RefreshToken.Reset();
	}
}

void ULactoseRestSubsystem::RemoveAuthorization()
{
	FHttpModule& HttpModule = FHttpModule::Get();
	
	HttpModule.AddDefaultHeader(
		TEXT("Authorization"),
		FString());

	DeleteRefreshToken();
}

void ULactoseRestSubsystem::LoadRefreshToken()
{
	const FString AccessTokenFile = FPaths::ProjectUserDir() / RefreshTokenFilePath;

	if (FPaths::FileExists(AccessTokenFile))
	{
		RefreshToken = FString();
		if (FFileHelper::LoadFileToString(OUT *RefreshToken, *AccessTokenFile))
		{
			UE_LOG(LogLactoseRest, Log, TEXT("Found and loaded user's auth refresh token"));
		}
		else
		{
			UE_LOG(LogLactoseRest, Error, TEXT("Failed to load user's auth refresh token"));
			RefreshToken.Reset();
		}
	}
}

void ULactoseRestSubsystem::SaveRefreshToken()
{
	const FString AccessTokenFile = FPaths::ProjectUserDir() / RefreshTokenFilePath;

	check(RefreshToken);
	
	if (FFileHelper::SaveStringToFile(*RefreshToken, *AccessTokenFile))
	{
		UE_LOG(LogLactoseRest, Log, TEXT("Saved user's auth refresh token"));
	}
	else
	{
		UE_LOG(LogLactoseRest, Error, TEXT("Failed to save user's auth refresh token"));
	}
}

void ULactoseRestSubsystem::DeleteRefreshToken()
{
	RefreshToken.Reset();
	
	const FString AccessTokenFile = FPaths::ProjectUserDir() / RefreshTokenFilePath;
	
	IFileManager::Get().Delete(*AccessTokenFile);
}
