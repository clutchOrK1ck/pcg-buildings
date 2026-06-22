// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PanelBuildingBounds.h"
#include "PCGComponent.h"
#include "PBGrammar.h"
#include "GameFramework/Actor.h"
#include "PanelBuildingActor.generated.h"

const FName GRAPH_PARAM_NAME_MAX_HEIGHT = FName("MaximumHeight");
const FName GRAPH_PARAM_NAME_FLOOR_HEIGHT = FName("FloorHeight");

UCLASS()
class PCGPANELHOUSE_API APanelBuildingActor : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Panel Buildings")
	FString Grammar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Panel Buildings")
	TArray<UPBPanelLayout*> PanelConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Panel Buildings")
	float BasementHeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Panel Buildings")
	float RoofHeight;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Panel Buildings")
	float FloorHeight;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Panel Buildings")
	float BuildingHeight;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Panel Buildings")
	int NumFloors;
	
	// Sets default values for this actor's properties
	APanelBuildingActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	
	UFUNCTION(CallInEditor, Category="Panel Buildings")
	void Validate();
	
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UPCGComponent* PCG;

	UPROPERTY(BlueprintReadOnly, Category="Panel Buildings")
	TArray<FPositionedPanelInfo> Panels;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Panel Buildings")
	bool IsValidPanelBuildingConfig(FString& OutErrorMessage, FPBRuleSet& OutRuleSet) const;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UPanelBuildingBounds* Boundaries;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
