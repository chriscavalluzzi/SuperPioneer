#pragma once

#include "Modules/ModuleManager.h"
#include "GameFramework/Character.h"
#include "FGCharacterPlayer.h"

class FSuperPioneerModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;
	virtual bool IsGameModule() const override { return true; }

private:
	const float superSprintMaxSpeed = 10000.0;
	
	//AFGCharacterPlayer* PlayerCharacter;
	
	void RegisterHooks();

	bool isSprintPressed;
	float sprintDuration;
	float defaultMaxSprintSpeed;

	void SprintTick(AFGCharacterPlayer* player);
	void SprintDurationTick(AFGCharacterPlayer* player, float deltaTime);
	float CalculateSprintSpeed(float duration);
	void SetPlayerSprintSpeed(AFGCharacterPlayer* player, float newSprintSpeed);
	float GetPlayerSprintSpeed(AFGCharacterPlayer* player);
	bool GetIsPlayerSprinting(AFGCharacterPlayer* player);
};