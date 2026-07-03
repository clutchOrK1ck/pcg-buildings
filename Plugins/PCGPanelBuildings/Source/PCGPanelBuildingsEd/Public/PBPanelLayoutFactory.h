// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "PBPanelLayoutFactory.generated.h"

/**
 * 
 */
UCLASS()
class UPBPanelLayoutFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPBPanelLayoutFactory();
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	                                  UObject* Context, FFeedbackContext* Warn) override;
	virtual FText GetToolTip() const override;
};
