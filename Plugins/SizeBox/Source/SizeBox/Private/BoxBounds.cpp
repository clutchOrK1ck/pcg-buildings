// Fill out your copyright notice in the Description page of Project Settings.


#include "BoxBounds.h"


void UBoxBounds::PostSizeUpdate()
{
	if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject) || IsTemplate())
	{
		return;
	}
	
	UpdateBounds();
	
	if (ReconstructOwningActorOnChange)
	{
		if (auto* Owner = GetOwner(); Owner && !Owner->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject) && !Owner->IsTemplate())
		{
			Owner->RerunConstructionScripts();
		}
	}
}

// Sets default values for this component's properties
UBoxBounds::UBoxBounds()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	ReconstructOwningActorOnChange = false;
	SetWidthDepthHeight(1500., 700., 500.);
	ReconstructOwningActorOnChange = true;
}


FBox UBoxBounds::GetBounds() const
{
	return FBox{
		FVector{-Depth, 0.f, 0.f},
		FVector{0.f, Width, Height}
	};
}

void UBoxBounds::SetWidthDepthHeight(float InWidth, float InDepth, float InHeight)
{
	this->Width = InWidth;
	this->Depth = InDepth;
	this->Height = InHeight;

	PostSizeUpdate();
}

void UBoxBounds::Expand(int Axis, float Value)
{
	switch (Axis)
	{
	case 0:
		Width += Value;
		break;
	case 1:
		Depth += Value;
		break;
	case 2:
		Height += Value;
		break;
	default:
		break;
	}

	PostSizeUpdate();
}

bool UBoxBounds::IsEditorOnly() const
{
	return true;
}

FBoxSphereBounds UBoxBounds::CalcBounds(const FTransform& LocalToWorld) const
{
	FBoxSphereBounds NewBounds;

	NewBounds.Origin = LocalToWorld.GetLocation();
	NewBounds.BoxExtent = FVector{
		Depth/2.,
		Width/2.,
		Height/2.
	};
	NewBounds.SphereRadius = FMath::Max(FMath::Max(Depth/2., Width/2.), Height/2.);

	UE_LOG(LogTemp, Display, TEXT("Calculated component bounds: %f %f %f"), Depth/2., Width/2., Height/2.);
	return NewBounds;
}

// Called when the game starts
void UBoxBounds::BeginPlay()
{
	Super::BeginPlay();
}

void UBoxBounds::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	PostSizeUpdate();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

