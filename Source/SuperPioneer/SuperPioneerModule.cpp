#include "SuperPioneerModule.h"
#include "Patching/NativeHookManager.h"
#include "SuperPioneerRemoteCallObject.h"
#include "FGGameMode.h"
#include "FGCharacterPlayer.h"
#include "FGCharacterMovementComponent.h"

void FSuperPioneerModule::StartupModule() {
	SPMovementComponentName = FName(TEXT("SuperPioneer.MovementComponent"));
	#if !WITH_EDITOR
	RegisterHooks();
	#endif
}

USuperPioneerMovementComponent* FSuperPioneerModule::GetMovementComponent(AActor* actor) {
	TArray<USuperPioneerMovementComponent*> components;
	actor->GetComponents<USuperPioneerMovementComponent>(components);
	for (int i = 0; i < components.Num(); i++) {
		return components[i];
	}
	return nullptr;
}

void FSuperPioneerModule::RegisterHooks() {

	AFGGameMode * exampleGameMode = GetMutableDefault<AFGGameMode>();

	SUBSCRIBE_METHOD_VIRTUAL(AFGGameMode::PostLogin, exampleGameMode, [](auto& scope, AFGGameMode* gm, APlayerController* pc) {
		UE_LOG(LogTemp, Warning, TEXT("[SP] >>>>>>>>>> POST LOGIN"))
		if (gm->HasAuthority() && !gm->IsMainMenuGameMode()) {
			UE_LOG(LogTemp, Warning, TEXT("[SP] >>>>>>>>>> REGISTERING RCO"))
			gm->RegisterRemoteCallObjectClass(USuperPioneerRemoteCallObject::StaticClass());
		}
	});

	AFGCharacterPlayer* examplePlayerCharacter = GetMutableDefault<AFGCharacterPlayer>();

	SUBSCRIBE_METHOD_AFTER(UConfigManager::MarkConfigurationDirty, [this](UConfigManager* self, const FConfigId& ConfigId) {
		if (ConfigId == FConfigId{ "SuperPioneer", "" } && IsValid(localSPMovementComponent)) {
			UE_LOG(LogTemp, Warning, TEXT("[SP] Config marked dirty, reloading...."))
			localSPMovementComponent->ReloadConfig();
		}
	})


	SUBSCRIBE_METHOD_VIRTUAL(AFGCharacterPlayer::SetupPlayerInputComponent, examplePlayerCharacter, [this](auto& scope, AFGCharacterPlayer* self, UInputComponent* PlayerInputComponent) {
		USuperPioneerMovementComponent* component = GetMovementComponent(self);
		if (!component) {
			// After each player's InputComponent is ready, setup the movement component and tie it to the player
			// Called after every spawn
			UE_LOG(LogTemp, Warning, TEXT("[SP] New player object created, starting component setup"))
			USuperPioneerMovementComponent* newComponent = NewObject<USuperPioneerMovementComponent>(self, SPMovementComponentName);
			newComponent->Setup(self, PlayerInputComponent);
			newComponent->RegisterComponent();
			if (self->IsLocallyControlled()) {
				localSPMovementComponent = newComponent;
			}
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
		USuperPioneerMovementComponent* component = GetMovementComponent(self);
		if (component) {
			if (component->CheckAndConsumeJump()) {
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
		USuperPioneerMovementComponent* component = GetMovementComponent(self);
		if (component) {
			component->OnLanded();
		}
	});
	SUBSCRIBE_METHOD_VIRTUAL(AFGCharacterBase::CalculateFallDamage, examplePlayerCharacter, [](auto& scope, const AFGCharacterBase* self, float zSpeed) {
		// Remove fall damage
		scope.Override((int32)0);
	});
}

IMPLEMENT_GAME_MODULE(FSuperPioneerModule, SuperPioneer);