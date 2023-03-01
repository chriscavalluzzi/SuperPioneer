#pragma once

#include "FGCharacterPlayer.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SuperPioneerMovementManager.generated.h"

UCLASS()
class SUPERPIONEER_API USuperPioneerMovementManager : public UActorComponent
{
	GENERATED_BODY()

public:

	USuperPioneerMovementManager();
	void Setup(AFGCharacterPlayer* _localPlayer, UInputComponent* _inputComponent);
	void Jump();

protected:

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

private:

	// General

	AFGCharacterPlayer* localPlayer;
	const char* superSprintCommandName = "SuperPioneer.SuperSprint";

	AFGCharacterPlayer* GetPlayer();
	UFGCharacterMovementComponent* GetPlayerMovementComponent();

	// Sprinting

	const float superSprintMaxSpeed = 10000.0f;

	bool isSuperSprintPressed;
	bool wasSprintingBeforeSuperSprint;
	bool wasHoldingToSprintBeforeSuperSprint;
	float sprintDuration;
	float defaultMaxSprintSpeed;

	void SuperSprintPressed();
	void SuperSprintReleased();
	void SprintTick(float deltaTime);
	float CalculateSprintSpeed(float duration);
	void SetPlayerSprintSpeed(float newSprintSpeed);
	float GetPlayerCurrentSprintSpeed();
	bool GetIsPlayerSprinting();
	float CalculateCurrentSpeedPercentOfMax();

	// Jumping

	const float superJumpMaxZVelocityMultiplier = 7.0f;
	const float superJumpMinZVelocityMultiplier = 3.0f;
	const float maxAirControl = 1.0f;

	float defaultJumpZVelocity;
	float defaultAirControl;

	float CalculateJumpZVelocity(float heldDuration);
	void SetPlayerJumpZVelocity(float newZVelocity);
	float GetPlayerJumpZVelocity();
	float CalculateAirControl();
	void SetPlayerAirControl(float newAirControl);

};