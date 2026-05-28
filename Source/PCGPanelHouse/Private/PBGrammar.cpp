// Fill out your copyright notice in the Description page of Project Settings.

#include "PBGrammar.h"
#include "Algo/Accumulate.h"
#include "Algo/AnyOf.h"
#include "Algo/Count.h"
#include "Algo/MaxElement.h"

constexpr TCHAR GDelimiter = ' ';
constexpr TCHAR GGroupOpeningChar = '{';
constexpr TCHAR GGroupClosingChar = '}';
constexpr TCHAR GRulesetSeparatorChar = '|';
constexpr TCHAR GFloorOverrideOpeningChar = '[';
constexpr TCHAR GFloorOverrideClosingChar = ']';
constexpr TCHAR GFloorOverrideRemapChar = '>';

const int FPBRuleSet::CompletionRules[][4] = {
	{-1, -1, -1, -1}, // 0 rules - impossible case
	{0, 0, 0, 0}, // 1 rule (front) => front, front, front, front
	{0, 1, 0, 1}, // 2 rules (front, right) => front, right, front, right
	{0, 1, 2, 1}, // 3 rules (front, right, back) => front, right, back, right
	{0, 1, 2, 3} // 4 rules (front, right, back, left) => front, right, back, left
};

/**
 * unsafe function for parsing integers, call only at valid position and a digit-char
 *
 * leaves the cursor at the last-read digit
 */
int ParseInt(const FString& Grammar, int& CursorIn)
{
	FString DigitText;

	while (CursorIn < Grammar.Len() && FChar::IsDigit(Grammar[CursorIn]))
	{
		DigitText.AppendChar(Grammar[CursorIn++]);
	}

	CursorIn--;

	return FCString::Atoi(*DigitText);
}

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
	const int GroupStartingPosition = CursorIn - 1;
	bool HasReachedGroupClosingChar{false};

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
			OutPanelGroup.PanelIndices.Add(ParseInt(Grammar, Cursor));
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
		}
		else if (Character == GRulesetSeparatorChar)
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
	// rule starts at 0 if this is the first rule, or at its opening pipe
	int RuleStartingPosition = CursorIn == 0 ? CursorIn : CursorIn - 1;
	bool HasGroup{false};

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
			OutRule.Items.Add(FPBRuleItem{TInPlaceType<int>(), ParseInt(Grammar, Cursor)});
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
		}
		else if (Character == GRulesetSeparatorChar)
		{
			break;
		}
		else
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

void FPBRule::AddPanelIndex(int ReadInt)
{
	Items.Add(FPBRuleItem(TInPlaceType<int>(), ReadInt));
}

void FPBRule::AddPanelGroup(const FPanelGroup& PanelGroup)
{
	Items.Add(FPBRuleItem(TInPlaceType<FPanelGroup>(), PanelGroup));
}

const FPBRule& FPBRuleSet::GetPBRule(const EPanelBuildingSide Side) const
{
	return this->Rules[
		this->CompletionRules[this->Rules.Num()][static_cast<int>(Side)]
	];
}

void FPBRuleSet::UpdateReferencedIndices()
{
	for (auto& Rule : this->Rules)
	{
		for (auto& Item : Rule.Items)
		{
			if (Item.IsType<int>())
			{
				Indices.Add(Item.Get<int>());
			} else
			{
				Indices.Append(Item.Get<FPanelGroup>().PanelIndices);
			}
		}
	}
}

const TSet<int>& FPBRuleSet::GetIndices() const
{
	return Indices;
}

const FPanelOverrideRule* FPBRuleSet::FindOverride(const int FloorIndex, const int PanelIndex) const
{
	auto FloorOverride = FloorOverrides.FindByPredicate([&FloorIndex](const FPanelFloorOverrides& FloorOverride)
	{
		return FloorOverride.FloorIndex == FloorIndex;
	});

	if (!FloorOverride)
	{
		return nullptr;
	}

	return FloorOverride->Overrides.FindByPredicate([&PanelIndex](const FPanelOverrideRule& OverrideRule)
	{
		return OverrideRule.OverridenPanelIndex == PanelIndex;
	});
}

FString FParsingException::GetErrorMessage() const
{
	return ErrorMessage;
}

uint32 FParsingException::GetPosition() const
{
	return Position;
}

TCHAR FUnexpectedSymbolException::GetSymbol() const
{
	return Symbol;
}

FString FUnexpectedSymbolException::GetErrorMessage() const
{
	return FParsingException::GetErrorMessage() + " '" + Symbol + "'";
}

template <typename U, typename S, typename E>
auto FBuildingGrammarParser::Read(U AccumulateCondition, S ResultTransform, E ExceptionFactory, bool SkipBlanks, int MaxLen)
{
	FString Accumulator;
	int Cursor_;

	for (Cursor_ = Cursor; Cursor_ < Grammar.Len(); Cursor_++)
	{
		const auto Char = Grammar[Cursor_];

		const bool bUnexpectedChar = !AccumulateCondition(Char);
		
		if (bUnexpectedChar)
		{
			if (!Accumulator.IsEmpty())
			{
				break;
			}

			if (FChar::IsWhitespace(Char) && SkipBlanks)
			{
				continue;
			}

			auto Exception = ExceptionFactory(Char, Cursor_);
			throw Exception;
		}
		
		Accumulator += Char;

		if (MaxLen > 0 && Accumulator.Len() >= MaxLen)
		{
			Cursor_++;
			break;
		} 
	}

	if (Accumulator.IsEmpty())
	{
		throw FEOL();
	}
	
	Cursor = Cursor_;
	return ResultTransform(Accumulator);
}

int FBuildingGrammarParser::ReadInt(bool bSkipBlanks)
{
	return Read(
		FChar::IsDigit,
		[](const FString& Accumulator)
		{
			return FCString::Atoi(*Accumulator);
		},
		[](const TCHAR& Char, const int Cursor)
		{
			return FParsingException("Expected integer", Cursor);
		},
		bSkipBlanks, -1);
}

FString FBuildingGrammarParser::ReadIdentifier(bool bSkipBlanks)
{
	return Read(
		FChar::IsIdentifier,
		[](const FString& Accumulator)
		{
			return Accumulator;
		},
		[](const TCHAR& Char, const int Cursor)
		{
			return FParsingException("Expected identifier", Cursor);
		},
		bSkipBlanks, -1);
}

TCHAR FBuildingGrammarParser::ReadRequiredChar(const TCHAR& Char, bool bSkipBlanks)
{
	return Read(
		[&Char](const TCHAR& TestChar)
		{
			return TestChar == Char;
		},
		[](const FString& Accumulator)
		{
			return Accumulator[0];
		},
		[&Char](const TCHAR& Char_, const int Cursor)
		{
			FString ErrorMessage = FString::Printf(TEXT("Expected '%c', received '%c'"), Char, Char_);
			return FParsingException(ErrorMessage, Cursor);
		},
		bSkipBlanks, 1
		);
}

TCHAR FBuildingGrammarParser::Peak(bool SkipBlanks) const
{
	for (int Cursor_ = Cursor; Cursor_ < Grammar.Len(); Cursor_++)
	{
		const auto Char = Grammar[Cursor_];
		
		if (FChar::IsWhitespace(Char) && SkipBlanks)
		{
			continue;
		}

		return Char;
	}

	return '\0';
}

TCHAR FBuildingGrammarParser::ReadChar(bool SkipBlanks)
{
	return Read(
		[](const TCHAR& Char)
		{
			return true;
		},
		[](const FString& Accumulator)
		{
			return Accumulator[0];
		},
		[](const TCHAR& Char, const int Cursor)
		{
			return FParsingException("", -1);
		},
		SkipBlanks, 1);
}

FPanelGroup FBuildingGrammarParser::ReadPanelGroup()
{
	FPanelGroup Result;
	
	ReadRequiredChar('{');

	// read panel indices
	while (Peak() != '}')
	{
		Result.PanelIndices.Add(ReadInt());
	}

	ReadRequiredChar('}');

	TCHAR GroupRepeatSpecifier = Peak();
	
	if (!GroupRepeatSpecifier || (GroupRepeatSpecifier != '*' && GroupRepeatSpecifier != '+'))
	{
		throw FParsingException("Expected '*' or '+' panel group specifier", Cursor);
	}

	Result.AtLeastOneOccurrence = GroupRepeatSpecifier == '+';
	ReadChar();

	return Result;
}

FPBRule FBuildingGrammarParser::ReadRule()
{
	FPBRule Result;
	auto Next = Peak();
	
	while (Next && Next != '|' && Next != '[')
	{
		if (FChar::IsDigit(Next))
		{
			Result.AddPanelIndex(ReadInt());
		} else if (Next == '{')
		{
			const auto Position = Cursor;
			auto PanelGroup = ReadPanelGroup();

			if (PanelGroup.PanelIndices.IsEmpty())
			{
				throw FParsingException("Empty panel groups are not allowed", Position);
			}
			
			Result.AddPanelGroup(PanelGroup);
		} else
		{
			throw FUnexpectedSymbolException(ReadChar(), Cursor-1);
		}

		Next = Peak();
	}

	return Result;
}

void FBuildingGrammarParser::ReadRules()
{
	auto Next = Peak();
	
	while (Next && Next != '[')
	{
		auto CurrentPosition = Cursor;
		FPBRule Rule = ReadRule();

		if (Rule.Items.IsEmpty())
		{
			throw FParsingException("Empty rules are not allowed", CurrentPosition);
		}

		if (Rule.Items.Num() == 1
			&& Rule.Items[0].IsType<FPanelGroup>()
			&& !Rule.Items[0].Get<FPanelGroup>().AtLeastOneOccurrence)
		{
			throw FParsingException("Rules consisting of a single optional group are not allowed", CurrentPosition);
		}

		if (Algo::CountIf(Rule.Items, [](const FPBRuleItem& Item)
		{
			return Item.IsType<FPanelGroup>();
		}) > 1)
		{
			throw FParsingException("Rules with multiple panel groups are not allowed", CurrentPosition);
		}
		
		RuleSet.Rules.Add(Rule);

		// consume the group separator if any
		if (Peak() == '|')
		{
			ReadRequiredChar('|');
		}

		Next = Peak();
	}
}

FPanelOverrideRule FBuildingGrammarParser::ReadOverrideRule()
{
	FPanelOverrideRule Result;

	Result.OverridenPanelIndex = ReadInt();
	ReadRequiredChar('>');
	Result.TargetPanelIndex = ReadInt();

	return Result;
}

FPanelFloorOverrides FBuildingGrammarParser::ReadFloorOverrideGroup()
{
	FPanelFloorOverrides Result;
	
	ReadRequiredChar('[');
	Result.FloorIndex = ReadInt();
	ReadRequiredChar(':');

	while (Peak() != ']')
	{
		Result.Overrides.Add(ReadOverrideRule());
	}
	ReadChar();
	
	return Result;
}

void FBuildingGrammarParser::ReadFloorOverrides()
{
	while (Peak() == '[')
	{
		RuleSet.FloorOverrides.Add(ReadFloorOverrideGroup());
	}
}

void FBuildingGrammarParser::Parse()
{
	try
	{
		ReadRules();

		if (RuleSet.Rules.IsEmpty())
		{
			throw FParsingException("Grammars must contain at least one rule", 0);
		}

		if (RuleSet.Rules.Num() > 4)
		{
			throw FParsingException("Grammars can contain at most 4 rules", 0);
		}
		
		ReadFloorOverrides();

		if (Peak())
		{
			throw FUnexpectedSymbolException(ReadChar(), Cursor - 1);
		}
	} catch (const FParsingException& Exception)
	{
		Error = Exception;
	}
}

FString ToString(const FParsingError& ParsingError, const FString& Grammar)
{
	FString ErrorMsg;

	ErrorMsg += "Grammar parsing error: '" + Grammar + "'\n";
	ErrorMsg += "ErrorMsg: " + ParsingError.ErrorMessage + "\n";

	if (ParsingError.Position >= 0)
	{
		ErrorMsg += "Loc:\n";
		ErrorMsg += Grammar + "\n";
		ErrorMsg += FString::ChrN(ParsingError.Position, TEXT(' ')) + "^";
	}
	
	return ErrorMsg;
}

bool ParsePBGrammar(const FString& Grammar, FPBRuleSet& OutRuleSet, FParsingError& OutError)
{
	auto Parser = FBuildingGrammarParser(Grammar);
	Parser.Parse();
	
	Parser.GetRuleSet().UpdateReferencedIndices();
	OutRuleSet = Parser.GetRuleSet();
	return true;
}

float CalculateLength(const TArray<FPanelPlacement>& Placements)
{
	float AccumulatedLength{0.};

	for (const FPanelPlacement& Placement : Placements)
	{
		if (Placement.IsRepeatableGroup)
		{
			const float GroupLength = Algo::TransformAccumulate(
				Placement.Panels,
				[](const UPBPanelLayout* Panel)
				{
					return Panel->Width;
				},
				0.0f,
				[](float Accum, float Val)
				{
					return Accum + Val;
				});
			AccumulatedLength += GroupLength * Placement.Repeat;
		}
		else
		{
			AccumulatedLength += Placement.Panels[0]->Width;
		}
	}

	return AccumulatedLength;
}

/**
 * place panels according to a rule and take the maximum wall length into consideration
 * 
 * @param InRule 
 * @param MaximumLength 
 * @param OutPlacements 
 * @return 
 */
float GeneratePlacements(const FPBRule& InRule, const TArray<UPBPanelLayout*>& PanelDefinitions, float MaximumLength,
                         TArray<FPanelPlacement>& OutPlacements)
{
	float AccumulatedLength;

	for (const auto& PanelIdx : InRule.Items)
	{
		if (PanelIdx.IsType<int>())
		{
			// add a single panel
			OutPlacements.Add(
				FPanelPlacement{
					false,
					TArray<UPBPanelLayout*>{PanelDefinitions[PanelIdx.Get<int>()]},
					1
				});
		}
		else
		{
			const auto& PanelGroup = PanelIdx.Get<FPanelGroup>();
			TArray<UPBPanelLayout*> GroupPanels;

			FPanelPlacement GroupPanelPlacement{
				true,
				TArray<UPBPanelLayout*>{},
				PanelIdx.Get<FPanelGroup>().AtLeastOneOccurrence ? 1 : 0
			};

			Algo::Transform(PanelGroup.PanelIndices, GroupPanelPlacement.Panels, [&PanelDefinitions](const int& Idx)
			{
				return PanelDefinitions[Idx];
				// TODO handle improper grammars with indices pointing to non-existing panels
			});

			// add a panel group with minimum number of repetitions so far
			OutPlacements.Add(MoveTemp(GroupPanelPlacement));
		}
	}

	AccumulatedLength = CalculateLength(OutPlacements);
	while (AccumulatedLength < MaximumLength)
	{
		const auto PreviousLength = AccumulatedLength;

		FPanelPlacement* RepeatableGroup = OutPlacements.FindByPredicate([](const FPanelPlacement& Placement)
		{
			return Placement.IsRepeatableGroup;
		});

		if (RepeatableGroup)
		{
			RepeatableGroup->Repeat += 1;
			AccumulatedLength = CalculateLength(OutPlacements);
			// TODO this introduces potential overshooting, may need to control for that

			if (AccumulatedLength <= PreviousLength) // guard against empty groups and the following infinite loop
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

	return AccumulatedLength;
}

/**
 * get direction to shift panel placement
 *
 * this results in a clockwise shifting around the building (viewed from the top)
 * @param ForSide 
 * @return 
 */
FVector GetShiftDirection(EPanelBuildingSide ForSide)
{
	switch (ForSide)
	{
	case Front:
		return FVector{0., 1., 0.};
	case Right:
		return FVector{-1., 0., 0.};
	case Back:
		return FVector{0., -1., 0.};
	case Left:
		return FVector{1., 0., 0.};
	}

	return FVector{};
}

void PositionPanelAndShiftLocation(UPBPanelLayout* Layout,
                                   FVector& LastPositionedPanelCornerLocation,
                                   EPanelBuildingSide BuildingSide,
                                   TArray<FPositionedPanelInfo>& OutPositions)
{
	auto PanelLocation = LastPositionedPanelCornerLocation
			+ GetShiftDirection(BuildingSide) * Layout->Width / 2.
			+ FVector::UpVector * Layout->Height / 2.;

	auto PanelRotation = FRotator{
		0.,
		static_cast<uint8>(BuildingSide) * 90.,
		0.
	};

	auto PanelTransform = FTransform{PanelRotation, PanelLocation};
	
	OutPositions.Add(
		FPositionedPanelInfo{
			0,
			BuildingSide,
			PanelTransform,
			Layout
		}
	);
	LastPositionedPanelCornerLocation += GetShiftDirection(BuildingSide) * Layout->Width;
}

void PositionPanels(const TArray<FPanelPlacement>& Placements,
                    EPanelBuildingSide BuildingSide,
                    FVector& LastPositionedPanelCornerLocation,
                    TArray<FPositionedPanelInfo>& OutPositions)
{
	for (const FPanelPlacement& Placement : Placements)
	{
		if (Placement.IsRepeatableGroup && Placement.Repeat > 0)
		{
			for (int i = 0; i < Placement.Repeat; i++)
			{
				for (UPBPanelLayout* PanelLayout : Placement.Panels)
				{
					PositionPanelAndShiftLocation(PanelLayout, LastPositionedPanelCornerLocation, BuildingSide,
					                              OutPositions);
				}
			}
		}
		else
		{
			PositionPanelAndShiftLocation(Placement.Panels[0], LastPositionedPanelCornerLocation, BuildingSide,
			                              OutPositions);
		}
	}
}

bool CheckRulesetReferencesValidIndex(const FPBRuleSet& Ruleset, const int NumPanelDefinitions)
{
	if (Ruleset.GetIndices().IsEmpty())
	{
		return true;
	}
	
	if (const int* MaxIndex = Algo::MaxElement(Ruleset.GetIndices()); *MaxIndex >= NumPanelDefinitions)
	{
		UE_LOG(LogTemp, Error, TEXT("Ruleset references an invalid index %d"), *MaxIndex);
		return false;
	}

	return true;
}

bool CheckValidPanelDefinitions(const TArray<UPBPanelLayout*>& Panels)
{
	if (Panels.IsEmpty() || Algo::AnyOf(Panels, [](const UPBPanelLayout* Layout)
	{
		return Layout == nullptr;
	}))
	{
		UE_LOG(LogTemp, Error, TEXT("Panel definitions contina nullptr"));
		return false;
	}

	return true;
}

bool UPCGPanelBuildingHelpers::FitPanelsToBoundingBox(const FPBRuleSet& PanelHouseRuleSet, const FBox& BoundingBox,
                                                   const TArray<UPBPanelLayout*>& Panels,
                                                   TArray<FPositionedPanelInfo>& OutPanels)
{
	if (PanelHouseRuleSet.Rules.IsEmpty())
	{
		return false;
	}
	
	if (!(CheckValidPanelDefinitions(Panels) && CheckRulesetReferencesValidIndex(PanelHouseRuleSet, Panels.Num())))
	{
		return false;
	}
	
	TArray<FPanelPlacement> FrontalPlacements;
	TArray<FPanelPlacement> RightPlacements;
	TArray<FPanelPlacement> BackPlacements;
	TArray<FPanelPlacement> LeftPlacements;

	const float FrontalWidth = GeneratePlacements(
		PanelHouseRuleSet.GetPBRule(Front),
		Panels,
		BoundingBox.GetSize().Y,
		FrontalPlacements);

	const float RightDepth = GeneratePlacements(
		PanelHouseRuleSet.GetPBRule(Right),
		Panels,
		BoundingBox.GetSize().X,
		RightPlacements);

	GeneratePlacements(
		PanelHouseRuleSet.GetPBRule(Back),
		Panels,
		FrontalWidth,
		BackPlacements);

	GeneratePlacements(
		PanelHouseRuleSet.GetPBRule(Left),
		Panels,
		RightDepth,
		LeftPlacements);

	// create the actual locations
	FVector LastPositionedPanelCornerLocation{
		BoundingBox.Max.X,
		BoundingBox.Min.Y,
		BoundingBox.Min.Z // start in front, on the left, and at the bottom
	};

	// 0th floor, other floors can inherit from this base
	PositionPanels(FrontalPlacements, Front, LastPositionedPanelCornerLocation, OutPanels);
	PositionPanels(RightPlacements, Right, LastPositionedPanelCornerLocation, OutPanels);
	PositionPanels(BackPlacements, Back, LastPositionedPanelCornerLocation, OutPanels);
	PositionPanels(LeftPlacements, Left, LastPositionedPanelCornerLocation, OutPanels);

	return true;
}

void ApplyOverrides(TArray<FPositionedPanelInfo>& Panels, const FPBRuleSet& RuleSet, const TArray<UPBPanelLayout*>& PanelLayouts)
{
	for (FPositionedPanelInfo& PositionedPanel : Panels)
	{
		auto PanelIdx = PanelLayouts.Find(PositionedPanel.PanelLayout);

		if (PanelIdx == INDEX_NONE)
		{
			UE_LOG(LogTemp, Error, TEXT("Cannot locate index of the panel for override"));
			continue;
		}
		
		if (auto Override = RuleSet.FindOverride(PositionedPanel.FloorOffset, PanelIdx); Override && PanelLayouts.IsValidIndex(Override->TargetPanelIndex))
		{
			PositionedPanel.PanelLayout = PanelLayouts[Override->TargetPanelIndex];
		}
	}
}

bool UPCGPanelBuildingHelpers::FitPanelsToBoundingBox2(const FPBRuleSet& PanelHouseRuleSet,
                                                       const FBox& TargetDimensions, 
                                                       const float BottomOffset, 
                                                       const float TopOffset, 
                                                       FBox& GeneratedDimensions,
                                                       const TArray<UPBPanelLayout*>& Panels, 
                                                       TArray<FPositionedPanelInfo>& OutPanels)
{
	if (PanelHouseRuleSet.Rules.IsEmpty())
	{
		return false;
	}
	
	if (!(CheckValidPanelDefinitions(Panels) && CheckRulesetReferencesValidIndex(PanelHouseRuleSet, Panels.Num())))
	{
		return false;
	}

	TArray<FPositionedPanelInfo> GroundFloor;
	
	TArray<FPanelPlacement> FrontalPlacements;
	TArray<FPanelPlacement> RightPlacements;
	TArray<FPanelPlacement> BackPlacements;
	TArray<FPanelPlacement> LeftPlacements;

	const float FrontalWidth = GeneratePlacements(
		PanelHouseRuleSet.GetPBRule(Front),
		Panels,
		TargetDimensions.GetSize().Y,
		FrontalPlacements);

	const float RightDepth = GeneratePlacements(
		PanelHouseRuleSet.GetPBRule(Right),
		Panels,
		TargetDimensions.GetSize().X,
		RightPlacements);

	GeneratePlacements(
		PanelHouseRuleSet.GetPBRule(Back),
		Panels,
		FrontalWidth,
		BackPlacements);

	GeneratePlacements(
		PanelHouseRuleSet.GetPBRule(Left),
		Panels,
		RightDepth,
		LeftPlacements);

	// create the actual locations
	FVector LastPositionedPanelCornerLocation{
		TargetDimensions.Max.X,
		TargetDimensions.Min.Y,
		TargetDimensions.Min.Z // start in front, on the left, and at the bottom
	};

	// 0th floor, other floors can inherit from this base
	PositionPanels(FrontalPlacements, Front, LastPositionedPanelCornerLocation, GroundFloor);
	PositionPanels(RightPlacements, Right, LastPositionedPanelCornerLocation, GroundFloor);
	PositionPanels(BackPlacements, Back, LastPositionedPanelCornerLocation, GroundFloor);
	PositionPanels(LeftPlacements, Left, LastPositionedPanelCornerLocation, GroundFloor);

	// positioned panels can now be simply copied over to other floors with a Z-offset
	auto AvailableHeight = TargetDimensions.Max.Z - BottomOffset - TopOffset;
	auto FloorHeight = Panels[0]->Height;
	auto NumFloors = FMath::FloorToInt32(AvailableHeight / FloorHeight);

	for (auto Floor = 0; Floor < NumFloors; Floor++)
	{
		TArray<FPositionedPanelInfo> CurrentFloor;
		Algo::Transform(GroundFloor, CurrentFloor, [Floor, FloorHeight, BottomOffset](const FPositionedPanelInfo& Positioning)
		{
			FPositionedPanelInfo FloorPositioning = Positioning;
			FloorPositioning.FloorOffset = Floor;

			auto OffsetLocation = Positioning.AssignedTransform.GetLocation();
			OffsetLocation.Z = BottomOffset + FloorHeight * Floor + FloorHeight / 2.;
			
			FloorPositioning.AssignedTransform = FTransform{
				Positioning.AssignedTransform.GetRotation(),
				OffsetLocation
			};

			return FloorPositioning;
		});
		
		OutPanels.Append(CurrentFloor);
	}

	// apply floor-overrides
	ApplyOverrides(OutPanels, PanelHouseRuleSet, Panels);
	
	// write the dimensions of the generated building to the out bounding box
	GeneratedDimensions.Min = FVector{0., -RightDepth, 0.};
	GeneratedDimensions.Max = FVector{FrontalWidth, 0., BottomOffset + NumFloors * FloorHeight + TopOffset};

	return true;
}

void UPCGPanelBuildingHelpers::ParseGrammar(const FString& Grammar, FPBRuleSet& OutRuleSet, bool& Success,
                                            FString& ErrorString)
{
	FParsingError ParsingError;
	ParsePBGrammar(Grammar, OutRuleSet, ParsingError);
	if (!ParsingError.ErrorMessage.IsEmpty())
	{
		Success = false;
		ErrorString = ToString(ParsingError, Grammar);
		return;
	}

	Success = true;
}

void UPCGPanelBuildingHelpers::GetReferencedIndices(const FPBRuleSet& RuleSet, TArray<int>& Indices)
{
	for (const auto& Idx : RuleSet.GetIndices())
	{
		Indices.Add(Idx);
	}
}
