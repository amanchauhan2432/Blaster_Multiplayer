// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"
#include "MainCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/Weapon/Weapon.h"

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
    EquippedWeapon = MainCharacter->GetEquippedWeapon();
    bIsCrouched = MainCharacter->bIsCrouched;
    bIsAiming = MainCharacter->IsAiming();

    TurningInPlace = MainCharacter->GetTurningInPlace();

    bRotateRootBone = MainCharacter->ShouldRotateRootBone();
    
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

    AO_Yaw = MainCharacter->GetAOYaw();
    AO_Pitch = MainCharacter->GetAOPitch();

    if (bIsWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && MainCharacter)
    {
        LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket", ERelativeTransformSpace::RTS_World));
        FVector OutPosition;
        FRotator OutRotation;
        MainCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
        LeftHandTransform.SetLocation(OutPosition);
        LeftHandTransform.SetRotation(FQuat(OutRotation));
        
        if (MainCharacter->IsLocallyControlled())
        {
            bIsLocallyControlled = true;
            FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R", ERelativeTransformSpace::RTS_World));
            FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - MainCharacter->GetHitTarget()));
            RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
        }
    }
}
