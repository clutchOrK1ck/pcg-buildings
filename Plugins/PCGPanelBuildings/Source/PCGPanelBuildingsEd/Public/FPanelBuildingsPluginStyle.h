#pragma once

class FPanelBuildingsPluginStyle : FSlateStyleSet
{
public:
	static void Register();
	static void Unregister();

	static FPanelBuildingsPluginStyle& Get();

private:
	FPanelBuildingsPluginStyle();
};
