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

struct FPanelOverrideRule
{
	int OverridenPanelIndex;	// override panels with this index
	int TargetPanelIndex;		// override to a panel with this index
};

/**
 * overrides in panel indices for a given floor
 */
struct FPanelFloorOverrides
{
	int FloorIndex;						// to which floor these overrides apply
	TArray<FPanelOverrideRule> Overrides;
};

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
	void AddPanelIndex(int ReadInt);
	void AddPanelGroup(const FPanelGroup& PanelGroup);
};

UENUM(BlueprintType)
enum EPanelBuildingSide : uint8
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

private:
	// if the number of rules actually stored is less than the index of requested side, we "complete" the rule
	static const int CompletionRules[][4];
	TSet<int> Indices {}; // panel indices that are referenced by this ruleset

public:
	TArray<FPBRule> Rules; // rules defined by the user
	TArray<FPanelFloorOverrides> FloorOverrides; // overrides of panels per floor
	
	const FPBRule& GetPBRule(const EPanelBuildingSide Side) const;

	// collects panel indices referenced by this panel
	void UpdateReferencedIndices();
	
	const TSet<int>& GetIndices() const;
	const FPanelOverrideRule* FindOverride(const int FloorIndex, const int PanelIndex) const;
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

class FParsingException
{
	FString ErrorMessage;
	uint32 Position;

public:
	FParsingException(const FString& ErrorMessage, const uint32 InPosition) : ErrorMessage(ErrorMessage), Position(InPosition) {}
	virtual ~FParsingException() {}
	
	virtual FString GetErrorMessage() const;
	uint32 GetPosition() const;
};

class FEOL : public FParsingException
{
public:
	FEOL() : FParsingException("Unexpected end of line", -1) {}
};

class FUnexpectedSymbolException : public FParsingException
{
	TCHAR Symbol;
public:
	FUnexpectedSymbolException(const TCHAR& Symbol, const uint32 InPosition) :
		FParsingException("Unexpected symbol", InPosition),
		Symbol(Symbol)
	{
	}
	
	TCHAR GetSymbol() const;
	virtual FString GetErrorMessage() const override;
};


class FBuildingGrammarParser
{
	FString Grammar;
	uint32 Cursor;
	FPBRuleSet RuleSet;
	FParsingException Error;

	/**
	 * accumulates characters into accumulator and returns the accumulator transformed into some value
	 *
	 * if accumulator is still empty when EOL is reached or a test condition returns 'false', an exception will be thrown
	 * 
	 * @tparam U a function taking a char and returning a bool
	 * @tparam S a function taking a string a returning some value
	 * @tparam E a function that accepts a char and a position and returns an exception object
	 * @param AccumulateCondition test whether to accumulate a given char
	 * @param ResultTransform transform the accumulator into a value
	 * @param SkipBlanks whether to ignore preceding blanks
	 * @param MaxLen accumulate only this many chars, then return
	 * @return 
	 */
	template<typename U, typename S, typename E>
	auto Read(U AccumulateCondition, S ResultTransform, E ExceptionFactory, bool SkipBlanks = true, int MaxLen = -1);
	
	/**
	 * attempt to read an integer and throw if EOL or an unexpected symbol are encountered
	 * @param SkipBlanks skip blanks preceding the integer
	 * @return 
	 */
	int ReadInt(bool SkipBlanks = true);

	/**
	 * attempt to read an identifier (alnum and '_') and throw if EOL or an unexpected symbol are encountered
	 * @param SkipBlanks skip blanks preceding the integer
	 * @return 
	 */
	FString ReadIdentifier(bool SkipBlanks = true);

	/**
	 * reads the required character, throws if EOL or another character are encountered
	 * @param SkipBlanks skip blanks preceding the char
	 * @param Char the required char
	 */
	TCHAR ReadRequiredChar(const TCHAR& Char, bool SkipBlanks = true);

	/**
	 * peaks the next character to read, \0 if EOL
	 * @param SkipBlanks skip preceding blanks
	 * @return
	 */
	TCHAR Peak(bool SkipBlanks = true) const;

	/**
	 * read the next non-whitespace character, can throw EOL
	 * @param SkipBlanks skip the preceding blanks
	 * @return 
	 */
	TCHAR ReadChar(bool SkipBlanks = true);

	/**
	 * read a panel group '{ 1 2 3 }*'
	 *
	 * @throws FEOL if end of line is reached before the entire group specification has been read
	 * @throws FUnexpectedSymbolException if the panel group contains non-numeric indeces
	 * @throws FParsingException if '*' or '+' repeat specifiers are missing
	 * @return 
	 */
	FPanelGroup ReadPanelGroup();
	
	/**
	 * reads a rule for one building's side: ' 1 2 3 {1 2}+ 0 '
	 *
	 * @throws FEOL on unfinished groups
	 * @throws FUnexpectedSymbolException unexpected symbols in a panel group or in the rule
	 * @throws FParsingException if group repeat specifier is missing
	 * @return 
	 */
	FPBRule ReadRule();

	/**
	 * reads the rules into the ruleset
	 *
	 * @throws FParsingException
	 */
	void ReadRules();

	/**
	 * reads an override rule '3 > 4'
	 * @return 
	 */
	FPanelOverrideRule ReadOverrideRule();
	
	/**
	 * 
	 * @return 
	 */
	FPanelFloorOverrides ReadFloorOverrideGroup();
	
	/**
	 * reads the panel floor overrides
	 *
	 * @throws FParsingException
	 */
	void ReadFloorOverrides();
	
public:
	FBuildingGrammarParser(const FString& InGrammar) : Grammar(InGrammar), Cursor(0), Error("", -1) {}
	void Parse();
	FPBRuleSet GetRuleSet() const {return RuleSet;}
};

FString ToString(const FParsingError& ParsingError, const FString& Grammar);

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
	int FloorOffset {0};

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EPanelBuildingSide> Position {Front};

	/**
	 * assigned panel location and rotation
	 *
	 * the first panel will be positioned in front and on the left of the bounding box
	 *
	 * panels will be rotated depending on the side of the building they are on
	 */
	UPROPERTY(BlueprintReadOnly)
	FTransform AssignedTransform;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPBPanelLayout> PanelLayout {nullptr}; // a reference to the panel layout for this panel
};

/**
 * a structure used by the fitting algorithm
 *
 * it defines a group of one or more panels, to be placed zero or more times
 */
struct FPanelPlacement
{
	bool IsRepeatableGroup {false};				// it is a group of panels, to be repeated zero or more times
	TArray<UPBPanelLayout*> Panels;		// panels in this group
	int Repeat {0};							// how many times this group is repeated
};

UCLASS()
class PCGPANELHOUSE_API UPCGPanelBuildingHelpers : public UBlueprintFunctionLibrary
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
	static bool FitPanelsToBoundingBox(const FPBRuleSet& PanelHouseRuleSet, const FBox& BoundingBox, const TArray<UPBPanelLayout*>& Panels, TArray<FPositionedPanelInfo>& OutPanels);

	/**
	 * 
	 * same as above but also generates floors above 0
	 */
	UFUNCTION(BlueprintCallable)
	static bool FitPanelsToBoundingBox2(const FPBRuleSet& PanelHouseRuleSet,
		const FBox& TargetDimensions,
		const float BottomOffset,
		const float TopOffset,
		FBox& GeneratedDimensions,
		const TArray<UPBPanelLayout*>& Panels,
		TArray<FPositionedPanelInfo>& OutPanels);
	
	UFUNCTION(BlueprintCallable)
	static void ParseGrammar(const FString& Grammar, FPBRuleSet& OutRuleSet, bool& Success, FString& ErrorString);

	UFUNCTION(BlueprintCallable)
	static void GetReferencedIndices(const FPBRuleSet& RuleSet, TArray<int>& Indices);
};
