// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Blaster/Character/MainCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

}


// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

void UCombatComponent::EquipWeapon(AWeapon* Weapon)
{
	if (Weapon && MainCharacter)
	{
		EquippedWeapon = Weapon;
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

		MainCharacter->GetMesh()->GetSocketByName(FName("WeaponSocket"))->AttachActor(Weapon, MainCharacter->GetMesh());
		MainCharacter->bIsWeaponEquipped = true;

		EquippedWeapon->SetOwner(MainCharacter);

		MainCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		MainCharacter->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (MainCharacter && EquippedWeapon)
	{
		MainCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		MainCharacter->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);

	if (MainCharacter)
	{
		MainCharacter->GetCharacterMovement()->MaxWalkSpeed = bAiming ? 200.f : 600.f;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	
	if (MainCharacter)
	{
		MainCharacter->GetCharacterMovement()->MaxWalkSpeed = bAiming ? 200.f : 600.f;
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFirePressed = bPressed;
	if (bFirePressed)
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		ServerFire(HitResult.ImpactPoint);
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (MainCharacter && EquippedWeapon)
	{
		MainCharacter->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	FVector2D CrosshairLocation{ ViewportSize.X / 2.f, ViewportSize.Y / 2.f };

	FVector CrosshairWorldPostion;
	FVector CrosshairWorldDirection;
	bool bScreenToHit = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPostion, CrosshairWorldDirection);

	if (bScreenToHit)
	{
		FVector Start = CrosshairWorldPostion;
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;
		bool bHit = GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}
	}
}