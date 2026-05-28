#include "PBGrammar.h"
#include "Misc/AutomationTest.h"

// TODO test flags?

IMPLEMENT_SIMPLE_AUTOMATION_TEST(PBGrammarParserTestNegatives, "PCGPanelBuildings.PBGrammarParserTest.Negatives",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(PBGrammarParserTestPositives, "PCGPanelBuildings.PBGrammarParserTest.Positives",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(PBFittingAlgorithmTests, "PCGPanelBuildings.FittingAlgorithm",
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
		{"1 2 | {}+ 2 | {}*", 6},

		// grammars containing invalid symbols
		{"asdfasdf", 0}
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

FString Print(const FPanelOverrideRule& OverrideRule)
{
	return FString::Printf(TEXT("%d>%d"), OverrideRule.OverridenPanelIndex, OverrideRule.TargetPanelIndex);
}

FString Print(const FPanelFloorOverrides& FloorOverride)
{
	TArray<FString> OverrideRules;
	Algo::Transform(FloorOverride.Overrides,
	                OverrideRules,
	                [](const FPanelOverrideRule& OverrideRule)
	                {
		                return Print(OverrideRule);
	                });
	
	return FString::Printf(TEXT("[%d: %s]"), FloorOverride.FloorIndex, *FString::Join(OverrideRules, TEXT(" ")));
}

FString Print(const FPBRuleSet& RuleSet)
{
	TArray<FString> RulesAsString;
	TArray<FString> Overrides;

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

	for (const auto& FloorOverride : RuleSet.FloorOverrides)
	{
		Overrides.Add(Print(FloorOverride));
	}

	FString Result = FString::Join(RulesAsString, TEXT(" | "));
	if (Overrides.Num() > 0)
	{
		Result += " " + FString::Join(Overrides, TEXT(" "));
	}

	return Result;
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
		{"1 2 {2 3}* 2 1|2 3| 1 2 {3 4}+ 2 1", "1 2 {2 3}* 2 1 | 2 3 | 1 2 {3 4}+ 2 1 | 2 3"},

		// panel floor overrides
		{"1 2 | {2}+ [0: 3 > 4] [1: 1>2 2>3]", "1 2 | {2}+ | 1 2 | {2}+ [0: 3>4] [1: 1>2 2>3]"}
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

using FDataGenerator = TFunction<void(FPBRuleSet&, TArray<UPBPanelLayout*>&, FBox&, bool&, FString&)>;

void SimpleFitInBoundingBox(FPBRuleSet& OutRuleSet,
	TArray<UPBPanelLayout*>& OutLayouts,
	FBox& OutBoundingBox,
	bool& Expect,
	FString& TestCaseDescr)
{
	// a wide panel with wide window
	if (!OutLayouts.IsValidIndex(0))
	{
		OutLayouts.Add(NewObject<UPBPanelLayout>());
	}
	OutLayouts[0]->Id = "WideW";
	OutLayouts[0]->Width = 360.f;
	OutLayouts[0]->Height = 240.f;
	OutLayouts[0]->HasWindow = true;
	OutLayouts[0]->WindowHeight = 140.f;
	OutLayouts[0]->WindowWidth = 280.f;
	OutLayouts[0]->WindowPosition = FVector2f{0.5f, 0.5f};

	// a narrow panel with a narrow window
	if (!OutLayouts.IsValidIndex(1))
	{
		OutLayouts.Add(NewObject<UPBPanelLayout>());
	}
	OutLayouts[1]->Id = "NarrowW";
	OutLayouts[1]->Width = 200.f;
	OutLayouts[1]->Height = 240.f;
	OutLayouts[1]->HasWindow = true;
	OutLayouts[1]->WindowHeight = 140.f;
	OutLayouts[1]->WindowWidth = 120.f;
	OutLayouts[1]->WindowPosition = FVector2f{0.5f, 0.5f};

	// 30 m wide, 10 m deep, 20 m tall
	OutBoundingBox = FBox{
		FVector{0., 0., 0.},
		FVector{1000., 3000., 2000.}
	};

	// ruleset:
	// - front: repeat narrow-wide-narrow all the way
	
	OutRuleSet.Rules.Add(FPBRule{
		{
			FPBRuleItem{
				TInPlaceType<FPanelGroup>(),
				FPanelGroup{
					{1, 0, 1},
					true
				}
			}
		}
	});

	// - side: repeat narrow-narrow all the way
	OutRuleSet.Rules.Add(FPBRule{
		{
			FPBRuleItem{
				TInPlaceType<FPanelGroup>(),
				FPanelGroup{
					{1, 1},
					true
				}
			}
		}
	});

	OutRuleSet.UpdateReferencedIndices();
	
	TestCaseDescr = "Fitting a simple ruleset into bounding box should work";
	Expect = true;
}

void BoundingBoxOverflowTest(FPBRuleSet& OutRuleSet,
	TArray<UPBPanelLayout*>& OutLayouts,
	FBox& OutBoundingBox,
	bool& Expect,
	FString& TestCaseDescr)
{
	SimpleFitInBoundingBox(OutRuleSet, OutLayouts, OutBoundingBox, Expect, TestCaseDescr);

	// bounding box is too small (the minimum required panels still get generated)
	OutBoundingBox = FBox{
		FVector{0., 0., 0.},
		FVector{1., 1., 1.}
	};

	Expect = true;
	TestCaseDescr = "Fitting a ruleset into a too small bounding box still generates minimum required panels";
}

FString ToString(const EPanelBuildingSide& Side)
{
	switch (Side)
	{
	case Front:
		return "front";
	case Right:
		return "right";
	case Back:
		return "back";
	case Left:
		return "left";
	default:
		return "left";
	}
}
void PositionedPanelsToString(const TArray<FPositionedPanelInfo>& PositionedPanels, FString& OutString)
{
	EPanelBuildingSide CurrentSide = Front;
	TArray<FString> Parts;

	for (const auto& PositionedPanel : PositionedPanels)
	{
		if (PositionedPanel.Position != CurrentSide || Parts.IsEmpty())
		{
			Parts.Add("[" + ToString(PositionedPanel.Position) + "]");
			CurrentSide = PositionedPanel.Position;
		}

		Parts.Add(PositionedPanel.PanelLayout->Id + "@" + PositionedPanel.AssignedTransform.GetLocation().ToString());
	}

	OutString = FString::Join(Parts, TEXT("\n"));
}

bool PBFittingAlgorithmTests::RunTest(const FString& Parameters)
{
	// just checking that no crashes or nullptr exceptions can occur
	TArray<FDataGenerator> DataGenerators {
		&SimpleFitInBoundingBox,
		&BoundingBoxOverflowTest
	};

	// run test cases
	for (const FDataGenerator& TestDataGenerator : DataGenerators)
	{
		FPBRuleSet RuleSet;
		TArray<UPBPanelLayout*> Layouts;
		FBox BoundingBox;
		bool Expect;
		TArray<FPositionedPanelInfo> OutPositionedPanels;
		FString TestCaseDescription;

		TestDataGenerator(RuleSet, Layouts, BoundingBox, Expect, TestCaseDescription);

		const auto Result = UPCGPanelBuildingHelpers::FitPanelsToBoundingBox(RuleSet, BoundingBox, Layouts, OutPositionedPanels);
		if (Result != Expect)
		{
			this->AddError("Unexpected result for test case: " + TestCaseDescription);
			continue;
		}

		if (Result)
		{
			FString PositionedPanels;
			PositionedPanelsToString(OutPositionedPanels, PositionedPanels);
			this->AddInfo("Fitting algorithm resulted in the following assignment: \n" + PositionedPanels);
		}
	}
	
	return true;
}

