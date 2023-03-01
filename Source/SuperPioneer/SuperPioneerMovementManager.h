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
	bool CheckAndConsumeJump();

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

	const float superJumpSpeedMultiplierMax = 2.0f;
	const float superJumpHoldMultiplierMax = 5.0f;
	const float superJumpHoldTimeMin = 0.1f;
	const float superJumpHoldTimeMax = 1.5f;
	const float maxAirControl = 0.6f;

	float defaultJumpZVelocity;
	float defaultAirControl;
	bool isJumpPressed;
	bool isJumpPrimed;
	float jumpHoldDuration;

	void JumpPressed();
	void JumpReleased();
	void JumpTick(float deltaTime);
	float CalculateJumpZVelocity();
	float CalculateCurrentJumpHoldPercentOfMax();
	void SetPlayerJumpZVelocity(float newZVelocity);
	float GetPlayerJumpZVelocity();
	float CalculateAirControl();
	void SetPlayerAirControl(float newAirControl);

	// Utilities
	static float lerp(float a, float b, float t);
};