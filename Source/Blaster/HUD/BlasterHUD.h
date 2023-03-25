// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"


USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	class UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsBottom;
	UTexture2D* CrosshairsLeft;
	float CrosshairSpread;
	FLinearColor CrosshairsColor;
};


/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

protected:


public:

	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }

private:

	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor Color);

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

};
