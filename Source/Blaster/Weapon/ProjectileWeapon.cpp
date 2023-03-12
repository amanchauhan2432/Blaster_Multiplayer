// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);

    if (!HasAuthority()) return;

    APawn* InstigatorPawn = Cast<APawn>(GetOwner());

    FTransform MuzzleSocketTransform = GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"));
    if (ProjectileClass && InstigatorPawn)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = GetOwner();
        SpawnParams.Instigator = InstigatorPawn;

        FVector ToTarget = HitTarget - MuzzleSocketTransform.GetLocation();
        GetWorld()->SpawnActor<AProjectile>(ProjectileClass, MuzzleSocketTransform.GetLocation(), ToTarget.Rotation(), SpawnParams);
    }
}