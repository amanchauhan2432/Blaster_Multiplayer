// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Weapon/Weapon.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/Blaster.h"

// Sets default values
AMainCharacter::AMainCharacter()
{
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	GetCharacterMovement()->RotationRate = FRotator(0.f, 850.f, 0.f);

	TurningInPlace = ETurningInPlace::ETIN_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}

	HideCameraIfCharacterClose();
}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AMainCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AMainCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis(TEXT("LookRight"), this, &APawn::AddControllerYawInput);

	PlayerInputComponent->BindAction(FName("Jump"), IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAction(FName("Equip"), IE_Pressed, this, &AMainCharacter::Equip);

	PlayerInputComponent->BindAction(FName("Crouch"), IE_Pressed, this, &AMainCharacter::CrouchButtonPressed);

	PlayerInputComponent->BindAction(FName("Aim"), IE_Pressed, this, &AMainCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction(FName("Aim"), IE_Released, this, &AMainCharacter::AimButtonReleased);

	PlayerInputComponent->BindAction(FName("Fire"), IE_Pressed, this, &AMainCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction(FName("Fire"), IE_Released, this, &AMainCharacter::FireButtonReleased);

}

void AMainCharacter::MoveForward(float AxisValue)
{
	if (Controller && AxisValue != 0)
	{
		const FRotator Rotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);

		AddMovementInput(Direction, AxisValue);
	}
}

void AMainCharacter::MoveRight(float AxisValue)
{
	if (Controller && AxisValue != 0)
	{
		const FRotator Rotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Direction, AxisValue);
	}
}

float AMainCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void AMainCharacter::AimOffset(float DeltaTime)
{

	if (Combat && Combat->EquippedWeapon == nullptr) return;

	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();
	if (Speed == 0.f && !bIsInAir)
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartAimRotation);
		AO_Yaw = DeltaRotation.Yaw;

		if (TurningInPlace == ETurningInPlace::ETIN_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}

		if (AO_Yaw > 90.f)
		{
			TurningInPlace = ETurningInPlace::ETIN_Right;
		}
		if (AO_Yaw < -90.f)
		{
			TurningInPlace = ETurningInPlace::ETIN_Left;
		}
		if (TurningInPlace != ETurningInPlace::ETIN_NotTurning)
		{
			InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 3.f);
			AO_Yaw = InterpAO_Yaw;
			if (FMath::Abs(AO_Yaw) < 15.f)
			{
				TurningInPlace = ETurningInPlace::ETIN_NotTurning;
				StartAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
			}
		}
	}
	if (Speed > 0 || bIsInAir)
	{
		bRotateRootBone = false;
		StartAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;

		TurningInPlace = ETurningInPlace::ETIN_NotTurning;
	}

	CalculateAO_Pitch();
}

void AMainCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// Map pitch from [270, 360) to [-90,0)
		FVector2d InRange(270.f,360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void AMainCharacter::SimProxiesTurn()
{
	if (Combat == nullptr && Combat->EquippedWeapon == nullptr) return;

	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIN_NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnThreshhold)
	{
		if (ProxyYaw > TurnThreshhold)
		{
			TurningInPlace = ETurningInPlace::ETIN_Right;
		}
		else if (ProxyYaw < -TurnThreshhold)
		{
			TurningInPlace = ETurningInPlace::ETIN_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIN_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIN_NotTurning;
}

void AMainCharacter::SetOverlappingWeapon(class AWeapon* Weapon)
{
	if (IsLocallyControlled() && OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;

	if (IsLocallyControlled() && OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
}

void AMainCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AMainCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AMainCharacter, bIsWeaponEquipped);
}

void AMainCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void AMainCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->MainCharacter = this;
	}
}

void AMainCharacter::Equip()
{
	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquip();
		}
	}
}

void AMainCharacter::ServerEquip_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void AMainCharacter::CrouchButtonPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AMainCharacter::AimButtonPressed()
{
	if (bIsWeaponEquipped && Combat)
	{
		Combat->SetAiming(true);
	}
}

void AMainCharacter::AimButtonReleased()
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

bool AMainCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* AMainCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;

	return Combat->EquippedWeapon;
}

void AMainCharacter::FireButtonPressed()
{
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void AMainCharacter::FireButtonReleased()
{
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void AMainCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return;

	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < 200.f)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void AMainCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName = bAiming ? FName("Rifle_Aim") : FName("Rifle_Hip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMainCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName = FName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMainCharacter::MulticastHit_Implementation()
{
	PlayHitReactMontage();
}

FVector AMainCharacter::GetHitTarget()
{
	if (Combat == nullptr) return FVector();

	return Combat->HitResult.ImpactPoint;
}

void AMainCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}
