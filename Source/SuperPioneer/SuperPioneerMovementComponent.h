#pragma once

#include "FGCharacterPlayer.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SuperPioneerMovementComponent.generated.h"

UCLASS()
class SUPERPIONEER_API USuperPioneerMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	USuperPioneerMovementComponent();
	void Setup(AFGCharacterPlayer* _localPlayer, UInputComponent* _inputComponent);
	bool CheckAndConsumeJump();
	void OnLanded();

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
	const float gravityScalingMultiplier = 0.5f;
	const float jumpMultiplierPerGravityScale = 6.0f;

	float defaultJumpZVelocity;
	float defaultAirControl;
	float defaultGravityScale;
	bool isJumpPressed;
	bool isJumpPrimed;
	float jumpHoldDuration;

	void JumpPressed();
	void JumpReleased();
	void JumpTick(float deltaTime);
	float CalculateJumpZVelocity();
	float CalculateJumpMultipliers();
	float CalculateCurrentJumpHoldPercentOfMax();
	void SetPlayerJumpZVelocity(float newZVelocity);
	float GetPlayerJumpZVelocity();
	float CalculateAirControl();
	void SetPlayerAirControl(float newAirControl);
	void SetPlayerGravityScale(float newGravityScale);

	// Utilities

	static float lerp(float a, float b, float t);
};