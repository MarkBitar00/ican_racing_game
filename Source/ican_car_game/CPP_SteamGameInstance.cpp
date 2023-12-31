#include "CPP_SteamGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UCPP_SteamGameInstance::UCPP_SteamGameInstance()
{
}

void UCPP_SteamGameInstance::Init()
{
	Super::Init();

	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		SessionInterface = Subsystem->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UCPP_SteamGameInstance::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UCPP_SteamGameInstance::OnFindSessionComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UCPP_SteamGameInstance::OnJoinSessionComplete);
		}
	}
}

void UCPP_SteamGameInstance::OnCreateSessionComplete(FName SessionName, bool Succeeded)
{
	if (Succeeded)
	{
		GetWorld()->ServerTravel("/Game/Levels/LEVEL_GA?listen");
	}
}

void UCPP_SteamGameInstance::OnFindSessionComplete(bool Succeeded)
{
	if (Succeeded)
	{
		TArray<FOnlineSessionSearchResult> SearchResults = SessionSearch->SearchResults;

		/*if (SearchResults.Num())
		{
			SessionInterface->JoinSession(0, FName("Polar Drift Steam Session"), SearchResults[0]);
		}*/
	}
}

void UCPP_SteamGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		FString JoinAddress = "";
		SessionInterface->GetResolvedConnectString(SessionName, JoinAddress);

		if (JoinAddress != "")
		{
			PlayerController->ClientTravel(JoinAddress, ETravelType::TRAVEL_Absolute);
		}
	}
}

void UCPP_SteamGameInstance::OnDestroySessionComplete(bool Succeeded)
{
	if (Succeeded)
	{
		UE_LOG(LogTemp, Warning, TEXT("DestroyServer"));
	}
}

void UCPP_SteamGameInstance::CreateServer(FName SessionName, int Slots)
{
	UE_LOG(LogTemp, Warning, TEXT("CreateServer"));

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bIsDedicated = false;
	SessionSettings.bIsLANMatch = (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL");
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.NumPublicConnections = Slots;

	SessionInterface->CreateSession(0, SessionName, SessionSettings);
}

void UCPP_SteamGameInstance::JoinServer()
{
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL");
	SessionSearch->MaxSearchResults = 10000;
	SessionSearch->QuerySettings.Set("SEARCH_PRESENCE", true, EOnlineComparisonOp::Equals);

	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void UCPP_SteamGameInstance::FindServers()
{
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL");
	SessionSearch->MaxSearchResults = 10000;
	SessionSearch->QuerySettings.Set("SEARCH_PRESENCE", true, EOnlineComparisonOp::Equals);

	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void UCPP_SteamGameInstance::DestroyServer()
{
	SessionInterface->DestroySession(FName("Polar Drift Steam Session"));
}
