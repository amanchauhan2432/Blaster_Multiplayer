#pragma once

UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
    ETIN_Left UMETA(DisplatName = "Turning Left"),
    ETIN_Right UMETA(DisplatName = "Turning Right"),
    ETIN_NotTurning UMETA(DisplatName = "Not Turning"),

    ETIN_MAX UMETA(DisplatName = "DefaultMAX")
};