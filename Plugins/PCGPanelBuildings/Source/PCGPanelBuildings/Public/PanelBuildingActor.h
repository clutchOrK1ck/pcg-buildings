// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BoxBounds.h"
#include "PCGComponent.h"
#include "PBGrammar.h"
#include "GameFramework/Actor.h"
#include "PanelBuildingActor.generated.h"

const FName GRAPH_PARAM_NAME_MAX_HEIGHT = FName("MaximumHeight");
const FName GRAPH_PARAM_NAME_FLOOR_HEIGHT = FName("FloorHeight");

UCLASS()
class PCGPANELBUILDINGS_API APanelBuildingActor : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Interp, Category="Building Configuration")
	FString Grammar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building Configuration")
	TArray<UPBPanelLayout*> PanelConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building Configuration")
	float BasementHeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building Configuration")
	float AtticHeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Building Configuration")
	float WindowFrameDepth = 12.7;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Building Calculated Params")
	float FloorHeight;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Building Calculated Params")
	float BuildingHeight;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Building Calculated Params")
	int NumFloors;
	
	// Sets default values for this actor's properties
	APanelBuildingActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	
	UFUNCTION(CallInEditor, Category="Building Configuration", DisplayName="Validate Grammar")
	void ValidateGrammar();
	
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
	UBoxBounds* Boundaries;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
