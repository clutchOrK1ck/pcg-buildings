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
UCLASS(BlueprintType)
class PCGPANELHOUSE_API UPBPanelLayout : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Meta")
	FString Id;
	
	// panel height in cm
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spatial")
	float Width;

	// panel width in cm
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spatial")
	float Height;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spatial|Window")
	bool HasWindow;

	// window width in cm
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Width", Category="Spatial|Window")
	float WindowWidth;

	// window height in cm
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Height", Category="Spatial|Window")
	float WindowHeight;

	/** window's position is its location as percentage of panels' width and height
	 *
	 * for example, (0.5, 0.5) would mean in the center of the panel
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName="Position", Category="Spatial|Window")
	FVector2f WindowPosition;

	/**
	 * 
	 * @return offset at which the window's center lies relative to the panel's center
	 */
	UFUNCTION(BlueprintPure)
	FVector GetWindowOffset() const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
