#include "SuperPioneerMovementManager.h"
#include "FGCharacterPlayer.h"
#include "FGCharacterMovementComponent.h"

USuperPioneerMovementManager::USuperPioneerMovementManager() {
  UE_LOG(LogTemp, Warning, TEXT("[SP] Sprint Manager Construction"))
};

void USuperPioneerMovementManager::Setup(AFGCharacterPlayer* _localPlayer, UInputComponent* _inputComponent) {
  UE_LOG(LogTemp,Warning,TEXT("[SP] Starting Manager Setup"))
  this->localPlayer = _localPlayer;
  this->inputComponent = _inputComponent;
	defaultMaxSprintSpeed = GetPlayerMovementComponent()->mMaxSprintSpeed;
	defaultJumpZVelocity = GetPlayerMovementComponent()->JumpZVelocity;
	isSuperSprintPressed = false;
	sprintDuration = 0.0;
  _inputComponent->BindAction("SuperPioneer.SuperSprint", EInputEvent::IE_Pressed, this, &USuperPioneerMovementManager::SuperSprintPressed);
	_inputComponent->BindAction("SuperPioneer.SuperSprint", EInputEvent::IE_Released, this, &USuperPioneerMovementManager::SuperSprintReleased);
}

void USuperPioneerMovementManager::Tick(float deltaTime) {
	SprintTick(deltaTime);
}

UFGCharacterMovementComponent* USuperPioneerMovementManager::GetPlayerMovementComponent() {
	return this->localPlayer->GetFGMovementComponent();
}

// Sprinting

void USuperPioneerMovementManager::SuperSprintPressed() {
	UE_LOG(LogTemp, Warning, TEXT("[SP] Super Sprint Pressed"))
	isSuperSprintPressed = true;
	localPlayer->SprintPressed();
}

void USuperPioneerMovementManager::SuperSprintReleased() {
	UE_LOG(LogTemp, Warning, TEXT("[SP] Super Sprint Released"))
	isSuperSprintPressed = false;
}

void USuperPioneerMovementManager::SprintTick(float deltaTime) {
	if (GetIsPlayerSprinting()) {
		//UE_LOG(LogTemp, Warning, TEXT("[SP] Attempting to Set Speed... %f"), CalculateSprintSpeed(sprintDuration))
		sprintDuration += deltaTime;
		SetPlayerSprintSpeed(CalculateSprintSpeed(sprintDuration));
		//UE_LOG(LogTemp, Warning, TEXT("[SP] Final Speed... %f"), GetPlayerSprintSpeed(player))
	}
}

float USuperPioneerMovementManager::CalculateSprintSpeed(float duration) {
	return std::min(float((1 + (2.0 * pow(duration, 2))) * defaultMaxSprintSpeed), superSprintMaxSpeed);
}

void USuperPioneerMovementManager::SetPlayerSprintSpeed(float newSprintSpeed) {
	GetPlayerMovementComponent()->mMaxSprintSpeed = newSprintSpeed;
}

float USuperPioneerMovementManager::GetPlayerCurrentSprintSpeed() {
	return GetPlayerMovementComponent()->mMaxSprintSpeed;
}

bool USuperPioneerMovementManager::GetIsPlayerSprinting() {
	return GetPlayerMovementComponent()->GetIsSprinting();
}

// Jumping

float USuperPioneerMovementManager::CalculateJumpZVelocity(AFGCharacterPlayer* player, float heldDuration) {
	float currentSpeed = GetPlayerCurrentSprintSpeed();
	float minSpeed = defaultMaxSprintSpeed;
	float maxSpeed = superSprintMaxSpeed;
	float minJump = defaultJumpZVelocity * superJumpMinZVelocityMultiplier;
	float maxJump = defaultJumpZVelocity * superJumpMaxZVelocityMultiplier * heldDuration;
	return ((maxJump - minJump) / (maxSpeed - minSpeed)) * (currentSpeed - minSpeed) + minJump;
}

void USuperPioneerMovementManager::SetPlayerJumpZVelocity(AFGCharacterPlayer* player, float newZVelocity) {
	GetPlayerMovementComponent()->JumpZVelocity = newZVelocity;
}

float USuperPioneerMovementManager::GetPlayerJumpZVelocity(AFGCharacterPlayer* player) {
	return GetPlayerMovementComponent()->JumpZVelocity;
}