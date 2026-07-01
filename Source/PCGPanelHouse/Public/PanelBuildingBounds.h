// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PanelBuildingBounds.generated.h"


/**
 * this component represents a bounding box with its bottom-front-left corner placed at the actor's origin
 *
 * it's mainly a visual fidelity and a helper for editing the panel building bounds
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PCGPANELHOUSE_API UPanelBuildingBounds : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPanelBuildingBounds();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Width;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Depth;

	UPROPERTY(EditAnywhere)
	float Height;

	UFUNCTION(BlueprintPure)
	FBox GetBounds() const;
	
	virtual bool IsEditorOnly() const override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};
