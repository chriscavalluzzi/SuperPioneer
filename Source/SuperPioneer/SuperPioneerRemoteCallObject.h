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
	UFGCharacterMovementComponent* GetMovementComponent(AFGCharacterPlayer* actor);

	// Dummy property required for RCO to function, do not remove
	UPROPERTY(Replicated)
	bool bDummy = true;

	UPROPERTY(Replicated)
	bool fallDamageDisabled = true;

public:

	// Property Updates

	UFUNCTION(Server, Reliable)
	void ServerSetSprintSpeed(AFGCharacterPlayer* player, float newMaxSprintSpeed);

	UFUNCTION(Server, Reliable)
	void ServerSetMaxStepHeight(AFGCharacterPlayer* player, float newMaxStepHeight);

	UFUNCTION(Server, Reliable)
	void ServerSetJumpZVelocity(AFGCharacterPlayer* player, float newZVelocity);

	UFUNCTION(Server, Reliable)
	void ServerSetAirControl(AFGCharacterPlayer* player, float newAirControl);

	UFUNCTION(Server, Reliable)
	void ServerSetGravityScale(AFGCharacterPlayer* player, float newGravityScale);

	UFUNCTION(Server, Reliable)
	void ServerSetFallDamageDisabled(bool newFallDamageDisabled);

	// Actions
	UFUNCTION(Server, Reliable)
	void ServerSprintPressed(AFGCharacterPlayer* player);

	UFUNCTION(Server, Reliable)
	void ServerSprintReleased(AFGCharacterPlayer* player);

	UFUNCTION(Server, Reliable)
	void ServerDoJump(AFGCharacterPlayer* player);

	bool GetFallDamageDisabled();
};