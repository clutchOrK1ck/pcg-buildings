#include "SizeBoxEd.h"

#include "BoxBounds.h"
#include "FBoxBoundsVisualizer.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"

#define LOCTEXT_NAMESPACE "FSizeBoxEdModule"

void FSizeBoxEdModule::StartupModule()
{
    // register the component visualizer for the box bounds
	if (GUnrealEd)
	{
		UE_LOG(LogTemp, Display, TEXT("Registering component visualizer for Box Bounds"));
		GUnrealEd->RegisterComponentVisualizer(
			UBoxBounds::StaticClass()->GetFName(),
			MakeShareable(new FBoxBoundsVisualizer)
		);
	}
}

void FSizeBoxEdModule::ShutdownModule()
{
    if (GUnrealEd)
    {
	    GUnrealEd->UnregisterComponentVisualizer(UBoxBounds::StaticClass()->GetFName());
    }
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FSizeBoxEdModule, SizeBoxEd)