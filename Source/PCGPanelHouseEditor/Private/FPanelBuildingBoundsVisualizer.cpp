#include "FPanelBuildingBoundsVisualizer.h"
#include "PanelBuildingBounds.h"
#include "SceneManagement.h"

EMouseCursor::Type HPanelBuildingBoundsControlHitProxy::GetMouseCursor()
{
	return EMouseCursor::Type::Hand;
}

FVector GetPanelBuildingControlLocation(const UPanelBuildingBounds* BoundsComponent,
										const EPanelBuildingDimension Dimension)
{
	const auto ActorTransform = BoundsComponent->GetOwner()->GetTransform();

	switch (Dimension)
	{
	case Width:
		return ActorTransform.TransformPosition({
			-BoundsComponent->Depth / 2.,
			BoundsComponent->Width + 100.,
			BoundsComponent->Height / 2.
		});
	case Depth:
		return ActorTransform.TransformPosition({
			-BoundsComponent->Depth - 100.f,
			BoundsComponent->Width / 2.,
			BoundsComponent->Height / 2.
		});
	default:
		return ActorTransform.TransformPosition({
			-BoundsComponent->Depth / 2.,
			BoundsComponent->Width / 2.,
			BoundsComponent->Height + 100.
		});
	}
}

bool FPanelBuildingBoundsVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient,
                                                       FVector& OutLocation) const
{
	if (this->State.ActiveControl != None && GetEditedComponent() != nullptr)
	{
		OutLocation = GetPanelBuildingControlLocation(Cast<UPanelBuildingBounds>(GetEditedComponent()), this->State.ActiveControl);
		return true;
	}

	return false;
}

UActorComponent* FPanelBuildingBoundsVisualizer::GetEditedComponent() const
{
	return this->State.EditedComponent;
}

bool FPanelBuildingBoundsVisualizer::GetCustomInputCoordinateSystem(const FEditorViewportClient* ViewportClient,
	FMatrix& OutMatrix) const
{
	if (this->State.EditedComponent != nullptr)
	{
		OutMatrix = FRotationMatrix(this->State.EditedComponent->GetOwner()->GetActorRotation());
		return true;
	}

	return false;
}

bool FPanelBuildingBoundsVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport,
                                                      FVector& DeltaTranslate, FRotator& DeltaRotate,
                                                      FVector& DeltaScale)
{
	if (!GetEditedComponent() || State.ActiveControl == None) return false;

	auto BoundsComponent = Cast<UPanelBuildingBounds>(GetEditedComponent());

	// find local delta transalte (we are supplied with a delta translate in world space)
	auto LocalTranslate = BoundsComponent->GetOwner()->GetActorRotation().GetInverse().RotateVector(DeltaTranslate);
	UE_LOG(LogTemp, Display, TEXT("Handling translate delta: %s"), *LocalTranslate.ToString());
	
	switch(State.ActiveControl)
	{
	case Width:
		BoundsComponent->Width += LocalTranslate.Y;
		return true;
	case Depth:
		BoundsComponent->Depth -= LocalTranslate.X;
		return true;
	default:
		BoundsComponent->Height += LocalTranslate.Z;
		return true;
	}
}

bool FPanelBuildingBoundsVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient,
                                                         HComponentVisProxy* VisProxy,
                                                         const FViewportClick& Click)
{
	if (VisProxy != nullptr && VisProxy->IsA(HPanelBuildingBoundsControlHitProxy::StaticGetType()))
	{
		this->State.ActiveControl = static_cast<HPanelBuildingBoundsControlHitProxy*>(VisProxy)->GetControlledDimension();
		this->State.EditedComponent = Cast<UPanelBuildingBounds>(
			const_cast<UActorComponent*>(VisProxy->Component.Get())
		);
	}

	return true;
}

void FPanelBuildingBoundsVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View,
                                                       FPrimitiveDrawInterface* PDI)
{
	if (Component == nullptr) return;
	if (Component->GetClass() != UPanelBuildingBounds::StaticClass()) return;

	const auto BuildingBoundsComponent = Cast<UPanelBuildingBounds>(Component);
	FBox LocalBounds = BuildingBoundsComponent->GetBounds();

	FMatrix GlobalTransform = Component->GetOwner() != nullptr && Component->GetOwner()->IsA<AActor>()
		                          ? Component->GetOwner()->GetTransform().ToMatrixNoScale()
		                          : FMatrix();

	// bounding box
	DrawWireBox(PDI, GlobalTransform, LocalBounds, FLinearColor::Red, 0);

	// draw bounding box controls
	PDI->SetHitProxy(new HPanelBuildingBoundsControlHitProxy(BuildingBoundsComponent, Width));
	PDI->DrawPoint(GetPanelBuildingControlLocation(BuildingBoundsComponent, Width), FColor::White, 10., 0);

	PDI->SetHitProxy(new HPanelBuildingBoundsControlHitProxy(BuildingBoundsComponent, Depth));
	PDI->DrawPoint(GetPanelBuildingControlLocation(BuildingBoundsComponent, Depth), FColor::White, 10., 0);

	PDI->SetHitProxy(new HPanelBuildingBoundsControlHitProxy(BuildingBoundsComponent, Height));
	PDI->DrawPoint(GetPanelBuildingControlLocation(BuildingBoundsComponent, Height), FColor::White, 10., 0);

	PDI->SetHitProxy(nullptr);
}
