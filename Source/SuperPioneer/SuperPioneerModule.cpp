#include "SuperPioneerModule.h"
#include "Patching/NativeHookManager.h"
#include "SuperPioneerRemoteCallObject.h"
#include "FGGameMode.h"
#include "FGCharacterPlayer.h"
#include "FGPlayerController.h"
#include "Animation/AnimInstance.h"
#include "Equipment/FGHoverPack.h"
#include "FGCharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

void FSuperPioneerModule::StartupModule() {
	#if !WITH_EDITOR
	UE_LOG(LogTemp, Warning, TEXT("[SP] Starting module"))
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

void FSuperPioneerModule::SetupMovementComponent(AFGCharacterPlayer* player, UInputComponent* inputComponent) {
	USuperPioneerMovementComponent* component = GetMovementComponent(player);
	if (!component) {
		// After each player's InputComponent is ready, setup the movement component and tie it to the player
		// Called after every spawn
		UE_LOG(LogTemp, Warning, TEXT("[SP] New player object created, starting component setup"))

		bool isHost = false;
		UWorld* world = player->GetWorld();
		AGameModeBase* gm = UGameplayStatics::GetGameMode(world);
		if (gm && gm->HasAuthority()) {
			isHost = true;
		}

		USuperPioneerMovementComponent* newComponent = NewObject<USuperPioneerMovementComponent>(player, SPMovementComponentName);
		newComponent->Setup(player, inputComponent, isHost);
		newComponent->RegisterComponent();

		if (player->IsLocallyControlled()) {
			if (localSPMovementComponent && IsValid(localSPMovementComponent)) {
				UE_LOG(LogTemp, Warning, TEXT("[SP] Destroying old component"))
				localSPMovementComponent->DestroyComponent();
			}
			localSPMovementComponent = newComponent;
		}
	}
}

void FSuperPioneerModule::RegisterHooks() {

	AFGCharacterPlayer::OnPlayerInputInitialized.AddRaw(this,&FSuperPioneerModule::SetupMovementComponent);

	AFGGameMode * exampleGameMode = GetMutableDefault<AFGGameMode>();

	SUBSCRIBE_METHOD_VIRTUAL(AFGGameMode::PostLogin, exampleGameMode, [this](auto& scope, AFGGameMode* gm, APlayerController* pc) {
		if (gm->HasAuthority() && !gm->IsMainMenuGameMode()) {
			// This machine is the host, register Remote Call Object
			UE_LOG(LogTemp, Warning, TEXT("[SP] Logged in as host, registering Remote Call Object"))
			gm->RegisterRemoteCallObjectClass(USuperPioneerRemoteCallObject::StaticClass());
		}
	});
	SUBSCRIBE_METHOD_AFTER(UConfigManager::MarkConfigurationDirty, [this](UConfigManager* self, const FConfigId& ConfigId) {
		if (ConfigId == FConfigId{ "SuperPioneer", "" } && IsValid(localSPMovementComponent)) {
			// The user config has been updated, reload it
			UE_LOG(LogTemp, Warning, TEXT("[SP] Config marked dirty, reloading...."))
			localSPMovementComponent->ReloadConfig();
		}
	});

	AFGCharacterPlayer* examplePlayerCharacter = GetMutableDefault<AFGCharacterPlayer>();

	SUBSCRIBE_METHOD(AFGCharacterPlayer::Input_Crouch, [this](auto& scope, AFGCharacterPlayer* self, const FInputActionValue& actionValue) {
		USuperPioneerMovementComponent* component = GetMovementComponent(self);
		if (component && component->AttemptGroundSlam()) {
			scope.Cancel();
		}
	});
	SUBSCRIBE_METHOD_VIRTUAL(AFGCharacterPlayer::Jump, examplePlayerCharacter, [this](auto& scope, AFGCharacterPlayer* self) {
		USuperPioneerMovementComponent* component = GetMovementComponent(self);
		if (component) {
			if (component->CheckAndConsumeJump()) {
				// Jump was primed, continue with jump
			} else {
				// Jump was not primed, cancel jump
				scope.Cancel();
			}
		}
	});
	SUBSCRIBE_METHOD(AFGCharacterPlayer::OnActiveEquipmentChangedInSlot, [this](auto& scope, AFGCharacterBase* self, EEquipmentSlot slot) {
		USuperPioneerMovementComponent* component = GetMovementComponent(self);
		if (component) {
			component->OnActiveEquipmentChanged();
		}
	});
	SUBSCRIBE_METHOD_AFTER(AFGCharacterPlayer::ToggleBuildGun, [this](AFGCharacterBase* self) {
		USuperPioneerMovementComponent* component = GetMovementComponent(self);
		if (component) {
			component->OnActiveEquipmentChanged();
		}
	});
	SUBSCRIBE_METHOD_AFTER(AFGCharacterPlayer::SetFirstPersonMode, [this](AFGCharacterBase* self) {
		USuperPioneerMovementComponent* component = GetMovementComponent(self);
		if (component) {
			component->SwitchCameraMode(ECameraMode::ECM_FirstPerson);
		}
	});
	SUBSCRIBE_METHOD_AFTER(AFGCharacterPlayer::SetThirdPersonMode, [this](AFGCharacterBase* self) {
		USuperPioneerMovementComponent* component = GetMovementComponent(self);
		if (component) {
			component->SwitchCameraMode(ECameraMode::ECM_ThirdPerson);
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
		AFGPlayerController* controller = Cast<AFGPlayerController>(self->Controller);
		if (controller) {
			RCO* rco = (RCO*)controller->GetRemoteCallObjectOfClass(RCO::StaticClass());
			if (rco && rco->GetFallDamageDisabled()) {
				scope.Override((int32)0);
			}
		}
	});

	AFGHoverPack* exampleHoverPack = GetMutableDefault<AFGHoverPack>();

	SUBSCRIBE_METHOD_VIRTUAL(AFGHoverPack::OnCharacterMovementModeChanged, exampleHoverPack, [this](auto& scope, const AFGHoverPack* self, EMovementMode PreviousMovementMode, uint8 PreviousCustomMode, EMovementMode NewMovementMode, uint8 NewCustomMode) {
		if (IsValid(localSPMovementComponent)) {
			localSPMovementComponent->CheckForHoverPackLand(PreviousMovementMode, PreviousCustomMode, NewMovementMode, NewCustomMode);
		}
	});

	SUBSCRIBE_METHOD_AFTER(AFGCharacterPlayer::OnPlayerCustomizationDataChanged, [this](AFGCharacterBase* self, const FPlayerCustomizationData& NewCustomizationData) {
		USuperPioneerMovementComponent* component = GetMovementComponent(self);
		if (component) {
			component->RefreshMesh1PVisibility();
		}
	});

}

IMPLEMENT_GAME_MODULE(FSuperPioneerModule, SuperPioneer);