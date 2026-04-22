// Fill out your copyright notice in the Description page of Project Settings.


#include "PBPanelLayoutFactory.h"

#include "AssetTypeCategories.h"
#include "PBPanelLayout.h"

UPBPanelLayoutFactory::UPBPanelLayoutFactory()
{
	this->bCreateNew = true;
	this->bEditAfterNew = false;
	this->bEditorImport = false;
	this->SupportedClass = UPBPanelLayout::StaticClass();
}

UObject* UPBPanelLayoutFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UPBPanelLayout>(InParent, InClass, InName, Flags | RF_Transactional);
}

FText UPBPanelLayoutFactory::GetDisplayName() const
{
	return FText::FromString("PB panel layout factory");
}

uint32 UPBPanelLayoutFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Misc;
}

FText UPBPanelLayoutFactory::GetToolTip() const
{
	return FText::FromString("A factory creating an panel building's panel layout asset");
}
