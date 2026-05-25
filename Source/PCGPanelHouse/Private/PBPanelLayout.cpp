// Fill out your copyright notice in the Description page of Project Settings.


#include "PBPanelLayout.h"

#if WITH_EDITOR
FVector UPBPanelLayout::GetWindowOffset() const
{
	if (!HasWindow)
	{
		return FVector();
	}

	// assume the panel's center is at (0;0)
	auto PanelTopLeft = FVector2f(-Width/2., Height/2.);
	auto WindowTopLeftOffset = FVector2f(WindowPosition.X, -WindowPosition.Y);
	auto WindowCenterOffset = FVector2f(WindowWidth/2., -WindowHeight/2.);

	auto Offset = PanelTopLeft + WindowTopLeftOffset + WindowCenterOffset;

	return FVector{
		0.,
		Offset.X,
		Offset.Y
	};
}

void UPBPanelLayout::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeProperty(PropertyChangedEvent);

	// TODO need to broadcast changes to panel building actors using this panel layout?
}
#endif
