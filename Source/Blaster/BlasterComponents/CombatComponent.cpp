// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Blaster/Character/MainCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/PlayerController/MainPlayerController.h"
#include "Engine/Texture2D.h"
#include "Camera/CameraComponent.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "TimerManager.h"

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

	if (MainCharacter->FollowCamera)
	{
		DefaultFOV = MainCharacter->FollowCamera->FieldOfView;
		CurrentFOV = DefaultFOV;
	}
}



// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	if (MainCharacter && MainCharacter->IsLocallyControlled())
	{
		TraceUnderCrosshairs(HitResult);

		SetHUDCrosshairs(DeltaTime);

		InterpFOV(DeltaTime);
	}
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

void UCombatComponent::Fire()
{
	TraceUnderCrosshairs(HitResult);
	ServerFire(HitResult.ImpactPoint);

	if (EquippedWeapon)
	{
		CrosshairShootingFactor = 1.f;
	}
	StartFireTimer();
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFirePressed = bPressed;
	if (bFirePressed)
	{
		Fire();
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::StartFireTimer()
{
	if (MainCharacter && EquippedWeapon)
	{
		MainCharacter->GetWorldTimerManager().SetTimer(FireTimer, this, &UCombatComponent::FireTimerFinished, EquippedWeapon->FireDelay);
	}
}

void UCombatComponent::FireTimerFinished()
{
	if (bFirePressed && EquippedWeapon && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
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

		if (MainCharacter)
		{
			float DistanceToCharacter = (MainCharacter->GetActorLocation() - Start).Size();
			Start = Start + CrosshairWorldDirection * DistanceToCharacter;
		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;
		bool bHit = GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}

		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairsColor = FColor::Red;
			CrosshairAimCharFactor = FMath::FInterpTo(CrosshairAimCharFactor, 0.3, GetWorld()->DeltaRealTimeSeconds, 5.f);
		}
		else
		{
			HUDPackage.CrosshairsColor = FColor::White;
			CrosshairAimCharFactor = FMath::FInterpTo(CrosshairAimCharFactor, 0.f, GetWorld()->DeltaRealTimeSeconds, 5.f);
		}
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (!MainCharacter || !MainCharacter->Controller) { return; }

	Controller = Controller == nullptr ? Cast<AMainPlayerController>(MainCharacter->Controller) : Controller;

	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;

		if (HUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
			}

			// Calculate spread for crosshairs
			FVector2D InputRange{ 0.f, MainCharacter->GetCharacterMovement()->MaxWalkSpeed };
			FVector2D OutputRange{ 0.f, 1.f };

			FVector Velocity = MainCharacter->GetVelocity();
			Velocity.Z = 0;
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(InputRange, OutputRange, Velocity.Size() * 2.f);

			if (MainCharacter->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 1.f, DeltaTime, 5.f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 10.f);
			}
			if (bAiming)
			{
				CrosshairAimingFactor = FMath::FInterpTo(CrosshairAimingFactor, 0.5f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimingFactor = FMath::FInterpTo(CrosshairAimingFactor, 0.f, DeltaTime, 30.f);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 5.f);

			HUDPackage.CrosshairSpread = 0.7f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimingFactor + CrosshairShootingFactor - CrosshairAimCharFactor;
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (MainCharacter && MainCharacter->FollowCamera && EquippedWeapon)
	{
		if (bAiming)
		{
			CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetInterpSpeed());
		}
		else
		{
			CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, EquippedWeapon->GetInterpSpeed());
		}
		MainCharacter->FollowCamera->SetFieldOfView(CurrentFOV);
	}
}