#include "SuperPioneerMovementManager.h"
#include <algorithm>
#include <cmath>
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
	defaultMaxSprintSpeed = GetPlayerMovementComponent()->mMaxSprintSpeed;
	defaultJumpZVelocity = GetPlayerMovementComponent()->JumpZVelocity;
	defaultAirControl = GetPlayerMovementComponent()->AirControl;
	isSuperSprintPressed = false;
	wasSprintingBeforeSuperSprint = false;
	wasHoldingToSprintBeforeSuperSprint = false;
	isJumpPressed = false;
	isJumpPrimed = false;
	jumpHoldDuration = 0.0;
	sprintDuration = 0.0;
  _inputComponent->BindAction(superSprintCommandName, EInputEvent::IE_Pressed, this, &USuperPioneerMovementManager::SuperSprintPressed);
	_inputComponent->BindAction(superSprintCommandName, EInputEvent::IE_Released, this, &USuperPioneerMovementManager::SuperSprintReleased);
	_inputComponent->BindAction("Jump_Drift", EInputEvent::IE_Pressed, this, &USuperPioneerMovementManager::JumpPressed);
	_inputComponent->BindAction("Jump_Drift", EInputEvent::IE_Released, this, &USuperPioneerMovementManager::JumpReleased);
}

void USuperPioneerMovementManager::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	SprintTick(DeltaTime);
	JumpTick(DeltaTime);
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
	//UE_LOG(LogTemp, Warning, TEXT("[SP] Super Sprint Pressed"))
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
	//UE_LOG(LogTemp, Warning, TEXT("[SP] HoldToSprint: %s"), (wasHoldingToSprintBeforeSuperSprint ? TEXT("true") : TEXT("false")))
}

void USuperPioneerMovementManager::SuperSprintReleased() {
	//UE_LOG(LogTemp, Warning, TEXT("[SP] Super Sprint Released"))
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

float USuperPioneerMovementManager::CalculateCurrentSpeedPercentOfMax() {
	float currentSpeed = GetPlayerCurrentSprintSpeed();
	float minSpeed = defaultMaxSprintSpeed;
	float maxSpeed = superSprintMaxSpeed;
	return std::clamp((currentSpeed - minSpeed) / (maxSpeed - minSpeed), 0.0f, 1.0f);
}

// Jumping

void USuperPioneerMovementManager::JumpPressed() {
	UE_LOG(LogTemp, Warning, TEXT("[SP] Jump Pressed"))
	isJumpPressed = true;
	isJumpPrimed = false;
	jumpHoldDuration = 0.0;
}

void USuperPioneerMovementManager::JumpReleased() {
	UE_LOG(LogTemp, Warning, TEXT("[SP] Jump Released"))
	SetPlayerJumpZVelocity(CalculateJumpZVelocity());
	SetPlayerAirControl(CalculateAirControl());
	isJumpPressed = false;
	isJumpPrimed = true;
	jumpHoldDuration = 0.0;
	UE_LOG(LogTemp, Warning, TEXT("[SP] Air Control: %f"), GetPlayerMovementComponent()->AirControl)
	GetPlayerMovementComponent()->DoJump(false);
}

void USuperPioneerMovementManager::JumpTick(float deltaTime) {
	if (isJumpPressed) {
		jumpHoldDuration += deltaTime;
	}
}

bool USuperPioneerMovementManager::CheckAndConsumeJump() {
	if (isJumpPrimed) {
		isJumpPrimed = false;
		return true;
	} else {
		return false;
	}
}

float USuperPioneerMovementManager::CalculateJumpZVelocity() {
	float speedMultiplier = lerp(1.0f, superJumpSpeedMultiplierMax, CalculateCurrentSpeedPercentOfMax());
	float holdMultiplier = lerp(1.0f, superJumpHoldMultiplierMax, CalculateCurrentJumpHoldPercentOfMax());
	return defaultJumpZVelocity * holdMultiplier * speedMultiplier;
}

float USuperPioneerMovementManager::CalculateCurrentJumpHoldPercentOfMax() {
	return std::clamp((jumpHoldDuration - superJumpHoldTimeMin) / (superJumpHoldTimeMax - superJumpHoldTimeMin), 0.0f, 1.0f);
}

void USuperPioneerMovementManager::SetPlayerJumpZVelocity(float newZVelocity) {
	GetPlayerMovementComponent()->JumpZVelocity = newZVelocity;
}

float USuperPioneerMovementManager::GetPlayerJumpZVelocity() {
	return GetPlayerMovementComponent()->JumpZVelocity;
}

float USuperPioneerMovementManager::CalculateAirControl() {
	return lerp(defaultAirControl, maxAirControl, CalculateCurrentSpeedPercentOfMax());
}

void USuperPioneerMovementManager::SetPlayerAirControl(float newAirControl) {
	GetPlayerMovementComponent()->AirControl = newAirControl;
}

float USuperPioneerMovementManager::lerp(float a, float b, float t) {
	return a * (1.0 - t) + (b * t);
}