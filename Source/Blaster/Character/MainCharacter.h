// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "MainCharacter.generated.h"

UCLASS()
class BLASTER_API AMainCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMainCharacter();

	UPROPERTY(EditAnywhere, Category = "Camera")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(EditAnywhere, Category = "Camera")
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* Combat;

	UPROPERTY(Replicated)
	bool bIsWeaponEquipped;

	bool IsAiming();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void AimOffset(float DeltaTime);


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void Equip();
	void CrouchButtonPressed();

	void AimButtonPressed();
	void AimButtonReleased();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetOverlappingWeapon(class AWeapon* Weapon);

	virtual void PostInitializeComponents() override;

	FORCEINLINE float GetAOYaw() const { return AO_Yaw; }
	FORCEINLINE float GetAOPitch() const { return AO_Pitch; }

	class AWeapon* GetEquippedWeapon();

	FORCEINLINE ETurningInPlace GetTurningInPlace() { return TurningInPlace; }

	void PlayFireMontage(bool bAiming);

private:

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(class AWeapon* LastWeapon);

	UFUNCTION(Server, Reliable)
	void ServerEquip();

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartAimRotation;

	ETurningInPlace TurningInPlace;

	void FireButtonPressed();
	void FireButtonReleased();
		
	UPROPERTY(EditAnywhere)
	class UAnimMontage* FireWeaponMontage;
};