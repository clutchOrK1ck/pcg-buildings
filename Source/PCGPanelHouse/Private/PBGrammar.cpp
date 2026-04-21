// Fill out your copyright notice in the Description page of Project Settings.

#include "PBGrammar.h"

constexpr TCHAR GDelimiter = ' ';
constexpr TCHAR GGroupOpeningChar = '{';
constexpr TCHAR GGroupClosingChar = '}';
constexpr TCHAR GRulesetSeparatorChar = '|';

const int FPBRuleSet::CompletionRules[][4] = {
	{-1, -1, -1, -1},		// 0 rules - impossible case
	{0, 0, 0, 0},			// 1 rule (front) => front, front, front, front
	{0, 1, 0, 1},			// 2 rules (front, right) => front, right, front, right
	{0, 1, 2, 1},			// 3 rules (front, right, back) => front, right, back, right
	{0, 1, 2, 3}			// 4 rules (front, right, back, left) => front, right, back, left
};

/**
 * parses the next panel group (enclosed in '{', '}')
 *
 * after parsing the group, the position will be that of the last character of the group definition, i.e.
 * position of the '+' in '{1-2-3}+'
 * @param Grammar
 * @param CursorIn 
 * @param OutPanelGroup 
 * @param OutError 
 * @return 
 */
bool ParsePBGroup(const FString& Grammar, int& CursorIn, FPanelGroup& OutPanelGroup, FParsingError& OutError)
{
	FString DigitAccumulator;
	const int GroupStartingPosition = CursorIn - 1;
	bool HasReachedGroupClosingChar {false};
	
	if (CursorIn >= Grammar.Len())
	{
		OutError.ErrorAtPosition("Expected panel group, found EOL", GroupStartingPosition);
		return false;
	}

	for (int& Cursor = CursorIn; Cursor < Grammar.Len(); Cursor++)
	{
		TCHAR Character = Grammar[Cursor];

		if (FChar::IsDigit(Character))
		{
			DigitAccumulator.AppendChar(Character);

			if (Cursor + 1 >= Grammar.Len() || !FChar::IsDigit(Grammar[Cursor + 1]))
			{
				OutPanelGroup.PanelIndices.Add(FCString::Atoi(*DigitAccumulator));
				DigitAccumulator.Empty();
			}
		}
		else if (Character == GDelimiter)
		{
		}
		else if (Character == GGroupOpeningChar)
		{
			OutError.ErrorAtPosition("Nested panel groups are not allowed", Cursor);
			return false;
		}
		else if (Character == GGroupClosingChar)
		{
			HasReachedGroupClosingChar = true;
			break;
		} else if (Character == GRulesetSeparatorChar)
		{
			// we don't throw unexpected char here - instead treat this as the group missing its closing char
			break;
		}
		else
		{
			OutError.ErrorAtPosition("Unexpected character", Cursor);
			return false;
		}
	}

	if (!HasReachedGroupClosingChar)
	{
		OutError.ErrorAtPosition("Group missing closing bracket", GroupStartingPosition);
		return false;
	}
	
	if (OutPanelGroup.PanelIndices.IsEmpty())
	{
		OutError.ErrorAtPosition("Empty groups are not allowed", GroupStartingPosition);
		return false;
	}

	if (++CursorIn >= Grammar.Len())
	{
		OutError.ErrorAtPosition("Expected group reoccurrence specifier, found EOL", CursorIn);
		return false;
	}

	// group reoccurrence specifier ('*' or '+')
	TCHAR ReoccurrenceSpecifier = Grammar[CursorIn];
	if (ReoccurrenceSpecifier == TEXT('+'))
	{
		OutPanelGroup.AtLeastOneOccurrence = true;
	}
	else if (ReoccurrenceSpecifier != TEXT('*'))
	{
		OutError.ErrorAtPosition("Expected a group reoccurrence specifier", CursorIn);
		return false;
	}

	return true;
}

/**
 * parse the panel-building rule from the grammar
 * 
 * returns an error if the rule is not found or is incomplete
 *
 * after parsing, the cursor will be at the '|' rule separator char if present, otherwise it'll be EOL
 * 
 * @param Grammar 
 * @param OutRule 
 * @param OutError 
 * @return 
 */
bool ParsePBRule(const FString& Grammar, int& CursorIn, FPBRule& OutRule, FParsingError& OutError)
{
	FString DigitAccumulator;
	// rule starts at 0 if this is the first rule, or at its opening pipe
	int RuleStartingPosition = CursorIn == 0 ? CursorIn : CursorIn - 1;
	bool HasGroup {false};

	if (CursorIn >= Grammar.Len())
	{
		OutError.ErrorAtPosition("Expected rule, found EOL", RuleStartingPosition);
		return false;
	}

	for (int& Cursor = CursorIn; Cursor < Grammar.Len(); Cursor++)
	{
		TCHAR Character = Grammar[Cursor];

		if (FChar::IsDigit(Character))
		{
			DigitAccumulator.AppendChar(Character);

			// look up the next char and flush digits accumulator if needed
			if (Cursor + 1 >= Grammar.Len() || !FChar::IsDigit(Grammar[Cursor + 1]))
			{
				OutRule.Items.Add(FPBRuleItem{TInPlaceType<int>(), FCString::Atoi(*DigitAccumulator)});
				DigitAccumulator.Empty();
			}
		}
		else if (Character == GDelimiter)
		{
		}
		else if (Character == GGroupOpeningChar)
		{
			// a group already exists - multiple groups are not allowed
			if (HasGroup)
			{
				OutError.ErrorAtPosition("Multiple groups within a rule are not allowed", Cursor);
				return false;
			}
			
			FPanelGroup PanelGroup;
			FParsingError GroupParsingError;

			auto Result = ParsePBGroup(Grammar, ++Cursor, PanelGroup, GroupParsingError);
			if (!Result)
			{
				OutError = GroupParsingError;
				return false;
			}

			OutRule.Items.Add(FPBRuleItem{TInPlaceType<FPanelGroup>(), PanelGroup});
			HasGroup = true;
		} else if (Character == GRulesetSeparatorChar)
		{
			break;
		} else
		{
			OutError.ErrorAtPosition("Unexpected character", Cursor);
			return false;
		}
	}

	// validate: empty rules are not allowed
	if (OutRule.Items.IsEmpty())
	{
		OutError.ErrorAtPosition("Empty rules are not allowed", RuleStartingPosition);
		return false;
	}

	// validate: rules with a single optional group are not allowed
	if (OutRule.Items.Num() == 1
		&& OutRule.Items[0].IsType<FPanelGroup>()
		&& !OutRule.Items[0].Get<FPanelGroup>().AtLeastOneOccurrence)
	{
		OutError.ErrorAtPosition("Rules with a single optional panel group are not allowed", RuleStartingPosition);
		return false;
	}
	
	return true;
}

const FPBRule& FPBRuleSet::GetPBRule(const EPanelBuildingSide Side) const
{
	return this->Rules[
		this->CompletionRules[this->Rules.Num()][static_cast<int>(Side)]
	];
}

bool ParsePBGrammar(const FString& Grammar, FPBRuleSet& OutRuleSet, FParsingError& OutError)
{
	if (Grammar.TrimStartAndEnd().IsEmpty())
	{
		OutError.ErrorAtPosition("Cannot parse an empty grammar", -1);
		return false;
	}

	int Cursor {0};
	int NumberOfRules {0}; // number of rules parsed
	
	while (Cursor < Grammar.Len())
	{
		// cursor at the ruleset separator after reading the previous rule
		if (Cursor > 0 && Grammar[Cursor] == GRulesetSeparatorChar)
		{
			if (NumberOfRules >= 4)
			{
				OutError.ErrorAtPosition("Rules cannot exceed 4", Cursor);
				return false;
			}
			
			Cursor++;
		}
		
		FPBRule Rule;
		FParsingError RuleParsingError;

		if (!ParsePBRule(Grammar, Cursor, Rule, RuleParsingError))
		{
			OutError = RuleParsingError;
			return false;
		}

		OutRuleSet.Rules.Add(Rule);
		NumberOfRules++;
	}

	return true;
}
