// Fill out your copyright notice in the Description page of Project Settings.


#include "PBPanelLayoutFactory.h"
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

FText UPBPanelLayoutFactory::GetToolTip() const
{
	return FText::FromString("Create a panel layout");
}
