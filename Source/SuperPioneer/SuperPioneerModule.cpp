#include "SuperPioneerModule.h"
#include "Patching/NativeHookManager.h"
#include "FGCharacterPlayer.h"
#include "FGCharacterMovementComponent.h"

/*
bool CanJump() const;

Maybe need to do a check inside each subscribe to see if self == playercharacter? for multiplayer
*/

void FSuperPioneerModule::StartupModule() {
	isSprintPressed = false;
	sprintDuration = 0.0;
	#if !WITH_EDITOR
	defaultMaxSprintSpeed = GetDefault<AFGCharacterPlayer>()->GetFGMovementComponent()->mMaxSprintSpeed;
	RegisterHooks();
	#endif
}

void FSuperPioneerModule::RegisterHooks() {
	AFGCharacterPlayer* examplePlayerCharacter = GetMutableDefault<AFGCharacterPlayer>();
	SUBSCRIBE_METHOD_VIRTUAL(AFGCharacterPlayer::Jump, examplePlayerCharacter, [](auto& scope, AFGCharacterPlayer* self) {
		UE_LOG(LogTemp, Warning, TEXT("[SP] Jumping"))
	});
	SUBSCRIBE_METHOD_VIRTUAL(AFGCharacterPlayer::SprintPressed, examplePlayerCharacter, [this](auto& scope, AFGCharacterPlayer* self) {
		UE_LOG(LogTemp, Warning, TEXT("[SP] Sprinting"))
			this->SetPlayerSprintSpeed(self, this->defaultMaxSprintSpeed);
			this->sprintDuration = 0.0;
	});
	SUBSCRIBE_METHOD_VIRTUAL(AFGCharacterPlayer::SprintReleased, examplePlayerCharacter, [this](auto& scope, AFGCharacterPlayer* self) {
		UE_LOG(LogTemp, Warning, TEXT("[SP] Stopping Sprinting"))
		this->SetPlayerSprintSpeed(self, this->defaultMaxSprintSpeed);
	});
	SUBSCRIBE_METHOD_VIRTUAL(AFGCharacterPlayer::Tick, examplePlayerCharacter, [this](auto& scope, AFGCharacterPlayer* self, float deltaTime) {
		this->SprintDurationTick(self, deltaTime);
		this->SprintTick(self);
	});
}

void FSuperPioneerModule::SprintTick(AFGCharacterPlayer* player) {
	if (GetIsPlayerSprinting(player)) {
		UE_LOG(LogTemp, Warning, TEXT("[SP] Attempting to Set Speed... %f"), CalculateSprintSpeed(sprintDuration))
		SetPlayerSprintSpeed(player, CalculateSprintSpeed(sprintDuration));
		UE_LOG(LogTemp, Warning, TEXT("[SP] Final Speed... %f"), GetPlayerSprintSpeed(player))
	}
}

void FSuperPioneerModule::SprintDurationTick(AFGCharacterPlayer* player, float deltaTime) {
	UE_LOG(LogTemp, Warning, TEXT("[SP] Passed player sprinting?... %s"), (GetIsPlayerSprinting(player) ? TEXT("true") : TEXT("false")))
	if (GetIsPlayerSprinting(player)) {
		sprintDuration += deltaTime;
		UE_LOG(LogTemp, Warning, TEXT("[SP] Sprint duration... %f"), sprintDuration)
	}
}

float FSuperPioneerModule::CalculateSprintSpeed(float duration) {
	return std::min(float((1 + (2.0 * pow(duration, 2))) * defaultMaxSprintSpeed), superSprintMaxSpeed);
}

void FSuperPioneerModule::SetPlayerSprintSpeed(AFGCharacterPlayer* player, float newSprintSpeed) {
	player->GetFGMovementComponent()->mMaxSprintSpeed = newSprintSpeed;
}

float FSuperPioneerModule::GetPlayerSprintSpeed(AFGCharacterPlayer* player) {
	return player->GetFGMovementComponent()->mMaxSprintSpeed;
}

bool FSuperPioneerModule::GetIsPlayerSprinting(AFGCharacterPlayer* player) {
	return player->GetFGMovementComponent()->GetIsSprinting();
}

IMPLEMENT_GAME_MODULE(FSuperPioneerModule, SuperPioneer);