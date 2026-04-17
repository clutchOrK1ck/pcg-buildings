// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TemporaryObject.generated.h"

struct FParsedPBExpression
{
	int Position; // the position in the grammar that this expression was parsed at
};

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
struct FPanelGroup : FParsedPBExpression
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
struct FPBRule : FParsedPBExpression
{
	TArray<FPBRuleItem> Items;
};

enum EPanelBuildingSide
{
	Front,
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
struct FPBRuleSet
{
	TArray<FPBRule> Rules; // rules defined by the user
	
	const FPBRule GetPBRule(EPanelBuildingSide Side) const;
};

/**
 * an error that occurred while parsing the panel building grammar
 */
struct FParsingError
{
	FString ErrorMessage;
	int Position {-1};

	void ErrorAtPosition(const FString& ErrorMessage, const int Position)
	{
		this->ErrorMessage = ErrorMessage;
		this->Position = Position;
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
bool ParsePBGrammar(const FString& Grammar, FPBRuleSet& OutRuleSet, FString& OutErrorMessage);

/**
 * 
 */
UCLASS()
class PCGPANELHOUSE_API UTemporaryObject : public UObject
{
	GENERATED_BODY()
};
