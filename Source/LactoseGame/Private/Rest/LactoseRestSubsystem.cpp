#include "Rest/LactoseRestSubsystem.h"

#include "Rest/LactoseRestLog.h"
#include "IPlatformCrypto.h"

constexpr auto RefreshTokenKeySalt = TEXT("You know what they say about jokes about salt... you gotta take 'em with a grain of salt!");

TUniquePtr<FEncryptionContext> EncryptionContext;

TArray<uint8> GetEncryptionKey()
{
	// Basic encryption key via obfuscation; better than nothing.
	// Without platform-specific encryption support, all encryption sucks anyway.
	// I think ideally, the Refresh Token should already have some kind of spoof-protection.
	const FString ComputerName = FPlatformProcess::ComputerName();
	const FString UserName = FPlatformProcess::UserName();
	const FString AppDir = FPlatformProcess::BaseDir();
	const FString KeySeed = ComputerName + UserName + AppDir + RefreshTokenKeySalt;

	FTCHARToUTF8 Converter(*KeySeed);
	TArray<uint8> KeySeedBytes;
	KeySeedBytes.SetNum(Converter.Length());
	FMemory::Memcpy(KeySeedBytes.GetData(), Converter.Get(), Converter.Length());

	TArray<uint8> Key;
	EncryptionContext->CalcSHA256(KeySeedBytes, OUT Key);
	
	return Key;
}

TArray<uint8> EncryptString(const FString& InString)
{
	FTCHARToUTF8 Converter(*InString);

	TArray<uint8> Bytes;
	Bytes.SetNum(Converter.Length());
	FMemory::Memcpy(Bytes.GetData(), Converter.Get(), Converter.Length());

	EPlatformCryptoResult EncryptResult;
	TArray<uint8> EncryptedBytes = EncryptionContext->Encrypt_AES_256_ECB(Bytes, GetEncryptionKey(), OUT EncryptResult);

	return EncryptedBytes;
}

FString DecryptString(const TArray<uint8>& InBytes)
{
	EPlatformCryptoResult DecryptResult;
	TArray<uint8> DecryptedBytes = EncryptionContext->Decrypt_AES_256_ECB(InBytes, GetEncryptionKey(), OUT DecryptResult);

	if (DecryptedBytes.Num() == 0)
		return {};
	
	FUTF8ToTCHAR Converter(reinterpret_cast<const UTF8CHAR*>(DecryptedBytes.GetData()), DecryptedBytes.Num());
	return FString(Converter.Length(), Converter.Get());
}

void ULactoseRestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	EncryptionContext = IPlatformCrypto::Get().CreateContext();
	
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
	const FString AccessTokenFile = FPaths::ProjectUserDir() / TEXT("Saved") / RefreshTokenFilePath;

	if (FPaths::FileExists(AccessTokenFile))
	{
		TArray<uint8> EncryptedRefreshToken;
		if (FFileHelper::LoadFileToArray(OUT EncryptedRefreshToken, *AccessTokenFile))
		{
			RefreshToken = DecryptString(EncryptedRefreshToken);
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
	const FString AccessTokenFile = FPaths::ProjectUserDir() / TEXT("Saved") / RefreshTokenFilePath;

	check(RefreshToken);

	TArray<uint8> EncryptedAccessToken = EncryptString(*RefreshToken);
	if (FFileHelper::SaveArrayToFile(EncryptedAccessToken, *AccessTokenFile))
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
	
	const FString AccessTokenFile = FPaths::ProjectUserDir() / TEXT("Saved") / RefreshTokenFilePath;
	
	IFileManager::Get().Delete(*AccessTokenFile);
}
