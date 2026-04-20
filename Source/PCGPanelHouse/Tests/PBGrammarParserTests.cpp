#include "TemporaryObject.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(PBGrammarParserTest, "PCGPanelBuildings.PBGrammarParserTest",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

struct FNegativeTestCase
{
	FString TestGrammar;
	int ExpectedErrorPosition;
};

bool PBGrammarParserTest::RunTest(const FString& Parameters)
{
	// TODO this really feels like a usecase for LLT
	
	FNegativeTestCase Cases[] = {
	};

	for (auto NegativeTestCase : Cases)
	{
		FPBRuleSet RuleSet;
		FParsingError ParsingError;

		auto Result = ParsePBGrammar(NegativeTestCase.TestGrammar, RuleSet, ParsingError);

		if (Result)
		{
			this->AddError("Expression '" + NegativeTestCase.TestGrammar + "' was expected to be unparseable, parsed successfully instead");
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
