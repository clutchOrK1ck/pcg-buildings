#include "PCGPanelHouseEditor.h"
#include "MessageLogModule.h"
#include "PanelBuildingBounds.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "FPanelBuildingBoundsVisualizer.h"

#define LOCTEXT_NAMESPACE "FPCGPanelHouseEditorModule"

void FPCGPanelHouseEditorModule::RegisterMessageLogCategory()
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

void FPCGPanelHouseEditorModule::UnregisterMessageLogCategory()
{
	if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
	{
		FModuleManager::GetModuleChecked<FMessageLogModule>("MessageLog").UnregisterLogListing("PCGPanelBuildings");
	}
}

void FPCGPanelHouseEditorModule::RegisterComponentVisualizers()
{
	// panel building bounds component
	if (GUnrealEd != nullptr)
	{
		GUnrealEd->RegisterComponentVisualizer(
			UPanelBuildingBounds::StaticClass()->GetFName(),
			MakeShareable(new FPanelBuildingBoundsVisualizer)
		);
	}
}

void FPCGPanelHouseEditorModule::UnregisterComponentVisualizers()
{
	if (GUnrealEd != nullptr)
	{
		GUnrealEd->UnregisterComponentVisualizer(UPanelBuildingBounds::StaticClass()->GetFName());
	}
}

void FPCGPanelHouseEditorModule::StartupModule()
{
	IAssetTools& AssetTools = FAssetToolsModule::GetModule().Get();

	AssetCategoryId = AssetTools.RegisterAdvancedAssetCategory(
		FName{"PCGPanelBuildings"},
		FText::FromString("Panel Buildings"));

	PBPanelLayoutAssetActions = MakeShared<FPBPanelLayoutAssetActions>(AssetCategoryId);
	AssetTools.RegisterAssetTypeActions(PBPanelLayoutAssetActions.ToSharedRef());

	RegisterMessageLogCategory();
	RegisterComponentVisualizers();
}

void FPCGPanelHouseEditorModule::ShutdownModule()
{
	if (!FModuleManager::Get().IsModuleLoaded("AssetTools")) return;
	FAssetToolsModule::GetModule().Get().UnregisterAssetTypeActions(PBPanelLayoutAssetActions.ToSharedRef());

	UnregisterMessageLogCategory();
	UnregisterComponentVisualizers();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPCGPanelHouseEditorModule, PCGPanelHouseEditor)
