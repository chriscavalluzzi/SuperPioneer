#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SuperPioneerHUD.generated.h"

UCLASS()
class SUPERPIONEER_API USuperPioneerHUD : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent)
	void ShowJumpChargeIndicator();

	UFUNCTION(BlueprintImplementableEvent)
	void HideJumpChargeIndicator();
	
	UFUNCTION(BlueprintImplementableEvent)
	void SetJumpChargeIndicatorValue(float value);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowGroundSlamIndicator();

	UFUNCTION(BlueprintImplementableEvent)
	void HideGroundSlamIndicator();

	UFUNCTION(BlueprintImplementableEvent)
	void ActivateGroundSlam();
};
