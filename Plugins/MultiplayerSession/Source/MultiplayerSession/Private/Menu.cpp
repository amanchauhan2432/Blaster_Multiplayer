// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionSubsystem.h"

void UMenu::MenuSetup(int32 NumPublicConnections, FString MatchType, FString LobbyPath)
{
    MultiplayerSessionSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSessionSubsystem>();

    NumofPublicConnections = NumPublicConnections;
    MatchofType = MatchType;
    MultiplayerSessionSubsystem->LobbyPath = FString::Printf(TEXT("%s?listen"), *LobbyPath);

	AddToViewport();

    FInputModeUIOnly InputData;
    GetWorld()->GetFirstPlayerController()->SetInputMode(InputData);
    GetWorld()->GetFirstPlayerController()->SetShowMouseCursor(true);
}

bool UMenu::Initialize()
{
	if (! Super::Initialize())
    {
        return false;
    }

    if (HostBtn && JoinBtn)
    {
        HostBtn->OnClicked.AddDynamic(this, &ThisClass::HostBtnClicked);
        JoinBtn->OnClicked.AddDynamic(this, &ThisClass::JoinBtnClicked);
    }
    return true;
}

void UMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);

    RemoveFromParent();

    FInputModeGameOnly InputData;
    GetWorld()->GetFirstPlayerController()->SetInputMode(InputData);
    GetWorld()->GetFirstPlayerController()->SetShowMouseCursor(false);
}

void UMenu::HostBtnClicked()
{
	if (MultiplayerSessionSubsystem)
    {
        MultiplayerSessionSubsystem->CreateGameSession(NumofPublicConnections, MatchofType);
    }
}

void UMenu::JoinBtnClicked()
{
	if (MultiplayerSessionSubsystem)
    {
        MultiplayerSessionSubsystem->FindGameSessions(10000);
    }
}