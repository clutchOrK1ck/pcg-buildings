#pragma once
#include "ComponentVisualizer.h"
#include "PanelBuildingBounds.h"

enum EPanelBuildingDimension : int
{
	None = -1,
	Width,
	Depth,
	Height
};

struct HPanelBuildingBoundsHitProxy : public HComponentVisProxy
{
	HPanelBuildingBoundsHitProxy(const UPanelBuildingBounds* BoundsComponent) : HComponentVisProxy(BoundsComponent) {}

	UPanelBuildingBounds* GetBoundsComponent() const
	{
		return Cast<UPanelBuildingBounds>(const_cast<UActorComponent*>(Component.Get()));
	}

	AActor* GetActor() const
	{
		return Component->GetOwner();
	}
};

struct HPanelBuildingBoundsControlHitProxy : HPanelBuildingBoundsHitProxy
{
private:
	EPanelBuildingDimension ControlledDimension;

public:

	virtual EMouseCursor::Type GetMouseCursor() override;
	
	HPanelBuildingBoundsControlHitProxy(const UPanelBuildingBounds* BoundsComponent,
	                                    const EPanelBuildingDimension ControlledDimension) :
		HPanelBuildingBoundsHitProxy(BoundsComponent),
		ControlledDimension(ControlledDimension)
	{
	}

	EPanelBuildingDimension GetControlledDimension() const
	{
		return ControlledDimension;
	}
};

struct FCachedVisualizerState
{
	AActor* ComponentOwningActor;
	UPanelBuildingBounds* VisualizedComponent {nullptr};
	EPanelBuildingDimension ActiveControl {None};
};

struct FEditedComponentCache
{
	AActor* EditedActor {nullptr};
	UPanelBuildingBounds* EditedComponent {nullptr};

	void Reset(UPanelBuildingBounds* Component)
	{
		EditedComponent = Component;
		EditedActor = Component->GetOwner();
	}
};

class FPanelBuildingBoundsVisualizer : public FComponentVisualizer
{
	FEditedComponentCache EditedComponentCache;
	EPanelBuildingDimension SelectedControl {None};
	
public:
	virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;
	virtual UActorComponent* GetEditedComponent() const override;
	virtual bool
	GetCustomInputCoordinateSystem(const FEditorViewportClient* ViewportClient, FMatrix& OutMatrix) const override;
	
	virtual bool HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate,
		FRotator& DeltaRotate, FVector& DeltaScale) override;

	virtual bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy,
	                                 const FViewportClick& Click) override;
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View,
	                               FPrimitiveDrawInterface* PDI) override;
};
