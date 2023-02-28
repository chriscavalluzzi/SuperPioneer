#pragma once

#include "Modules/ModuleManager.h"
#include "GameFramework/Character.h"
#include "FGCharacterPlayer.h"
#include "SuperPioneerMovementManager.h"

class FSuperPioneerModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;
	virtual bool IsGameModule() const override { return true; }

private:

	AFGCharacterPlayer* localPlayer;

	void RegisterHooks();

	USuperPioneerMovementManager* movementManager;
	FName movementManagerName;

};