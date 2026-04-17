// Fill out your copyright notice in the Description page of Project Settings.


#include "PCGPanelHouse/Public/TemporaryObject.h"
#include "Algo/Count.h"

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

bool ParsePBGroup(const FString& Grammar, int& Iterator, FPanelGroup& OutPanelGroup, FParsingError& OutError)
{
	FString CurrentSubstring;

	if (Iterator >= Grammar.Len())
	{
		OutError.ErrorAtPosition("Expected panel group, found EOL", Iterator);
		return false;
	}

	for (int& i = Iterator; i < Grammar.Len(); i++)
	{
		TCHAR Character = Grammar[i];

		if (FChar::IsDigit(Character))
		{
			CurrentSubstring.AppendChar(Character);
		}
		else if (Character == TEXT('-'))
		{
			if (CurrentSubstring.IsEmpty())
			{
				OutError.ErrorAtPosition("Expected panel index, found '-'", Iterator);
				return false;
			}

			OutPanelGroup.PanelIndices.Add(FCString::Atoi(*CurrentSubstring));
			CurrentSubstring.Empty();
		}
		else if (Character == TEXT('{'))
		{
			OutError.ErrorAtPosition("Nexted panel groups are not allowed", Iterator);
			return false;
		}
		else if (Character == TEXT('}'))
		{
			if (!CurrentSubstring.IsEmpty())
			{
				OutPanelGroup.PanelIndices.Add(FCString::Atoi(*CurrentSubstring));
				CurrentSubstring.Empty();
			}

			Iterator++;
			break;
		}
		else
		{
			OutError.ErrorAtPosition("Unexpected character", Iterator);
			return false;
		}
	}

	if (OutPanelGroup.PanelIndices.IsEmpty())
	{
		OutError.ErrorAtPosition("Empty groups are not allowed", OutPanelGroup.Position);
		return false;
	}

	if (Iterator >= Grammar.Len())
	{
		OutError.ErrorAtPosition("Expected group reoccurrence specifier, found EOL", Iterator);
		return false;
	}

	// group reoccurrence specifier ('*' or '+')
	TCHAR ReoccurrenceSpecifier = Grammar[Iterator++];
	if (ReoccurrenceSpecifier == TEXT('+'))
	{
		OutPanelGroup.AtLeastOneOccurrence = true;
	}
	else if (ReoccurrenceSpecifier != TEXT('*'))
	{
		OutError.ErrorAtPosition("Expected a group reoccurrence specifier", Iterator);
		return false;
	}

	return true;
}

bool ParsePBGrammar(const FString& Grammar, FPBRuleSet& OutRuleSet, FString& OutErrorMessage, FParsingError& OutError)
{
	if (Grammar.TrimStartAndEnd().IsEmpty())
	{
		OutError.ErrorAtPosition("Cannot parse an empty grammar", -1);
		return false;
	}

	FPBRuleSet PBRuleSet;
	FPBRule CurrentPBRule;
	FString CurrentSubstring;

	for (int i = 0; i < Grammar.Len(); i++)
	{
		if (const TCHAR Character = Grammar[i]; FChar::IsDigit(Character))
		{
			CurrentSubstring.AppendChar(Character);
		}
		else if (Character == TEXT('-'))
		{
			if (CurrentSubstring.IsEmpty())
			{
				OutError.ErrorAtPosition("Expected panel index, found '-'", i);
				return false;
			}

			CurrentPBRule.Items.Add(FPBRuleItem<int>{FCString::Atoi(*CurrentSubstring)});
			CurrentSubstring.Empty();
		}
		else if (Character == TEXT('{'))
		{
			FPanelGroup PanelGroup;
			PanelGroup.Position = i;
			
			FParsingError GroupParsingError;

			auto Success = ParsePBGroup(Grammar, ++i, PanelGroup, GroupParsingError);
			if (!Success)
			{
				OutError.ErrorAtPosition(GroupParsingError.ErrorMessage, GroupParsingError.Position);
				return false;
			}

			CurrentPBRule.Items.Add(FPBRuleItem<FPanelGroup>{PanelGroup});

			// EOL after reading the group - validate the current rule before exiting
			if (i >= Grammar.Len())
			{
				if (!ValidatePBRule(CurrentPBRule, OutError))
				{
					return false;
				}

				PBRuleSet.Rules.Add(CurrentPBRule);
				break;
			}
		}
		else if (Character == TEXT('|'))
		{
			if (!ValidatePBRule(CurrentPBRule, OutError))
			{
				return false;
			}

			PBRuleSet.Rules.Add(CurrentPBRule);
			CurrentPBRule = FPBRule();
		}
		else
		{
			OutError.ErrorAtPosition("Unexpected character", i);
			return false;
		}
	}

	return true;
}
