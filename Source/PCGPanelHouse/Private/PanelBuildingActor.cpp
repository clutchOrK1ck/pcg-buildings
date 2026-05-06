// Fill out your copyright notice in the Description page of Project Settings.


#include "PanelBuildingActor.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "PBGrammar.h"
#include "Algo/AnyOf.h"


// Sets default values
APanelBuildingActor::APanelBuildingActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PCG = CreateDefaultSubobject<UPCGComponent>("PCG");
}

void APanelBuildingActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	FString ValidationError;
	if (!Grammar.IsEmpty() && IsValidPanelBuildingConfig(ValidationError))
	{
		if (PCG->GetGraph() != nullptr)
		{
			PCG->GenerateLocal(true);
		}
	} else
	{
		UE_LOG(LogTemp, Error, TEXT("PCG Panel building will not be generated because if an invalid config: %s"), *ValidationError);
	}
}

void APanelBuildingActor::Validate()
{
	FString ValidationError;
	
	if (const bool ValidConfig = IsValidPanelBuildingConfig(ValidationError); ValidConfig)
	{
		FNotificationInfo SuccessNotification{INVTEXT("Successfully validated")};

		SuccessNotification.ExpireDuration = 3.f;
		SuccessNotification.bUseSuccessFailIcons = true;
		
		auto NotificationWidget = FSlateNotificationManager::Get().AddNotification(SuccessNotification);
		NotificationWidget->SetCompletionState(SNotificationItem::CS_Success);
	} else
	{
		FMessageLog ErrorLog {FName{"PCGPanelBuildings"}};
		
		ErrorLog.Error(FText::FromString(ValidationError));
		ErrorLog.Open();
	}
}

// Called when the game starts or when spawned
void APanelBuildingActor::BeginPlay()
{
	Super::BeginPlay();
	
}

bool APanelBuildingActor::IsValidPanelBuildingConfig(FString& OutErrorMessage) const
{
	// catch invalid grammars first
	if (!Grammar.IsEmpty())
	{
		FPBRuleSet RuleSet;
		FParsingError ParsingError;
		
		ParsePBGrammar(this->Grammar, RuleSet, ParsingError);

		if (!ParsingError.ErrorMessage.IsEmpty())
		{
			OutErrorMessage = ToString(ParsingError, Grammar);
			return false;
		}
	}

	if (!Grammar.IsEmpty() && (PanelConfig.IsEmpty() || Algo::AnyOf(PanelConfig, [](const UPBPanelLayout* PanelLayout)
	{
		return PanelLayout == nullptr;
	})))
	{
		OutErrorMessage = "Panel configuration is empty or contains empty members";
		return false;
	}
	
	return true;
}

// Called every frame
void APanelBuildingActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

