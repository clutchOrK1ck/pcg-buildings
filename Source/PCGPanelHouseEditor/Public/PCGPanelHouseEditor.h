#pragma once

#include "CoreMinimal.h"
#include "FPBPanelLayoutAssetActions.h"
#include "Modules/ModuleManager.h"

class FPCGPanelHouseEditorModule : public IModuleInterface
{
    TSharedPtr<FPBPanelLayoutAssetActions> PBPanelLayoutAssetActions;
    
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
