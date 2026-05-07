#pragma once

#include "CoreMinimal.h"
#include "FPBPanelLayoutAssetActions.h"
#include "Modules/ModuleManager.h"

class FPCGPanelHouseEditorModule : public IModuleInterface
{
    TSharedPtr<FPBPanelLayoutAssetActions> PBPanelLayoutAssetActions;
	EAssetTypeCategories::Type AssetCategoryId {EAssetTypeCategories::Misc};

	void RegisterMessageLogCategory();
	void UnregisterMessageLogCategory();
	void RegisterComponentVisualizers();
	void UnregisterComponentVisualizers();
    
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
