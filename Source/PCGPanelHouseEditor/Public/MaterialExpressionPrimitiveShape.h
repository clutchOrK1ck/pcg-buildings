// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialExpression.h"
#include "MaterialExpressionPrimitiveShape.generated.h"

UENUM()
enum class EPrimitiveShapeType {
	Rectangle,
	Disc,
};

FString ToString(const EPrimitiveShapeType Shape);

struct FShapeSettings
{
	FVector2D Center;
	float Sharpness;
	bool bInvert;
};

template<typename T>
class FShapeFactory
{
	FShapeSettings Settings;
	
public:
	virtual ~FShapeFactory() = default;
	explicit FShapeFactory(const FShapeSettings& Settings) :
		Settings(Settings) {}
	
	virtual T Create() = 0;

	const FShapeSettings& GetSettings() const { return Settings; }
};

class FLegacyShapeFactory : public FShapeFactory<int32>
{
	int32 UV; // input UVs
	
protected:
	// legacy shape factories use the material compilers (not HLSL)
	FMaterialCompiler* Compiler;
	
	int32 GetUVsShapeSpace() const;
	int32 ApplySharpness(int32 InputIndex) const;
	
	virtual int32 ComputeDistances() = 0;
	virtual int32 NormalizeDistances(int32 Distances) = 0;

public:
	FLegacyShapeFactory(FMaterialCompiler* Compiler, const int32 UV, const FShapeSettings& Settings) :
		FShapeFactory(Settings), Compiler(Compiler), UV(UV)
	{
	}
	
	virtual int32 Create() override;
};

class FLegacyRectFactory : public FLegacyShapeFactory
{
	FVector2D Extent;
	
protected:
	virtual int32 ComputeDistances() override;
	virtual int32 NormalizeDistances(int32 Distances) override;
	
public:
	FLegacyRectFactory(FMaterialCompiler* Compiler, const int32 UV, const FShapeSettings& Settings, const FVector2D& Extent) :
		FLegacyShapeFactory(Compiler, UV, Settings), Extent(Extent) {}
};

class FLegacyDiscFactory : public FLegacyShapeFactory
{
	float Radius;

protected:
	virtual int32 ComputeDistances() override;
	virtual int32 NormalizeDistances(int32 Distances) override;

public:
	FLegacyDiscFactory(FMaterialCompiler* Compiler, const int32 UV, const FShapeSettings& Settings, const float Radius) :
		FLegacyShapeFactory(Compiler, UV, Settings), Radius(Radius) {}
};

/**
 * 
 */
UCLASS()
class PCGPANELHOUSEEDITOR_API UMaterialExpressionPrimitiveShape : public UMaterialExpression
{
	GENERATED_BODY()

	bool GenerateHLSLExpressionRect(FMaterialHLSLGenerator& Generator, UE::HLSLTree::FScope& Scope, int32 OutputIdx, UE::HLSLTree::FExpression const*& Expression) const;
	
public:
	
	UPROPERTY(meta = (RequiredInput = "false", ToolTip = "Manipulate UVs to create primitive 2D shapes"))
	FExpressionInput UV;

	UPROPERTY(EditAnywhere, Category=MaterialExpressionPrimitiveShape)
	EPrimitiveShapeType Shape = EPrimitiveShapeType::Rectangle;

	UPROPERTY(EditAnywhere, Category=MaterialExpressionPrimitiveShape)
	FVector2D Center {0.5, 0.5};

	UPROPERTY(EditAnywhere, Category=MaterialExpressionPrimitiveShape, meta=(ClampMin=0., ClampMax=3., UIMin=0., UIMax=3.))
	float Sharpness = 1.;
	
	UPROPERTY(EditAnywhere, Category=MaterialExpressionPrimitiveShape)
	bool Invert {false};

	UPROPERTY(EditAnywhere, Category=MaterialExpressionPrimitiveShape, meta = (EditCondition="Shape==EPrimitiveShapeType::Rectangle", EditConditionHides))
	FVector2D SquareSize {0.1, 0.1};

	UPROPERTY(EditAnywhere, Category=MaterialExpressionPrimitiveShape, meta = (EditCondition="Shape==EPrimitiveShapeType::Disc", EditConditionHides))
	float Radius {0.1};
	
	// this needs to be overridden due to the engine bug
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif

	UMaterialExpressionPrimitiveShape(const FObjectInitializer& ObjectInitializer) : UMaterialExpression(ObjectInitializer)
	{
#if WITH_EDITORONLY_DATA
		bHidePreviewWindow = false;
		bCollapsed = false;
		bNeedToUpdatePreview = true;
#endif
	}
	
	virtual bool GenerateHLSLExpression(FMaterialHLSLGenerator& Generator, UE::HLSLTree::FScope& Scope, int32 OutputIndex, UE::HLSLTree::FExpression const*& OutExpression) const override;
	virtual void GetCaption(TArray<FString>& OutCaptions) const override;
	virtual FString GetDescription() const override;
	virtual void GetExpressionToolTip(TArray<FString>& OutToolTip) override;
	virtual int32 Compile(FMaterialCompiler* Compiler, int32 OutputIndex) override;
	virtual int32 CompilePreview(FMaterialCompiler* Compiler, int32 OutputIndex) override;
	virtual bool NeedsRealtimePreview() override { return true; }
};
