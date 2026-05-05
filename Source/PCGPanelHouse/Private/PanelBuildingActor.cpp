// Fill out your copyright notice in the Description page of Project Settings.


#include "PanelBuildingActor.h"


// Sets default values
APanelBuildingActor::APanelBuildingActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PCG = CreateDefaultSubobject<UPCGComponent>("PCG");
}

// Called when the game starts or when spawned
void APanelBuildingActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APanelBuildingActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

