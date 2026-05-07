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

struct HPanelBuildingBoundsControlHitProxy : public HComponentVisProxy
{
private:
	EPanelBuildingDimension ControlledDimension;

public:

	virtual EMouseCursor::Type GetMouseCursor() override;
	
	HPanelBuildingBoundsControlHitProxy(const UPanelBuildingBounds* BoundsComponent,
	                                    const EPanelBuildingDimension ControlledDimension) :
		HComponentVisProxy(BoundsComponent),
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
	UPanelBuildingBounds* EditedComponent {nullptr};
	EPanelBuildingDimension ActiveControl {None};
};

class FPanelBuildingBoundsVisualizer : public FComponentVisualizer
{
	FCachedVisualizerState State;
	
public:
	virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;
	virtual void TrackingStarted(FEditorViewportClient* InViewportClient) override;
	virtual void TrackingStopped(FEditorViewportClient* InViewportClient, bool bInDidMove) override;
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
