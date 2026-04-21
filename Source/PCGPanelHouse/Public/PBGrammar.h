// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"



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
struct FPBRuleSet
{
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
