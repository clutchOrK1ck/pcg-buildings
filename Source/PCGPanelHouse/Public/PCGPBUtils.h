// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PanelBuildingActor.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PCGPBUtils.generated.h"

/**
 * 
 */
UCLASS()
class PCGPANELHOUSE_API UPCGPBUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	static bool DoesRayHitAABB(const FVector& Origin, const FVector& Direction, const FVector& AABBMin, const FVector& AABBMax);

	UFUNCTION(BlueprintPure)
	static bool IsSatelliteDishOccluded(const FVector& WorldLocation, const FVector& SatelliteDirection, APanelBuildingActor* Building);
};
