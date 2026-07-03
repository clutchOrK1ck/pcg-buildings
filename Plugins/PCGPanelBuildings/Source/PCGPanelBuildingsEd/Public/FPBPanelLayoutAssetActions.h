#pragma once
#include "AssetTypeActions_Base.h"

class FPBPanelLayoutAssetActions : public FAssetTypeActions_Base
{
	uint32 CategoryId; // the EAssetTypeCategories::Type int to assign these actions to a category id
public:
	FPBPanelLayoutAssetActions(const uint32 CategoryId) : CategoryId(CategoryId) {}
	virtual FColor GetTypeColor() const override;
	virtual uint32 GetCategories() override;
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
};
