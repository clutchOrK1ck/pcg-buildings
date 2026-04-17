// Fill out your copyright notice in the Description page of Project Settings.


#include "PCGPanelHouse/Public/TemporaryObject.h"
#include "Algo/Count.h"

constexpr TCHAR GDelimiter = ' ';
constexpr TCHAR GGroupOpeningChar = '{';
constexpr TCHAR GGroupClosingChar = '}';
constexpr TCHAR GRulesetSeparatorChar = '|';

/**
 * check's the rules validity (the rule should be non-empty and must not consist of a single optional panel group)
 * @param Rule 
 * @param OutError 
 * @return 
 */
bool ValidatePBRule(const FPBRule& Rule, FParsingError& OutError)
{
	if (Rule.Items.IsEmpty())
	{
		OutError.ErrorAtPosition("Empty rules are not allowed", Rule.Position);
		return false;
	}

	if (Algo::CountIf(Rule.Items,
	                  [](const FPBRuleItem& Item)
	                  {
		                  return Item.IsType<FPanelGroup>();
	                  }) > 1)
	{
		OutError.ErrorAtPosition("A rule cannot contain more than one group", Rule.Position);
		return false;
	}

	if (Rule.Items.Num() == 1 && Rule.Items[0].IsType<FPanelGroup>() && !Rule.Items[0].Get<FPanelGroup>().
		AtLeastOneOccurrence)
	{
		OutError.ErrorAtPosition("Rules containing a single optional group are not allowed", Rule.Position);
		return false;
	}

	return true;
}

/**
 * parses the next panel group (enclosed in '{', '}')
 *
 * after parsing the group, the position will be that of the last character of the group definition, i.e.
 * position of the '+' in '{1-2-3}+'
 * @param Grammar
 * @param PositionIn 
 * @param OutPanelGroup 
 * @param OutError 
 * @return 
 */
bool ParsePBGroup(const FString& Grammar, int& PositionIn, FPanelGroup& OutPanelGroup, FParsingError& OutError)
{
	FString CurrentSubstring;

	if (PositionIn >= Grammar.Len())
	{
		OutError.ErrorAtPosition("Expected panel group, found EOL", PositionIn);
		return false;
	}

	for (int& Position = PositionIn; Position < Grammar.Len(); Position++)
	{
		TCHAR Character = Grammar[Position];

		if (FChar::IsDigit(Character))
		{
			CurrentSubstring.AppendChar(Character);

			if (Position + 1 >= Grammar.Len() || !FChar::IsDigit(Grammar[Position + 1]))
			{
				OutPanelGroup.PanelIndices.Add(FPBRuleItem<int>(FCString::Atoi(*CurrentSubstring)));
				CurrentSubstring.Empty();
			}
		}
		else if (Character == GDelimiter)
		{
		}
		else if (Character == GGroupOpeningChar)
		{
			OutError.ErrorAtPosition("Nested panel groups are not allowed", Position);
			return false;
		}
		else if (Character == GGroupClosingChar)
		{
			break;
		}
		else
		{
			OutError.ErrorAtPosition("Unexpected character", Position);
			return false;
		}
	}

	if (OutPanelGroup.PanelIndices.IsEmpty())
	{
		OutError.ErrorAtPosition("Empty groups are not allowed", OutPanelGroup.Position);
		return false;
	}

	if (++PositionIn >= Grammar.Len())
	{
		OutError.ErrorAtPosition("Expected group reoccurrence specifier, found EOL", PositionIn);
		return false;
	}

	// group reoccurrence specifier ('*' or '+')
	TCHAR ReoccurrenceSpecifier = Grammar[PositionIn];
	if (ReoccurrenceSpecifier == TEXT('+'))
	{
		OutPanelGroup.AtLeastOneOccurrence = true;
	}
	else if (ReoccurrenceSpecifier != TEXT('*'))
	{
		OutError.ErrorAtPosition("Expected a group reoccurrence specifier", PositionIn);
		return false;
	}

	return true;
}

void Flush(FString& AccumulatedDigits, TArray<FPBRuleItem>& Items)
{
	if (!AccumulatedDigits.IsEmpty())
	{
		Items.Add(FPBRuleItem<int>{FCString::Atoi(*AccumulatedDigits)});
		AccumulatedDigits.Empty();
	}
}

bool ParsePBGrammar(const FString& Grammar, FPBRuleSet& OutRuleSet, FParsingError& OutError)
{
	if (Grammar.TrimStartAndEnd().IsEmpty())
	{
		OutError.ErrorAtPosition("Cannot parse an empty grammar", -1);
		return false;
	}

	FPBRule CurrentPBRule;
	CurrentPBRule.Position = 0;
	FString CurrentSubstring;

	for (int Position = 0; Position < Grammar.Len(); Position++)
	{
		if (const TCHAR Character = Grammar[Position]; FChar::IsDigit(Character))
		{
			CurrentSubstring.AppendChar(Character);

			if (Position + 1 >= Grammar.Len() || !FChar::IsDigit(Grammar[Position + 1]))
			{
				Flush(CurrentSubstring, CurrentPBRule.Items);
			}
		}
		else if (Character == GDelimiter)
		{
		}
		else if (Character == GGroupOpeningChar)
		{
			FPanelGroup PanelGroup;
			PanelGroup.Position = Position;

			FParsingError GroupParsingError;

			auto Success = ParsePBGroup(Grammar, ++Position, PanelGroup, GroupParsingError);
			if (!Success)
			{
				OutError.ErrorAtPosition(GroupParsingError.ErrorMessage, GroupParsingError.Position);
				return false;
			}

			CurrentPBRule.Items.Add(FPBRuleItem<FPanelGroup>{PanelGroup});
		}
		else if (Character == GRulesetSeparatorChar)
		{
			if (!ValidatePBRule(CurrentPBRule, OutError))
			{
				return false;
			}

			OutRuleSet.Rules.Add(CurrentPBRule);

			// initialize a new rule
			CurrentPBRule = FPBRule();
			CurrentPBRule.Position = Position + 1;
		}
		else
		{
			OutError.ErrorAtPosition("Unexpected character", Position);
			return false;
		}
	}

	// flush the last-read rule
	if (!CurrentPBRule.Items.IsEmpty())
	{
		if (!ValidatePBRule(CurrentPBRule, OutError))
		{
			return false;
		}
		OutRuleSet.Rules.Add(CurrentPBRule);
	}

	return true;
}
