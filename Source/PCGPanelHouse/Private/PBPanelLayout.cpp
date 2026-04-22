// Fill out your copyright notice in the Description page of Project Settings.


#include "PBPanelLayout.h"

#if WITH_EDITOR
void UPBPanelLayout::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeProperty(PropertyChangedEvent);

	// TODO need to broadcast changes to panel building actors using this panel layout?
}
#endif
