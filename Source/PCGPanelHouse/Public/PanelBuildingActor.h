// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCGComponent.h"
#include "PBGrammar.h"
#include "GameFramework/Actor.h"
#include "PanelBuildingActor.generated.h"

UCLASS()
class PCGPANELHOUSE_API APanelBuildingActor : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Panel Buildings")
	FString Grammar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Panel Buildings")
	TArray<UPBPanelLayout*> PanelConfig;
	
	// Sets default values for this actor's properties
	APanelBuildingActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	
	UFUNCTION(CallInEditor, Category="Panel Buildings")
	void Validate();
	
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UPCGComponent* PCG;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Panel Buildings")
	bool IsValidPanelBuildingConfig(FString& OutErrorMessage) const;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
