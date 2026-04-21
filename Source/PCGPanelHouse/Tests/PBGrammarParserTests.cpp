#include "PBGrammar.h"
#include "Misc/AutomationTest.h"

// TODO test flags?

IMPLEMENT_SIMPLE_AUTOMATION_TEST(PBGrammarParserTestNegatives, "PCGPanelBuildings.PBGrammarParserTest.Negatives",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(PBGrammarParserTestPositives, "PCGPanelBuildings.PBGrammarParserTest.Positives",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

struct FNegativeTestCase
{
	FString TestGrammar; // the grammar that should fail to be parsed
	int ExpectedErrorPosition; // the position in the grammar string where the error is expected to have occurred
};

bool PBGrammarParserTestNegatives::RunTest(const FString& Parameters)
{
	// TODO this really feels like a use case for LLT, not automation tests

	FNegativeTestCase Cases[] = {
		// empty grammars are not allowed
		{"", -1},
		{" ", -1},
		{"     ", -1},

		// rules that consist of a single optional group are not allowed
		{"1 2 | {2 3}*", 4},
		{"{1 2}*", 0},

		// groups missing closing bracket
		{"{1 2 | {1 2}*", 0},
		{"{1 2}+ | {1 2 3", 9},

		// nested groups are not allowed
		{"{1 2 {1 2}*", 5}, // note: this one is also missing the closing bracket
		{"{1 2 {1 2}}+", 5},

		// unexpected tokens
		{"1 2i | {panel}*", 3},

		// multiple groups within a single rule are not allowed
		{"{1 2}* {2 3}+", 7},
		{"{1}+ | {2 3}* {1 2 3}+", 14},

		// groups must have the reoccurrence specifier
		{"{1 2 } +", 6},
		{"{1 2 3} *", 7},

		// maximum of four rules  for four building sides
		{"1|2|3|4|5", 7},

		// no empty rules
		{" | ", 0},
		{"1 2| | {3 5}*", 3},
		{"||", 0},
		{"1 2||", 3},

		// no empty groups
		{" 1 2 | {} |", 7}, // note that this one also breaks multiple other rules
		{"1 2 | {}+ 2 | {}*", 6}
	};

	for (const auto& NegativeTestCase : Cases)
	{
		FPBRuleSet RuleSet;
		FParsingError ParsingError;

		auto Result = ParsePBGrammar(NegativeTestCase.TestGrammar, RuleSet, ParsingError);

		if (Result)
		{
			this->AddError(
				"Expression '" + NegativeTestCase.TestGrammar +
				"' was expected to be unparseable, parsed successfully instead");
			continue;
		}

		if (NegativeTestCase.ExpectedErrorPosition != ParsingError.Position)
		{
			this->AddError("Expected parsing error at position {"
				+ FString::FromInt(NegativeTestCase.ExpectedErrorPosition)
				+ "}, received {"
				+ FString::FromInt(ParsingError.Position)
				+ "} instead in grammar '"
				+ NegativeTestCase.TestGrammar + "'");
		}
	}

	return true;
}

FString Print(const FPanelGroup& Group)
{
	TArray<FString> IndicesAsString;
	Algo::Transform(
		Group.PanelIndices,
		IndicesAsString,
		[](const int& Index)
		{
			return FString::FromInt(Index);
		});

	return "{" + FString::Join(IndicesAsString, TEXT(" ")) + "}" + (
		Group.AtLeastOneOccurrence ? "+" : "*"
	);
}

FString Print(const FPBRule& Rule)
{
	TArray<FString> RuleItemsAsString;
	Algo::Transform(
		Rule.Items,
		RuleItemsAsString,
		[](const FPBRuleItem& Item)
		{
			return Item.IsType<int>() ? FString::FromInt(Item.Get<int>()) : Print(Item.Get<FPanelGroup>());
		});
	return FString::Join(RuleItemsAsString, TEXT(" "));
}

FString Print(const FPBRuleSet& RuleSet)
{
	TArray<FString> RulesAsString;

	EPanelBuildingSide Sides[] = {
		EPanelBuildingSide::Front,
		EPanelBuildingSide::Right,
		EPanelBuildingSide::Back,
		EPanelBuildingSide::Left
	};
	
	for (const auto& Side : Sides)
	{
		RulesAsString.Add(Print(RuleSet.GetPBRule(Side)));
	}

	return FString::Join(RulesAsString, TEXT(" | "));
}

struct FPositiveTestCase
{
	FString TestGrammar; // Grammar under test
	FString ExpectedGrammar; // Grammar that should be returned when Printing the parsed ruleset
};

bool PBGrammarParserTestPositives::RunTest(const FString& Parameters)
{
	FPositiveTestCase TestCases[] = {
		// completion of incomplete panel building grammars
		{"1 2     ", "1 2 | 1 2 | 1 2 | 1 2"},
		{"1 2{2 3}+|      3", "1 2 {2 3}+ | 3 | 1 2 {2 3}+ | 3"},
		{"1|2|3", "1 | 2 | 3 | 2"},
		{"1 | {2 3 }+ |2|{2 3}+", "1 | {2 3}+ | 2 | {2 3}+"},

		// general parsing correctness test
		{"1 2 {2 3}* 2 1|2 3| 1 2 {3 4}+ 2 1", "1 2 {2 3}* 2 1 | 2 3 | 1 2 {3 4}+ 2 1 | 2 3"}
	};

	for (const auto& TestCase : TestCases)
	{
		FPBRuleSet RuleSet;
		FParsingError ParsingError;
		
		auto Result = ParsePBGrammar(TestCase.TestGrammar, RuleSet, ParsingError);

		if (!Result)
		{
			this->AddError("Grammar '" + TestCase.TestGrammar + "' could not be parsed successfully");
			continue;
		}

		FString StringifiedRuleSet = Print(RuleSet);
		this->TestEqual("Grammar parsed correctly", StringifiedRuleSet, TestCase.ExpectedGrammar);
	}
	
	return true;
}
