// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	friend class AMainCharacter;

	// Sets default values for this component's properties
	UCombatComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void Fire();

	void FireButtonPressed(bool bPressed);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

private:
	// Variables
	UPROPERTY(Replicated)
	bool bAiming;

	// Functions
	class AMainCharacter* MainCharacter;
	void EquipWeapon(class AWeapon* Weapon);
	class AMainPlayerController* Controller;
	class ABlasterHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	bool bFirePressed;

	FHitResult HitResult;

	// HUD and Crosshairs

	FHUDPackage HUDPackage;

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimingFactor;
	float CrosshairAimCharFactor;
	float CrosshairShootingFactor;

	// Aiming and FOV

	float DefaultFOV;
	float CurrentFOV;

	void InterpFOV(float DeltaTime);

	// Automatic Fire

	FTimerHandle FireTimer;
	void StartFireTimer();
	void FireTimerFinished();
};
