#include "SuperPioneerModule.h"
#include "Patching/NativeHookManager.h"
#include "FGCharacterPlayer.h"
#include "FGCharacterMovementComponent.h"

void FSuperPioneerModule::StartupModule() {
	movementManagerName = FName(TEXT("SuperPioneer.MovementManager"));
	#if !WITH_EDITOR
	RegisterHooks();
	#endif
}

void FSuperPioneerModule::RegisterHooks() {
	AFGCharacterPlayer* examplePlayerCharacter = GetMutableDefault<AFGCharacterPlayer>();

	SUBSCRIBE_METHOD_VIRTUAL(AFGCharacterPlayer::SetupPlayerInputComponent, examplePlayerCharacter, [this](auto& scope, AFGCharacterPlayer* self, UInputComponent* PlayerInputComponent) {
		if (self->IsLocallyControlled() && self != this->localPlayer) {
			// After local player's InputComponent is ready, setup manager and tie to player
			// Called after every spawn
			UE_LOG(LogTemp, Warning, TEXT("[SP] New local player detected, updating pointer"))
			this->localPlayer = self;
			movementManager = NewObject<USuperPioneerMovementManager>(self, movementManagerName);
			movementManager->Setup(self, PlayerInputComponent);
		}
	});

	/*SUBSCRIBE_METHOD_VIRTUAL(AFGCharacterPlayer::SprintPressed, examplePlayerCharacter, [this](auto& scope, AFGCharacterPlayer* self) {
		UE_LOG(LogTemp, Warning, TEXT("[SP] Sprinting"))
		this->SetPlayerSprintSpeed(self, this->defaultMaxSprintSpeed);
		this->sprintDuration = 0.0;
	});*/
	/*SUBSCRIBE_METHOD_VIRTUAL(AFGCharacterPlayer::SprintReleased, examplePlayerCharacter, [this](auto& scope, AFGCharacterPlayer* self) {
		UE_LOG(LogTemp, Warning, TEXT("[SP] Stopping Sprinting"))
		this->SetPlayerSprintSpeed(self, this->defaultMaxSprintSpeed);
	});*/
	/*SUBSCRIBE_METHOD_VIRTUAL(AFGCharacterPlayer::Tick, examplePlayerCharacter, [this](auto& scope, AFGCharacterPlayer* self, float deltaTime) {
		if (self->IsLocallyControlled() && self == this->localPlayer && IsValid(movementManager)) {
			movementManager->Tick(deltaTime);
		}
	});*/

	SUBSCRIBE_METHOD_VIRTUAL(AFGCharacterPlayer::Jump, examplePlayerCharacter, [this](auto& scope, AFGCharacterPlayer* self) {
		if (self->IsLocallyControlled() && self == this->localPlayer && IsValid(movementManager)) {
			if (movementManager->CheckAndConsumeJump()) {
				UE_LOG(LogTemp, Warning, TEXT("[SP] JUMP PASSED"))
				// Jump was primed, continue with jump
			} else {
				UE_LOG(LogTemp, Warning, TEXT("[SP] JUMP CANCELLED"))
				// Jump was not primed, cancel jump
				scope.Cancel();
			}
		}
	});
	SUBSCRIBE_METHOD_VIRTUAL(AFGCharacterBase::CalculateFallDamage, examplePlayerCharacter, [](auto& scope, const AFGCharacterBase* self, float zSpeed) {
		// Remove fall damage
		scope.Override((int32)0);
	});
}

IMPLEMENT_GAME_MODULE(FSuperPioneerModule, SuperPioneer);