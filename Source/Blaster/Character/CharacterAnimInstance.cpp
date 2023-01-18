// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"
#include "MainCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

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

    bIsWeaponEquipped = MainCharacter->bIsWeaponEquipped;
    bIsCrouched = MainCharacter->bIsCrouched;
    bIsAiming = MainCharacter->IsAiming();
    
    // Strafing
    FRotator AimRotation = MainCharacter->GetBaseAimRotation();
    FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(MainCharacter->GetVelocity());

    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
    DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
    YawOffset = DeltaRotation.Yaw;

    //Lean
    CharacterRotationLastFrame = CharacterRotation;
    CharacterRotation = MainCharacter->GetActorRotation();
    const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
    const float Target = Delta.Yaw / DeltaTime;
    const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
    Lean = FMath::Clamp(Interp, -90.f, 90.f);
}
