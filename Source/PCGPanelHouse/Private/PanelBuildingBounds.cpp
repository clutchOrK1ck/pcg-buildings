// Fill out your copyright notice in the Description page of Project Settings.


#include "PanelBuildingBounds.h"


// Sets default values for this component's properties
UPanelBuildingBounds::UPanelBuildingBounds()
{
	PrimaryComponentTick.bCanEverTick = false;

	// these default values are an average-ish panel house, not to make this too huge in the level
	Width = 1500.f;
	Depth = 700.f;
	Height = 500.f;
}


FBox UPanelBuildingBounds::GetBounds() const
{
	return FBox{
		FVector{-Depth, 0.f, 0.f},
		FVector{0.f, Width, Height}
	};
}

bool UPanelBuildingBounds::IsEditorOnly() const
{
	return true;
}

// Called when the game starts
void UPanelBuildingBounds::BeginPlay()
{
	Super::BeginPlay();
}

