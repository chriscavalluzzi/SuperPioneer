#pragma once

#include "Modules/ModuleManager.h"
#include "GameFramework/Character.h"
#include "FGCharacterPlayer.h"

class FSuperPioneerModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;
	virtual bool IsGameModule() const override { return true; }

private:
	
	void RegisterHooks();

	// Sprinting

	const float superSprintMaxSpeed = 10000.0f;

	bool isSprintPressed;
	float sprintDuration;
	float defaultMaxSprintSpeed;

	void SprintTick(AFGCharacterPlayer* player);
	void SprintDurationTick(AFGCharacterPlayer* player, float deltaTime);
	float CalculateSprintSpeed(float duration);
	void SetPlayerSprintSpeed(AFGCharacterPlayer* player, float newSprintSpeed);
	float GetPlayerSprintSpeed(AFGCharacterPlayer* player);
	bool GetIsPlayerSprinting(AFGCharacterPlayer* player);

	// Jumping

	const float superJumpMaxZVelocityMultiplier = 7.0f;
	const float superJumpMinZVelocityMultiplier = 3.0f;

	float defaultJumpZVelocity;

	float CalculateJumpZVelocity(AFGCharacterPlayer* player, float heldDuration);
	void SetPlayerJumpZVelocity(AFGCharacterPlayer* player, float newZVelocity);
	float GetPlayerJumpZVelocity(AFGCharacterPlayer* player);
};