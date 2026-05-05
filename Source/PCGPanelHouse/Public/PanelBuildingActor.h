// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCGComponent.h"
#include "GameFramework/Actor.h"
#include "PanelBuildingActor.generated.h"

UCLASS()
class PCGPANELHOUSE_API APanelBuildingActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APanelBuildingActor();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UPCGComponent* PCG;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
