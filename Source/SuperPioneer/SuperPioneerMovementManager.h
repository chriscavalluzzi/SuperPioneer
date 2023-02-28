#pragma once

#include "FGCharacterPlayer.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SuperPioneerMovementManager.generated.h"

UCLASS()
class SUPERPIONEER_API USuperPioneerMovementManager : public UObject
{
	GENERATED_BODY()

	AFGCharacterPlayer* localPlayer;
	UInputComponent* inputComponent;

public:
	
	// General
	
	USuperPioneerMovementManager();

	void Setup(AFGCharacterPlayer* _localPlayer, UInputComponent* _inputComponent);
	void Tick(float deltaTime);
	UFGCharacterMovementComponent* USuperPioneerMovementManager::GetPlayerMovementComponent();

	// Sprinting

	const float superSprintMaxSpeed = 10000.0f;

	bool isSuperSprintPressed;
	float sprintDuration;
	float defaultMaxSprintSpeed;

	void SuperSprintPressed();
	void SuperSprintReleased();
	void SprintTick(float deltaTime);
	float CalculateSprintSpeed(float duration);
	void SetPlayerSprintSpeed(float newSprintSpeed);
	float GetPlayerCurrentSprintSpeed();
	bool GetIsPlayerSprinting();

	// Jumping

	const float superJumpMaxZVelocityMultiplier = 7.0f;
	const float superJumpMinZVelocityMultiplier = 3.0f;

	float defaultJumpZVelocity;

	float CalculateJumpZVelocity(AFGCharacterPlayer* player, float heldDuration);
	void SetPlayerJumpZVelocity(AFGCharacterPlayer* player, float newZVelocity);
	float GetPlayerJumpZVelocity(AFGCharacterPlayer* player);

};