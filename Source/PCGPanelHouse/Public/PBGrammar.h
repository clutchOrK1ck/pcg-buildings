// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PBPanelLayout.h"
#include "PBGrammar.generated.h"


/**
 * describes a reoccurring group of panels
 * 
 * in the panel building rule grammar, a group is enclosed in curly braces:
 * 
 * 1-2-{1-2-3}*-2-1
 *
 * - the number of occurrences can be set to 0 or more (*) or 1 or more (+)
 * - nested groups are not allowed
 * - a single rule can contain only one panel group
 */
struct FPanelGroup
{
	TArray<int> PanelIndices;
	bool AtLeastOneOccurrence {false};
};

using FPBRuleItem = TVariant<int, FPanelGroup>;

/**
 * describes a rule for placement of panels
 *
 * The rule is a grammar of form:
 *
 * 1-2-{1-2-3}*-2-1
 */
struct FPBRule
{
	TArray<FPBRuleItem> Items;
};

UENUM(BlueprintType)
enum EPanelBuildingSide
{
	Front = 0,
	Right,
	Back,
	Left
};

/**
 * describes a set of rules for placement of panels in a panel building
 *
 * the pipe `|` denotes the corner of the building
 *
 * the rules are defined in the following order:
 * - front side
 * - right side
 * - back side
 * - left side
 *
 * front/right/back/left are defined from actor's forward/right vectors
 */
USTRUCT(BlueprintType)
struct FPBRuleSet
{
	GENERATED_BODY()
	
	// if the number of rules actually stored is less than the index of requested side, we "complete" the rule
	static const int CompletionRules[][4];
	
	TArray<FPBRule> Rules; // rules defined by the user
	
	const FPBRule& GetPBRule(const EPanelBuildingSide Side) const;
};

/**
 * an error that occurred while parsing the panel building grammar
 */
struct FParsingError
{
	FString ErrorMessage;
	int Position {-1};

	FParsingError()
	{
	}
	
	FParsingError(const FParsingError& Other)
		: ErrorMessage(Other.ErrorMessage),
		  Position(Other.Position)
	{
	}

	FParsingError(FParsingError&& Other) noexcept
		: ErrorMessage(std::move(Other.ErrorMessage)),
		  Position(Other.Position)
	{
	}

	FParsingError& operator=(const FParsingError& Other)
	{
		if (this == &Other)
			return *this;
		ErrorMessage = Other.ErrorMessage;
		Position = Other.Position;
		return *this;
	}

	FParsingError& operator=(FParsingError&& Other) noexcept
	{
		if (this == &Other)
			return *this;
		ErrorMessage = std::move(Other.ErrorMessage);
		Position = Other.Position;
		return *this;
	}

	void ErrorAtPosition(const FString& InErrorMessage, const int InPosition)
	{
		this->ErrorMessage = InErrorMessage;
		this->Position = InPosition;
	}
};

/**
 * parse a grammar string (like 1-2-{2-3}+-2-1)
 * 
 * @param Grammar
 * @param OutRuleSet
 * @param OutErrorMessage information on parsing errors
 * @return 
 */
bool ParsePBGrammar(const FString& Grammar, FPBRuleSet& OutRuleSet, FParsingError& OutError);



USTRUCT(BlueprintType)
struct FPositionedPanelInfo
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	int FloorOffset;

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EPanelBuildingSide> Position;

	/**
	 * assigned panel location and rotation
	 *
	 * the first panel will be positioned in front and on the left of the bounding box
	 */
	UPROPERTY(BlueprintReadOnly)
	FVector AssignedLocation;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPBPanelLayout> PanelLayout; // a reference to the panel layout for this panel
};

/**
 * a structure used by the fitting algorithm
 *
 * it defines a group of one or more panels, to be placed zero or more times
 */
struct FPanelPlacement
{
	bool IsRepeatableGroup;				// it is a group of panels, to be repeated zero or more times
	TArray<UPBPanelLayout*> Panels;		// panels in this group
	int Repeat;							// how many times this group is repeated
};

UCLASS()
class PCGPANELHOUSE_API UPCGPanelHouseGrammar : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * fits a grammar to a bounding box
	 *
	 * the fitting algorithm will repeat or remove (if optional) panel groups to fit the bounding box as closely as possible
	 * 
	 * @param PanelHouseRuleSet the rule set parsed from a building grammar
	 * @param BoundingBox bounding box describing the desired building dimensions
	 * @param Panels a collection of panels used by this building
	 * @param OutPanels an array of panel info (position, meta, etc.) that can be used to generate PCG point data
	 */
	UFUNCTION(BlueprintCallable)
	static void FitPanelsToBoundingBox(const FPBRuleSet& PanelHouseRuleSet, const FBox& BoundingBox, const TArray<UPBPanelLayout*>& Panels, TArray<FPositionedPanelInfo>& OutPanels);

	UFUNCTION(BlueprintCallable)
	static void ParseGrammar(const FString& Grammar, FPBRuleSet& OutRuleSet, bool& Success, FString& ErrorString);
};
