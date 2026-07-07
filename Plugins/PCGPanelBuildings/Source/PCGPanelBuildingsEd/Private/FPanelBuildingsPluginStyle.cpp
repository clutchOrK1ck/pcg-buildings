#include "FPanelBuildingsPluginStyle.h"

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/SlateStyleRegistry.h"

void FPanelBuildingsPluginStyle::Register()
{
	FSlateStyleRegistry::RegisterSlateStyle(Get());
}

void FPanelBuildingsPluginStyle::Unregister()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(Get());
}

FPanelBuildingsPluginStyle& FPanelBuildingsPluginStyle::Get()
{
	static FPanelBuildingsPluginStyle Style;
	return Style;
}

FPanelBuildingsPluginStyle::FPanelBuildingsPluginStyle() : FSlateStyleSet("PanelBuildingsStyleSet")
{
	// sizes
	auto Icon16x16 = FVector2D(16., 16.);
	auto Icon64x64 = FVector2D(64., 64.);
	
	SetContentRoot(IPluginManager::Get().FindPlugin("PCGPanelBuildings")->GetBaseDir() / TEXT("Content"));

	// panel asset icon and brush
	Set("ClassIcon.PBPanelLayout", new IMAGE_BRUSH("Icons/PanelLayoutSm", Icon16x16));
	Set("ClassThumbnail.PBPanelLayout", new IMAGE_BRUSH("Icons/PanelLayout", Icon64x64));
}
