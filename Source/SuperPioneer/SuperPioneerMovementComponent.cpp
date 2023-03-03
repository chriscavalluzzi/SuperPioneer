#include "SuperPioneerMovementComponent.h"
#include <algorithm>
#include <cmath>
#include <string>
#include "FGCharacterPlayer.h"
#include "FGCharacterMovementComponent.h"

USuperPioneerMovementComponent::USuperPioneerMovementComponent() {
  UE_LOG(LogTemp, Warning, TEXT("[SP] Sprint Manager Construction"))
	PrimaryComponentTick.bCanEverTick = true;
	isSuperSprintPressed = false;
	isNormalSprintPressed = false;
	wasSprintingBeforeSuperSprint = false;
	wasHoldingToSprintBeforeSuperSprint = false;
	eligibleForSprintResume = false;
	isJumpPressed = false;
	isJumpPrimed = false;
	isFalling = false;
	jumpHoldDuration = 0.0;
	sprintDuration = 0.0;
};

void USuperPioneerMovementComponent::Setup(AFGCharacterPlayer* _localPlayer, UInputComponent* _inputComponent) {
  UE_LOG(LogTemp,Warning,TEXT("[SP] Starting Manager Setup"))
  this->localPlayer = _localPlayer;

	ReloadConfig();

	defaultMaxSprintSpeed = GetPlayerMovementComponent()->mMaxSprintSpeed;
	defaultJumpZVelocity = GetPlayerMovementComponent()->JumpZVelocity;
	defaultAirControl = GetPlayerMovementComponent()->AirControl;
	defaultGravityScale = GetPlayerMovementComponent()->GravityScale;

  _inputComponent->BindAction(superSprintCommandName, EInputEvent::IE_Pressed, this, &USuperPioneerMovementComponent::SuperSprintPressed);
	_inputComponent->BindAction(superSprintCommandName, EInputEvent::IE_Released, this, &USuperPioneerMovementComponent::SuperSprintReleased);
	_inputComponent->BindAction("Jump_Drift", EInputEvent::IE_Pressed, this, &USuperPioneerMovementComponent::JumpPressed);
	_inputComponent->BindAction("Jump_Drift", EInputEvent::IE_Released, this, &USuperPioneerMovementComponent::JumpReleased);
	_inputComponent->BindAction("ToggleSprint", EInputEvent::IE_Pressed, this, &USuperPioneerMovementComponent::NormalSprintPressed);
	_inputComponent->BindAction("ToggleSprint", EInputEvent::IE_Released, this, &USuperPioneerMovementComponent::NormalSprintReleased);
}

void USuperPioneerMovementComponent::ReloadConfig() {
	FSuperPioneer_ConfigStruct SPConfig = FSuperPioneer_ConfigStruct::GetActiveConfig();
	config_superSprintMaxSpeed = SPConfig.superSprint.superSprintMaxSpeed;
	config_superJumpSpeedMultiplierMax = SPConfig.superJump.superJumpSpeedMultiplierMax;
	config_superJumpHoldMultiplierMax = SPConfig.superJump.superJumpHoldMultiplierMax;
	config_superJumpHoldTimeMin = SPConfig.superJump.superJumpHoldTimeMin;
	config_superJumpHoldTimeMax = std::max(SPConfig.superJump.superJumpHoldTimeMax,config_superJumpHoldTimeMin);
	config_maxAirControl = SPConfig.superJump.maxAirControl;
	config_gravityScalingMultiplier = SPConfig.superJump.gravityScalingMultiplier;
	config_jumpMultiplierPerGravityScale = SPConfig.superJump.jumpMultiplierPerGravityScale;
}

void USuperPioneerMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	SprintTick(DeltaTime);
	JumpTick(DeltaTime);
}

AFGCharacterPlayer* USuperPioneerMovementComponent::GetPlayer() {
	// If the else block never triggers, this can probably all be replaced with: return static_cast<AFGCharacterPlayer*>(GetOwner())
	AFGCharacterPlayer* playerPtr = dynamic_cast<AFGCharacterPlayer*>(GetOwner());
	if (playerPtr) {
		return playerPtr;
	} else {
		UE_LOG(LogTemp, Warning, TEXT("[SP!] Get owner cast failed!"))
		return playerPtr;
	}
}

UFGCharacterMovementComponent* USuperPioneerMovementComponent::GetPlayerMovementComponent() {
	return GetPlayer()->GetFGMovementComponent();
}

// Sprinting

void USuperPioneerMovementComponent::SuperSprintPressed() {
	//UE_LOG(LogTemp, Warning, TEXT("[SP] Super Sprint Pressed"))
	isSuperSprintPressed = true;
	if (eligibleForSprintResume) {
		return;
	}
	if (GetIsPlayerSprinting()) {
		wasSprintingBeforeSuperSprint = true;
		wasHoldingToSprintBeforeSuperSprint = GetPlayerMovementComponent()->mHoldToSprint;
	} else {
		GetPlayer()->SprintPressed();
		wasSprintingBeforeSuperSprint = false;
		wasHoldingToSprintBeforeSuperSprint = GetPlayerMovementComponent()->mHoldToSprint;
	}
	//UE_LOG(LogTemp, Warning, TEXT("[SP] HoldToSprint: %s"), (wasHoldingToSprintBeforeSuperSprint ? TEXT("true") : TEXT("false")))
}

void USuperPioneerMovementComponent::SuperSprintReleased() {
	//UE_LOG(LogTemp, Warning, TEXT("[SP] Super Sprint Released"))
	isSuperSprintPressed = false;
	if (!GetPlayerMovementComponent()->IsFalling()) {
		ResetSprintToDefaults();
	}
}

void USuperPioneerMovementComponent::NormalSprintPressed() {
	isNormalSprintPressed = true;
}

void USuperPioneerMovementComponent::NormalSprintReleased() {
	isNormalSprintPressed = false;
}

void USuperPioneerMovementComponent::OnFalling() {
	if (isSuperSprintPressed) {
		eligibleForSprintResume = true;
	} else {
		eligibleForSprintResume = false;
	}
}

void USuperPioneerMovementComponent::SprintTick(float deltaTime) {
	if (GetIsPlayerSprinting() && isSuperSprintPressed) {
		sprintDuration += deltaTime;
		SetPlayerSprintSpeed(CalculateSprintSpeed(sprintDuration));
	}
	if (!isFalling && GetPlayerMovementComponent()->IsFalling()) {
		isFalling = true;
		OnFalling();
	}
}

void USuperPioneerMovementComponent::ResetSprintToDefaults() {
	SetPlayerSprintSpeed(defaultMaxSprintSpeed);
	sprintDuration = 0.0;
	if (GetPlayerMovementComponent()->mHoldToSprint) {
		// Hold to sprint enabled
		if (isNormalSprintPressed) {
			GetPlayer()->SprintPressed();
		} else {
			GetPlayer()->SprintReleased();
		}
	} else {
		// Toggle sprint enabled
		if (!wasSprintingBeforeSuperSprint && GetIsPlayerSprinting()) {
			GetPlayer()->SprintPressed();
		}
	}
}

float USuperPioneerMovementComponent::CalculateSprintSpeed(float duration) {
	return std::min(float((1 + (2.0 * pow(duration, 2))) * defaultMaxSprintSpeed), config_superSprintMaxSpeed);
}

void USuperPioneerMovementComponent::SetPlayerSprintSpeed(float newSprintSpeed) {
	GetPlayerMovementComponent()->mMaxSprintSpeed = newSprintSpeed;
}

float USuperPioneerMovementComponent::GetPlayerCurrentSprintSpeed() {
	return GetPlayerMovementComponent()->mMaxSprintSpeed;
}

bool USuperPioneerMovementComponent::GetIsPlayerSprinting() {
	return GetPlayerMovementComponent()->GetIsSprinting();
}

float USuperPioneerMovementComponent::CalculateCurrentSpeedPercentOfMax() {
	float currentSpeed = GetPlayerCurrentSprintSpeed();
	float minSpeed = defaultMaxSprintSpeed;
	float maxSpeed = config_superSprintMaxSpeed;
	return std::clamp((currentSpeed - minSpeed) / std::max((maxSpeed - minSpeed),0.1f), 0.0f, 1.0f);
}

// Jumping

void USuperPioneerMovementComponent::JumpPressed() {
	isJumpPressed = true;
	isJumpPrimed = false;
	jumpHoldDuration = 0.0;
}

void USuperPioneerMovementComponent::JumpReleased() {
	SetPlayerJumpZVelocity(CalculateJumpZVelocity());
	SetPlayerAirControl(CalculateAirControl());
	SetPlayerGravityScale(defaultGravityScale + (CalculateJumpMultipliers() - 1.0f) * config_gravityScalingMultiplier);
	UE_LOG(LogTemp, Warning, TEXT("[SP] ############ Jump Modifications ############"))
	UE_LOG(LogTemp, Warning, TEXT("[SP] Raw Multiplier:    %f"), CalculateJumpMultipliers())
	UE_LOG(LogTemp, Warning, TEXT("[SP] New JumpZVelocity: %f"), GetPlayerMovementComponent()->JumpZVelocity)
	UE_LOG(LogTemp, Warning, TEXT("[SP] New AirControl:    %f"), GetPlayerMovementComponent()->AirControl)
	UE_LOG(LogTemp, Warning, TEXT("[SP] New GravityScale:  %f"), GetPlayerMovementComponent()->GravityScale)
	UE_LOG(LogTemp, Warning, TEXT("[SP] ############################################"))
	isJumpPressed = false;
	isJumpPrimed = true;
	jumpHoldDuration = 0.0;
	GetPlayerMovementComponent()->DoJump(false);
}

void USuperPioneerMovementComponent::OnLanded() {
	SetPlayerJumpZVelocity(defaultJumpZVelocity);
	SetPlayerAirControl(defaultAirControl);
	SetPlayerGravityScale(defaultGravityScale);
	if (eligibleForSprintResume && isSuperSprintPressed) {
		//GetPlayer()->SprintPressed();
	} else {
		ResetSprintToDefaults();
	}
	eligibleForSprintResume = false;
	isFalling = false;
}

void USuperPioneerMovementComponent::JumpTick(float deltaTime) {
	if (isJumpPressed) {
		jumpHoldDuration += deltaTime;
	}
}

bool USuperPioneerMovementComponent::CheckAndConsumeJump() {
	if (isJumpPrimed) {
		isJumpPrimed = false;
		return true;
	} else {
		return false;
	}
}

float USuperPioneerMovementComponent::CalculateJumpZVelocity() {
	float adjustedgravityScalingMultiplier = config_jumpMultiplierPerGravityScale * config_gravityScalingMultiplier;
	return defaultJumpZVelocity * ((CalculateJumpMultipliers() - 1.0f) * adjustedgravityScalingMultiplier + 1.0f);
}

float USuperPioneerMovementComponent::CalculateJumpMultipliers() {
	float speedMultiplier = lerp(1.0f, config_superJumpSpeedMultiplierMax, CalculateCurrentSpeedPercentOfMax());
	float holdMultiplier = lerp(1.0f, config_superJumpHoldMultiplierMax, CalculateCurrentJumpHoldPercentOfMax());
	return speedMultiplier * holdMultiplier;
}

float USuperPioneerMovementComponent::CalculateCurrentJumpHoldPercentOfMax() {
	return std::clamp((jumpHoldDuration - config_superJumpHoldTimeMin) / std::max(config_superJumpHoldTimeMax - config_superJumpHoldTimeMin,0.1f), 0.0f, 1.0f);
}

void USuperPioneerMovementComponent::SetPlayerJumpZVelocity(float newZVelocity) {
	GetPlayerMovementComponent()->JumpZVelocity = newZVelocity;
}

float USuperPioneerMovementComponent::GetPlayerJumpZVelocity() {
	return GetPlayerMovementComponent()->JumpZVelocity;
}

float USuperPioneerMovementComponent::CalculateAirControl() {
	return lerp(defaultAirControl, config_maxAirControl, CalculateCurrentSpeedPercentOfMax());
}

void USuperPioneerMovementComponent::SetPlayerAirControl(float newAirControl) {
	GetPlayerMovementComponent()->AirControl = newAirControl;
}

void USuperPioneerMovementComponent::SetPlayerGravityScale(float newGravityScale) {
	GetPlayerMovementComponent()->GravityScale = newGravityScale;
}

float USuperPioneerMovementComponent::lerp(float a, float b, float t) {
	return a * (1.0 - t) + (b * t);
}