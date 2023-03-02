#include "SuperPioneerModule.h"
#include "Patching/NativeHookManager.h"
#include "FGCharacterPlayer.h"
#include "FGCharacterMovementComponent.h"

void FSuperPioneerModule::StartupModule() {
	SPMovementComponentName = FName(TEXT("SuperPioneer.MovementComponent"));
	#if !WITH_EDITOR
	RegisterHooks();
	#endif
}



void FSuperPioneerModule::RegisterHooks() {
	AFGCharacterPlayer* examplePlayerCharacter = GetMutableDefault<AFGCharacterPlayer>();

	SUBSCRIBE_METHOD_AFTER(UConfigManager::MarkConfigurationDirty, [this](UConfigManager* self, const FConfigId& ConfigId) {
		if (ConfigId == FConfigId{ "SuperPioneer", "" } && IsValid(SPMovementComponent)) {
			UE_LOG(LogTemp, Warning, TEXT("[SP] Config marked dirty, reloading...."))
			SPMovementComponent->ReloadConfig();
		}
	})

	SUBSCRIBE_METHOD_VIRTUAL(AFGCharacterPlayer::SetupPlayerInputComponent, examplePlayerCharacter, [this](auto& scope, AFGCharacterPlayer* self, UInputComponent* PlayerInputComponent) {
		if (self->IsLocallyControlled() && self != this->localPlayer) {
			// After the local player's InputComponent is ready, setup the movement component and tie it to the player
			// Called after every spawn
			UE_LOG(LogTemp, Warning, TEXT("[SP] New local player detected, updating pointer"))
			this->localPlayer = self;
			SPMovementComponent = NewObject<USuperPioneerMovementComponent>(self, SPMovementComponentName);
			SPMovementComponent->Setup(self, PlayerInputComponent);
			SPMovementComponent->RegisterComponent();
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
		if (self->IsLocallyControlled() && self == this->localPlayer && IsValid(SPMovementComponent)) {
			SPMovementComponent->Tick(deltaTime);
		}
	});*/

	SUBSCRIBE_METHOD_VIRTUAL(AFGCharacterPlayer::Jump, examplePlayerCharacter, [this](auto& scope, AFGCharacterPlayer* self) {
		if (self->IsLocallyControlled() && self == this->localPlayer && IsValid(SPMovementComponent)) {
			if (SPMovementComponent->CheckAndConsumeJump()) {
				// Jump was primed, continue with jump
			} else {
				// Jump was not primed, cancel jump
				if (!self->GetMovementComponent()->IsSwimming()) {
					scope.Cancel();
				}
			}
		}
	});
	SUBSCRIBE_METHOD_VIRTUAL(AFGCharacterBase::Landed, examplePlayerCharacter, [this](auto& scope, AFGCharacterBase* self, const FHitResult& Hit) {
		if (self->IsLocallyControlled() && self == this->localPlayer && IsValid(SPMovementComponent)) {
			SPMovementComponent->OnLanded();
		}
	});
	SUBSCRIBE_METHOD_VIRTUAL(AFGCharacterBase::CalculateFallDamage, examplePlayerCharacter, [](auto& scope, const AFGCharacterBase* self, float zSpeed) {
		// Remove fall damage
		scope.Override((int32)0);
	});
}

IMPLEMENT_GAME_MODULE(FSuperPioneerModule, SuperPioneer);