#include "PCGPanelBuildingsEd.h"

#include "FPanelBuildingsPluginStyle.h"
#include "MessageLogModule.h"

#define LOCTEXT_NAMESPACE "FPCGPanelBuildingsEdModule"

void FPCGPanelBuildingsEdModule::RegisterMessageLogCategory()
{
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");

	FMessageLogInitializationOptions MessageLogOptions;

	MessageLogOptions.bAllowClear = true;
	MessageLogOptions.bShowFilters = true;
	MessageLogOptions.bScrollToBottom = true;
	MessageLogOptions.bDiscardDuplicates = true;

	MessageLogModule.RegisterLogListing(
		"PCGPanelBuildings",
		INVTEXT("PCG Panel Buildings"),
		MessageLogOptions
	);
}

void FPCGPanelBuildingsEdModule::UnregisterMessageLogCategory()
{
	if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
	{
		FModuleManager::GetModuleChecked<FMessageLogModule>("MessageLog").UnregisterLogListing("PCGPanelBuildings");
	}
}

void FPCGPanelBuildingsEdModule::StartupModule()
{
	IAssetTools& AssetTools = FAssetToolsModule::GetModule().Get();

	AssetCategoryId = AssetTools.RegisterAdvancedAssetCategory(
		FName{"PCGPanelBuildings"},
		FText::FromString("Panel Buildings"));

	PBPanelLayoutAssetActions = MakeShared<FPBPanelLayoutAssetActions>(AssetCategoryId);
	AssetTools.RegisterAssetTypeActions(PBPanelLayoutAssetActions.ToSharedRef());

	RegisterMessageLogCategory();
	FPanelBuildingsPluginStyle::Register();
}

void FPCGPanelBuildingsEdModule::ShutdownModule()
{
	if (!FModuleManager::Get().IsModuleLoaded("AssetTools")) return;
	FAssetToolsModule::GetModule().Get().UnregisterAssetTypeActions(PBPanelLayoutAssetActions.ToSharedRef());

	UnregisterMessageLogCategory();
	FPanelBuildingsPluginStyle::Unregister();
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FPCGPanelBuildingsEdModule, PCGPanelBuildingsEd)