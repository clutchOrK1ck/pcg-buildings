#include "PCGPanelHouseEditor.h"

#define LOCTEXT_NAMESPACE "FPCGPanelHouseEditorModule"

void FPCGPanelHouseEditorModule::StartupModule()
{
	IAssetTools& AssetTools = FAssetToolsModule::GetModule().Get();

	AssetCategoryId = AssetTools.RegisterAdvancedAssetCategory(
		FName{"PCGPanelBuildings"},
		FText::FromString("Panel Buildings"));

	PBPanelLayoutAssetActions = MakeShared<FPBPanelLayoutAssetActions>(AssetCategoryId);
	AssetTools.RegisterAssetTypeActions(PBPanelLayoutAssetActions.ToSharedRef());
}

void FPCGPanelHouseEditorModule::ShutdownModule()
{
	if (!FModuleManager::Get().IsModuleLoaded("AssetTools")) return;
	FAssetToolsModule::GetModule().Get().UnregisterAssetTypeActions(PBPanelLayoutAssetActions.ToSharedRef());
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPCGPanelHouseEditorModule, PCGPanelHouseEditor)
