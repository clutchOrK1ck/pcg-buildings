// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PBPanelLayout.generated.h"

/**
 * panel layout specify spatial dimensions and properties of a panel building's panel
 *
 * specifically, it contains width/height of the panel, presence, size, and position of a window etc.
 */
UCLASS()
class PCGPANELHOUSE_API UPBPanelLayout : public UObject
{
	GENERATED_BODY()

public:
	// panel height in cm
	UPROPERTY(EditAnywhere, Category="Spatial")
	float Width;

	// panel width in cm
	UPROPERTY(EditAnywhere, Category="Spatial")
	float Height;

	UPROPERTY(EditAnywhere, Category="Spatial|Window")
	bool HasWindow;

	UPROPERTY(EditAnywhere, DisplayName="Width", Category="Spatial|Window")
	float WindowWidth;

	UPROPERTY(EditAnywhere, DisplayName="Height", Category="Spatial|Window")
	float WindowHeight;

	// to be decided if cm, percentage, and relative to center or corner
	UPROPERTY(EditAnywhere, DisplayName="Position", Category="Spatial|Window")
	FVector2f WindowPosition;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
