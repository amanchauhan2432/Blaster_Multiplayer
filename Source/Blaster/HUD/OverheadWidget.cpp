// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"

void UOverheadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
    Super::OnLevelRemovedFromWorld(InLevel, InWorld);

    RemoveFromParent();
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
    ENetRole LocalRole = InPawn->GetLocalRole();
    FString Role;

    switch (LocalRole)
    {
    case ENetRole::ROLE_Authority:
        Role = FString("Local Role : Authority");
        break;
    case ENetRole::ROLE_AutonomousProxy:
        Role = FString("Local Role : Autonomous Proxy");
        break;
    case ENetRole::ROLE_SimulatedProxy:
        Role = FString("Local Role : Simulated Proxy");
        break;
    case ENetRole::ROLE_None:
        Role = FString("None");
        break;
    }
    if (DisplayText)
    {
        DisplayText->SetText(FText::FromString(Role));
    }
}
