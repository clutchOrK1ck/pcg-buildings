#pragma once
#include "AssetTypeActions_Base.h"

class FPBPanelLayoutAssetActions : public FAssetTypeActions_Base
{
public:
	virtual FColor GetTypeColor() const override;
	virtual uint32 GetCategories() override;
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
};
