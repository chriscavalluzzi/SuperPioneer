#pragma once

#include "Modules/ModuleManager.h"
#include "GameFramework/Character.h"
#include "FGCharacterPlayer.h"
#include "SuperPioneerMovementComponent.h"

class FSuperPioneerModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;
	virtual bool IsGameModule() const override { return true; }

private:

	const FName SPMovementComponentName = FName(TEXT("SuperPioneer.MovementComponent"));
	USuperPioneerMovementComponent* localSPMovementComponent;

	void SetupMovementComponent(AFGCharacterPlayer* player, UInputComponent* inputComponent);
	void RegisterHooks();
	USuperPioneerMovementComponent* GetMovementComponent(AActor* actor);

};