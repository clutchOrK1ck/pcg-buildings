#include "FPBPanelLayoutAssetActions.h"

#include "PBPanelLayout.h"

FColor FPBPanelLayoutAssetActions::GetTypeColor() const
{
	return FColor::Emerald;
}

uint32 FPBPanelLayoutAssetActions::GetCategories()
{
	return CategoryId;
}

FText FPBPanelLayoutAssetActions::GetName() const
{
	return FText::FromString("Panel Layout");
}

UClass* FPBPanelLayoutAssetActions::GetSupportedClass() const
{
	return UPBPanelLayout::StaticClass();
}
