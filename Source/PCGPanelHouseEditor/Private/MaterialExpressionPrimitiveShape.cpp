// Fill out your copyright notice in the Description page of Project Settings.

#include "MaterialCompiler.h"
#include "MaterialHLSLTree.h"
#include "MaterialHLSLGenerator.h"
#include "MaterialExpressionPrimitiveShape.h"

FString ToString(const EPrimitiveShapeType Shape)
{
	switch (Shape)
	{
	case EPrimitiveShapeType::Rectangle:
		return "Rectangle";
	default:
		return "";
	}
}

int32 FLegacyShapeFactory::GetUVsShapeSpace() const
{
	return Compiler->Sub(
		UV,
		Compiler->Constant2(GetSettings().Center.X, GetSettings().Center.Y));
}

int32 FLegacyShapeFactory::ApplySharpness(int32 InputIndex) const
{
	float SharpnessCoefficient = FMath::Pow(0.01, GetSettings().Sharpness);
	return Compiler->Power(InputIndex, Compiler->Constant(SharpnessCoefficient));
}

int32 FLegacyShapeFactory::Create()
{
	int32 Distances = ComputeDistances();
	int32 NormalizedDistances = NormalizeDistances(Distances);
	int32 WithSharpness = ApplySharpness(NormalizedDistances);

	// apply the inversion
	if (GetSettings().bInvert)
	{
		return Compiler->Sub(
			Compiler->Constant(1.),
			WithSharpness);
	}

	return WithSharpness;
}

int32 FLegacyRectFactory::ComputeDistances()
{
	// distance from the absolute UV to the square's top-right corner
	int32 SimpleDistance = Compiler->Sub(
		Compiler->Constant2(Extent.X, Extent.Y),
		Compiler->Abs(GetUVsShapeSpace())
	);

	// take the 'overshooting' component of the distance by only considering negative vector coord and compute length
	return Compiler->Length(
		Compiler->Min(
			SimpleDistance,
			Compiler->Constant(0.)
		)
	);
}

int32 FLegacyRectFactory::NormalizeDistances(int32 Distances)
{
	// remap distance values to (0, 1)
	float MarginV = 0.5 - Extent.Y;
	float MarginU = 0.5 - Extent.X;

	return Compiler->Clamp(
		Compiler->Div(
			Distances,
			Compiler->Constant(FMath::Min(MarginU, MarginV))
		),
		Compiler->Constant(0.),
		Compiler->Constant(1.)
	);
}

int32 FLegacyDiscFactory::ComputeDistances()
{
	// distance to disc is simply the difference length - radius, only positive
	return Compiler->Max(
		Compiler->Sub(
			Compiler->Length(GetUVsShapeSpace()),
			Compiler->Constant(Radius)),
		Compiler->Constant(0.)
	);
}

int32 FLegacyDiscFactory::NormalizeDistances(int32 Distances)
{
	// remap distances to (0;1)
	return Compiler->Clamp(Compiler->Div(
		                       Distances,
		                       Compiler->Constant(0.5 - Radius)),
	                       Compiler->Constant(0.),
	                       Compiler->Constant(1.)
	);
}

bool UMaterialExpressionPrimitiveShape::GenerateHLSLExpressionRect(FMaterialHLSLGenerator& Generator,
                                                                   UE::HLSLTree::FScope& Scope, int32 OutputIndex, UE::HLSLTree::FExpression const*& OutExpression) const
{
	const UE::HLSLTree::FExpression* UVs = UV.AcquireHLSLExpressionOrExternalInput(Generator,
		Scope,
		UE::HLSLTree::Material::MakeInputTexCoord(0));

	OutExpression = UVs;
	return true;
}

#if WITH_ENGINE
bool UMaterialExpressionPrimitiveShape::CanEditChange(const FProperty* InProperty) const
{
	bool bIsEditable = UObject::CanEditChange(InProperty);
	if (bIsEditable && InProperty != nullptr)
	{
		static FName OverridingInputPropertyMetaData(TEXT("OverridingInputProperty"));

		if (InProperty->HasMetaData(OverridingInputPropertyMetaData))
		{
			const FString& OverridingPropertyName = InProperty->GetMetaData(OverridingInputPropertyMetaData);

			FStructProperty* StructProp = FindFProperty<FStructProperty>(GetClass(), *OverridingPropertyName);
			if (ensure(StructProp != nullptr))
			{
				static FName RequiredInputMetaData(TEXT("RequiredInput"));

				// Must be a single FExpressionInput member, not an array, and must be tagged with metadata RequiredInput="false"
				if (ensure(	StructProp->Struct->GetFName() == NAME_ExpressionInput &&
							StructProp->ArrayDim == 1 &&
							StructProp->HasMetaData(RequiredInputMetaData) &&
							!StructProp->GetBoolMetaData(RequiredInputMetaData)))
				{
					const FExpressionInput* Input = StructProp->ContainerPtrToValuePtr<FExpressionInput>(this);

					if (Input->Expression != nullptr && Input->GetTracedInput().Expression != nullptr)
					{
						bIsEditable = false;
					}
				}
			}
		}

		// TODO report if not fixed in a later engine version. this here is a bug when a more complex condition is used
		/*if (bIsEditable)
		{
			// If the property has EditCondition metadata, then whether it's editable depends on the other EditCondition property
			const FString EditConditionPropertyName = InProperty->GetMetaData(TEXT("EditCondition"));
			if (!EditConditionPropertyName.IsEmpty())
			{
				FBoolProperty* EditConditionProperty = FindFProperty<FBoolProperty>(GetClass(), *EditConditionPropertyName);
				{
					bIsEditable = *EditConditionProperty->ContainerPtrToValuePtr<bool>(this);
				}
			}
		}*/
	}

	return bIsEditable;
}
#endif

bool UMaterialExpressionPrimitiveShape::GenerateHLSLExpression(FMaterialHLSLGenerator& Generator,
                                                               UE::HLSLTree::FScope& Scope, int32 OutputIndex, UE::HLSLTree::FExpression const*& OutExpression) const
{
	switch (Shape)
	{
	case EPrimitiveShapeType::Rectangle:
		return GenerateHLSLExpressionRect(Generator, Scope, OutputIndex, OutExpression);
	default:
		return false;
	}
}

void UMaterialExpressionPrimitiveShape::GetCaption(TArray<FString>& OutCaptions) const
{
	OutCaptions.Add("Primitive shape: " + ToString(Shape));
}

FString UMaterialExpressionPrimitiveShape::GetDescription() const
{
	return "Manipulate UVs to create a primitive shape";
}

void UMaterialExpressionPrimitiveShape::GetExpressionToolTip(TArray<FString>& OutToolTip)
{
	OutToolTip.Add("Manipulate UVs to create a primitive shape");
}

int32 UMaterialExpressionPrimitiveShape::Compile(FMaterialCompiler* Compiler, int32 OutputIndex)
{
	TUniquePtr<FLegacyShapeFactory> Factory;
	
	FShapeSettings SharedSettings {
		Center, Sharpness, Invert
	};

	int32 UVs = UV.GetTracedInput().Expression ? UV.Compile(Compiler) : Compiler->TextureCoordinate(0, false, false);
	
	switch (Shape)
	{
	case EPrimitiveShapeType::Rectangle:
		Factory = MakeUnique<FLegacyRectFactory>(Compiler, UVs, SharedSettings, SquareSize/2.);
		break;
	case EPrimitiveShapeType::Disc:
		Factory = MakeUnique<FLegacyDiscFactory>(Compiler, UVs, SharedSettings, Radius);
		break;
	default:
		break;
	}

	if (Factory)
	{
		return Factory->Create();
	}

	return -1;
}

int32 UMaterialExpressionPrimitiveShape::CompilePreview(FMaterialCompiler* Compiler, int32 OutputIndex)
{
	int32 R = Compile(Compiler, OutputIndex);

	// preview expects color, so it will show red instead of white - need to duplicate to other channels
	return Compiler->AppendVector(Compiler->AppendVector(R, R), R);
}
