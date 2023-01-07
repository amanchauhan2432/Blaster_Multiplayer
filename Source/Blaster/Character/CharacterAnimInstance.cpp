// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"
#include "MainCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UCharacterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    
    MainCharacter = Cast<AMainCharacter>(TryGetPawnOwner());
}

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);

    if (!MainCharacter)
    {
        MainCharacter = Cast<AMainCharacter>(TryGetPawnOwner());
    }

    if (!MainCharacter) { return ;}

    Speed = MainCharacter->GetVelocity().Size();
    bIsInAir = MainCharacter->GetCharacterMovement()->IsFalling();
    bIsAccelarating = MainCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() ? true : false;
}
