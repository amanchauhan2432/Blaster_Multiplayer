// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionSubsystem.h"
#include "OnlineSubsystem.h"

UMultiplayerSessionSubsystem::UMultiplayerSessionSubsystem():
    CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete))
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem && GEngine)
    {
        Interface = Subsystem->GetSessionInterface();
    }
}

void UMultiplayerSessionSubsystem::CreateGameSession(int32 NumPublicConnections, FString MatchType)
{
	if (!Interface.IsValid()) { return;}

	if (Interface->GetNamedSession(NAME_GameSession))
	{
		Interface->DestroySession(NAME_GameSession);
	}

	CreateSessionCompleteDelegateHandle = Interface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	SessionSettings->bIsLANMatch = false;
	SessionSettings->NumPublicConnections = NumPublicConnections;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bUsesPresence = true;
	SessionSettings->bUseLobbiesIfAvailable = true;
	SessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings->BuildUniqueId = 1;

	if (!Interface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessionSettings))
    {
        Interface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
    }
}

void UMultiplayerSessionSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		GetWorld()->ServerTravel(LobbyPath);

        Interface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString(TEXT("Session Not Created!!!")));

            Interface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		}
	}
}

void UMultiplayerSessionSubsystem::FindGameSessions(int32 MaxSearchResults)
{
	if (!Interface.IsValid()) { return ;}

	FindSessionsCompleteDelegateHandle = Interface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	SessionSearch->MaxSearchResults = MaxSearchResults;
	SessionSearch->bIsLanQuery = false;
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	if (!Interface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef()))
    {
        Interface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
    }
}

void UMultiplayerSessionSubsystem::OnFindSessionComplete(bool bWasSuccessful)
{
	if (!Interface.IsValid()) { return ;}

	if (bWasSuccessful)
	{
		for (auto Results : SessionSearch->SearchResults)
		{
			FString MatchType;
			Results.Session.SessionSettings.Get(FName("MatchType"), MatchType);
			if (MatchType == FString("FreeForAll"))
			{
				JoinSessionCompleteDelegateHandle = Interface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
				
				LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
				if (!Interface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, Results))
                {
                    Interface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
                }
			}
            Interface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString(TEXT("Session Not Found !!!")));

            Interface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		}
	}
}

void UMultiplayerSessionSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!Interface.IsValid()) { return ;}

	FString Address;
	if (Interface->GetResolvedConnectString(NAME_GameSession, Address))
	{
		GetGameInstance()->GetFirstLocalPlayerController()->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
	}
    Interface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
}