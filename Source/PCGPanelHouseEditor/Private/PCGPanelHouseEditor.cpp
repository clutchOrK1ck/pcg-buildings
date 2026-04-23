#include "PCGPanelHouseEditor.h"

#define LOCTEXT_NAMESPACE "FPCGPanelHouseEditorModule"

void FPCGPanelHouseEditorModule::StartupModule()
{
    PBPanelLayoutAssetActions = MakeShared<FPBPanelLayoutAssetActions>();
    FAssetToolsModule::GetModule().Get().RegisterAssetTypeActions(PBPanelLayoutAssetActions.ToSharedRef());
}

void FPCGPanelHouseEditorModule::ShutdownModule()
{
    if (!FModuleManager::Get().IsModuleLoaded("AssetTools")) return;
    FAssetToolsModule::GetModule().Get().UnregisterAssetTypeActions(PBPanelLayoutAssetActions.ToSharedRef());
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FPCGPanelHouseEditorModule, PCGPanelHouseEditor)