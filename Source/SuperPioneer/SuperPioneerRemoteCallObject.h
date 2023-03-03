#pragma once

#include "CoreMinimal.h"
#include "FGRemoteCallObject.h"
#include "FGCharacterPlayer.h"
#include "SuperPioneerMovementComponent.h"
#include "SuperPioneerRemoteCallObject.generated.h"

UCLASS()
class SUPERPIONEER_API USuperPioneerRemoteCallObject : public UFGRemoteCallObject {
	GENERATED_BODY()

private:
	UFGCharacterMovementComponent* GetMovementComponent(AActor* actor);

	// Dummy property required for RCO to function, do not remove
	UPROPERTY(Replicated)
	bool bDummy = true;

public:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetSprintSpeed(AActor* player, float newMaxSprintSpeed);

};