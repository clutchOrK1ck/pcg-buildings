#include "FPBPanelLayoutAssetActions.h"

#include "PBPanelLayout.h"

FColor FPBPanelLayoutAssetActions::GetTypeColor() const
{
	return FColor::Emerald;
}

uint32 FPBPanelLayoutAssetActions::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

FText FPBPanelLayoutAssetActions::GetName() const
{
	return FText::FromString("Panel layout");
}

UClass* FPBPanelLayoutAssetActions::GetSupportedClass() const
{
	return UPBPanelLayout::StaticClass();
}
