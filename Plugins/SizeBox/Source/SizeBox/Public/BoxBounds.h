// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BoxBounds.generated.h"


/**
 * this component represents a bounding box with its bottom-front-left corner placed at the actor's origin
 *
 * it's mainly a visual fidelity and a helper for editing the panel building bounds
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SIZEBOX_API UBoxBounds : public UPrimitiveComponent
{
	GENERATED_BODY()

	void PostSizeUpdate();
	
public:
	// Sets default values for this component's properties
	UBoxBounds();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Width;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Depth;

	UPROPERTY(EditAnywhere)
	float Height;

	UPROPERTY(EditAnywhere)
	bool ReconstructOwningActorOnChange;
	
	UFUNCTION(BlueprintPure)
	FBox GetBounds() const;

	void SetWidthDepthHeight(float InWidth, float InDepth, float InHeight);

	void Expand(int Axis, float Value);
	
	virtual bool IsEditorOnly() const override;

	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};
