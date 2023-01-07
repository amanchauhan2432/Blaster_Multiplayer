// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Blaster/Character/MainCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Engine/SkeletalMeshSocket.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

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

void UCombatComponent::EquipWeapon(AWeapon* Weapon)
{
	if (Weapon && MainCharacter)
	{
		Weapon->SetWeaponState(EWeaponState::EWS_Equipped);

		MainCharacter->GetMesh()->GetSocketByName(FName("WeaponSocket"))->AttachActor(Weapon, MainCharacter->GetMesh());

		Weapon->SetOwner(MainCharacter);
		Weapon->ShowPickupWidget(false);
	}
}