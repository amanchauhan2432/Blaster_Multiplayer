// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Menu.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSION_API UMenu : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	class UButton* HostBtn;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinBtn;

	class UMultiplayerSessionSubsystem* MultiplayerSessionSubsystem;

	int32 NumofPublicConnections;
	FString MatchofType;

protected:

	virtual bool Initialize() override;
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld);

public:

	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumPublicConnections = 4, FString MatchType = FString("FreeForAll"), FString LobbyPath = FString("/Game/Maps/Lobby"));

	UFUNCTION()
	void HostBtnClicked();

	UFUNCTION()
	void JoinBtnClicked();
};