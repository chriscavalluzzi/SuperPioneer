#include "SuperPioneerMovementManager.h"
#include <algorithm>
#include <string>
#include "FGCharacterPlayer.h"
#include "FGCharacterMovementComponent.h"

USuperPioneerMovementManager::USuperPioneerMovementManager() {
  UE_LOG(LogTemp, Warning, TEXT("[SP] Sprint Manager Construction"))
	PrimaryComponentTick.bCanEverTick = true;
	RegisterComponent();
};

void USuperPioneerMovementManager::Setup(AFGCharacterPlayer* _localPlayer, UInputComponent* _inputComponent) {
  UE_LOG(LogTemp,Warning,TEXT("[SP] Starting Manager Setup"))
  this->localPlayer = _localPlayer;
  this->inputComponent = _inputComponent;
	defaultMaxSprintSpeed = GetPlayerMovementComponent()->mMaxSprintSpeed;
	defaultJumpZVelocity = GetPlayerMovementComponent()->JumpZVelocity;
	isSuperSprintPressed = false;
	wasSprintingBeforeSuperSprint = false;
	wasHoldingToSprintBeforeSuperSprint = false;
	sprintDuration = 0.0;
  _inputComponent->BindAction(superSprintCommandName, EInputEvent::IE_Pressed, this, &USuperPioneerMovementManager::SuperSprintPressed);
	_inputComponent->BindAction(superSprintCommandName, EInputEvent::IE_Released, this, &USuperPioneerMovementManager::SuperSprintReleased);
}

void USuperPioneerMovementManager::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	SprintTick(DeltaTime);
}

AFGCharacterPlayer* USuperPioneerMovementManager::GetPlayer() {
	// If the else block never triggers, this can probably all be replaced with: return static_cast<AFGCharacterPlayer*>(GetOwner())
	AFGCharacterPlayer* playerPtr = dynamic_cast<AFGCharacterPlayer*>(GetOwner());
	if (playerPtr) {
		return playerPtr;
	} else {
		UE_LOG(LogTemp, Warning, TEXT("[SP!] Get owner cast failed!"))
		return playerPtr;
	}
}

UFGCharacterMovementComponent* USuperPioneerMovementManager::GetPlayerMovementComponent() {
	return GetPlayer()->GetFGMovementComponent();
}

// Sprinting

void USuperPioneerMovementManager::SuperSprintPressed() {
	UE_LOG(LogTemp, Warning, TEXT("[SP] Super Sprint Pressed"))
	isSuperSprintPressed = true;
	sprintDuration = 0.0;
	SetPlayerSprintSpeed(defaultMaxSprintSpeed);
	if (GetIsPlayerSprinting()) {
		wasSprintingBeforeSuperSprint = true;
		wasHoldingToSprintBeforeSuperSprint = GetPlayerMovementComponent()->mHoldToSprint;
	}
	else {
		GetPlayer()->SprintPressed();
		wasSprintingBeforeSuperSprint = false;
		wasHoldingToSprintBeforeSuperSprint = GetPlayerMovementComponent()->mHoldToSprint;
	}
	UE_LOG(LogTemp, Warning, TEXT("[SP] HoldToSprint: %s"), (wasHoldingToSprintBeforeSuperSprint ? TEXT("true") : TEXT("false")))
}

void USuperPioneerMovementManager::SuperSprintReleased() {
	UE_LOG(LogTemp, Warning, TEXT("[SP] Super Sprint Released"))
	isSuperSprintPressed = false;
	SetPlayerSprintSpeed(defaultMaxSprintSpeed);
	if (wasHoldingToSprintBeforeSuperSprint) {
		GetPlayer()->SprintReleased();
	} else {
		if (wasSprintingBeforeSuperSprint) {
			GetPlayer()->SprintPressed();
		}
	}
}

void USuperPioneerMovementManager::SprintTick(float deltaTime) {
	if (GetIsPlayerSprinting() && isSuperSprintPressed) {
		sprintDuration += deltaTime;
		SetPlayerSprintSpeed(CalculateSprintSpeed(sprintDuration));
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