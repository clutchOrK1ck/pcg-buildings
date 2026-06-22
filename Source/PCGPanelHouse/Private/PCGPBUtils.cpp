// Fill out your copyright notice in the Description page of Project Settings.


#include "PCGPBUtils.h"

bool UPCGPBUtils::DoesRayHitAABB(const FVector& Origin, const FVector& Direction, const FVector& AABBMin, const FVector& AABBMax)
{
	float TMin = 0.;
	float TMax = FLT_MAX;

	for (int Axis = 0; Axis < 3; Axis++)
	{
		if (Direction[Axis] != 0.)
		{
			auto T1 = (AABBMin[Axis] - Origin[Axis]) / Direction[Axis];
			auto T2 = (AABBMax[Axis] - Origin[Axis]) / Direction[Axis];

			if (T1 > T2)
			{
				Swap(T1, T2);
			}

			TMin = FMath::Max(TMin, T1);
			TMax = FMath::Min(TMax, T2);
		} else
		{
			if (!FDoubleInterval(AABBMin[Axis], AABBMax[Axis]).Contains(Origin[Axis]))
			{
				return false;
			}
		}
	}
	
	return TMin <= TMax;
}

bool UPCGPBUtils::IsSatelliteDishOccluded(const FVector& WorldLocation, const FVector& SatelliteDirection, APanelBuildingActor* Building)
{
	// satellite dish location and satellite direction in building coordinates
	auto LocalPos = Building->GetActorTransform().InverseTransformPosition(WorldLocation);
	auto SatelliteLocalDirection = Building->GetActorTransform().InverseTransformVector(SatelliteDirection);

	auto BBoxMin = FVector(
		-Building->Boundaries->Depth,
		0., 0.
	);

	auto BBoxMax = FVector(
		0.,
		Building->Boundaries->Width,
		Building->Boundaries->Height
	);

	return DoesRayHitAABB(LocalPos, SatelliteLocalDirection, BBoxMin, BBoxMax);
}
