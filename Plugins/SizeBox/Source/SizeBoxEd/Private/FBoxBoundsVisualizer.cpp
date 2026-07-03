#include "FBoxBoundsVisualizer.h"
#include "BoxBounds.h"
#include "SceneManagement.h"

EMouseCursor::Type HPanelBuildingBoundsControlHitProxy::GetMouseCursor()
{
	return EMouseCursor::Type::Hand;
}

FVector GetPanelBuildingControlLocation(const UBoxBounds* BoundsComponent,
										const EPanelBuildingDimension Dimension)
{
	// one of these might be null when recompiling blueprints and such
	if (!(BoundsComponent && BoundsComponent->GetOwner()))
	{
		return FVector::ZeroVector;
	}

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

bool FBoxBoundsVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient,
                                                       FVector& OutLocation) const
{
	if (SelectedControl != None && GetEditedComponent() != nullptr)
	{
		OutLocation = GetPanelBuildingControlLocation(Cast<UBoxBounds>(GetEditedComponent()), SelectedControl);
		return true;
	}
	
	return false;
}

UActorComponent* FBoxBoundsVisualizer::GetEditedComponent() const
{
	return EditedComponentCache.EditedComponent;
}

bool FBoxBoundsVisualizer::GetCustomInputCoordinateSystem(const FEditorViewportClient* ViewportClient,
	FMatrix& OutMatrix) const
{
	if (EditedComponentCache.EditedComponent && EditedComponentCache.EditedComponent->GetOwner())
	{
		OutMatrix = FRotationMatrix(EditedComponentCache.EditedComponent->GetOwner()->GetActorRotation());
		return true;
	}

	return false;
}

bool FBoxBoundsVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport,
                                                      FVector& DeltaTranslate, FRotator& DeltaRotate,
                                                      FVector& DeltaScale)
{
	if (!EditedComponentCache.EditedComponent || SelectedControl == None) return false;

	// find local delta transalte (we are supplied with a delta translate in world space)
	auto LocalTranslate = EditedComponentCache.EditedComponent->GetOwner()->GetActorRotation().GetInverse().RotateVector(DeltaTranslate);
	
	switch(SelectedControl)
	{
	case Width:
		EditedComponentCache.EditedComponent->Expand(0, LocalTranslate.Y);
		return true;
	case Depth:
		EditedComponentCache.EditedComponent->Expand(1, -LocalTranslate.X);
		return true;
	default:
		EditedComponentCache.EditedComponent->Expand(2, LocalTranslate.Z);
		return true;
	}
}

bool FBoxBoundsVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient,
                                                         HComponentVisProxy* VisProxy,
                                                         const FViewportClick& Click)
{
	if (VisProxy)
	{
		if (VisProxy->IsA(HPanelBuildingBoundsControlHitProxy::StaticGetType()))
		{
			auto Proxy = static_cast<HPanelBuildingBoundsControlHitProxy*>(VisProxy);
			EditedComponentCache.Reset(Proxy->GetBoundsComponent());
			SelectedControl = Proxy->GetControlledDimension();
			return true;
		}

		if (VisProxy->IsA(HPanelBuildingBoundsHitProxy::StaticGetType()))
		{
			auto Proxy = static_cast<HPanelBuildingBoundsHitProxy*>(VisProxy);
			EditedComponentCache.Reset(Proxy->GetBoundsComponent());
			SelectedControl = None;
			return true;
		}

		return false;
	}
	
	return false;
}

void FBoxBoundsVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View,
                                                       FPrimitiveDrawInterface* PDI)
{
	if (Component == nullptr) return;
	if (Component->GetClass() != UBoxBounds::StaticClass()) return;
	auto BoundariesComponent = Cast<UBoxBounds>(const_cast<UActorComponent*>(Component));
	
	if (EditedComponentCache.EditedActor != BoundariesComponent->GetOwner())
	{
		SelectedControl = None;
		EditedComponentCache.Reset(BoundariesComponent);
	}
	
	FBox LocalBounds = BoundariesComponent->GetBounds();

	FMatrix GlobalTransform = Component->GetOwner() != nullptr && Component->GetOwner()->IsA<AActor>()
		                          ? Component->GetOwner()->GetTransform().ToMatrixNoScale()
		                          : FMatrix();

	// bounding box
	PDI->SetHitProxy(new HPanelBuildingBoundsHitProxy(BoundariesComponent));
	DrawWireBox(PDI, GlobalTransform, LocalBounds, FLinearColor::Red, 0);

	// draw bounding box controls
	PDI->SetHitProxy(new HPanelBuildingBoundsControlHitProxy(BoundariesComponent, Width));
	PDI->DrawPoint(GetPanelBuildingControlLocation(BoundariesComponent, Width), FColor::White, 10., 0);

	PDI->SetHitProxy(new HPanelBuildingBoundsControlHitProxy(BoundariesComponent, Depth));
	PDI->DrawPoint(GetPanelBuildingControlLocation(BoundariesComponent, Depth), FColor::White, 10., 0);

	PDI->SetHitProxy(new HPanelBuildingBoundsControlHitProxy(BoundariesComponent, Height));
	PDI->DrawPoint(GetPanelBuildingControlLocation(BoundariesComponent, Height), FColor::White, 10., 0);

	PDI->SetHitProxy(nullptr);
}
