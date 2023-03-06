#include "SuperPioneerMovementComponent.h"
#include <algorithm>
#include <cmath>
#include <string>
#include "SuperPioneerRemoteCallObject.h"
#include "FGPlayerController.h"
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

void USuperPioneerMovementComponent::Setup(AFGCharacterPlayer* _localPlayer, UInputComponent* _inputComponent, bool _isHost) {
  UE_LOG(LogTemp,Warning,TEXT("[SP] Starting Manager Setup"))
	UE_LOG(LogTemp,Warning,TEXT("[SP] isHost: %s"), (_isHost ? TEXT("true") : TEXT("false")))

	this->localPlayer = _localPlayer;
	this->isHost = _isHost;

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

	config_superSprintEnabled = SPConfig.superSprint.superSprintEnabled;
	config_superSprintMaxSpeed = SPConfig.superSprint.superSprintMaxSpeed;
	config_superSprintAccelerationEasing = SPConfig.superSprint.superSprintAccelerationEasing;
	config_superSprintAccelerationMultiplier = SPConfig.superSprint.superSprintAccelerationMultiplier;

	config_superJumpChargingEnabled = SPConfig.superJumpCharging.jumpChargingEnabled;
	config_superJumpHoldMultiplierMax = SPConfig.superJumpCharging.superJumpHoldMultiplierMax;
	config_superJumpHoldTimeMin = SPConfig.superJumpCharging.superJumpHoldTimeMin;
	config_superJumpHoldTimeMax = std::max(SPConfig.superJumpCharging.superJumpHoldTimeMax, config_superJumpHoldTimeMin);

	config_superJumpModificationsEnabled = SPConfig.superJumpModifications.jumpModificationsEnabled;
	config_superJumpSpeedMultiplierMax = SPConfig.superJumpModifications.superJumpSpeedMultiplierMax;
	config_maxAirControl = SPConfig.superJumpModifications.maxAirControl;
	config_gravityScalingMultiplier = SPConfig.superJumpModifications.gravityScalingMultiplier;
	config_jumpMultiplierPerGravityScale = SPConfig.superJumpModifications.jumpMultiplierPerGravityScale;

	config_disableFallDamage = SPConfig.other.disableFallDamage;

	if (!config_superSprintEnabled) {
		SetPlayerSprintSpeed(defaultMaxSprintSpeed);
	}

	RCO* rco = GetRCO();
	if (rco) {
		rco->ServerSetFallDamageDisabled(config_disableFallDamage);
	}
}

void USuperPioneerMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	if (config_superSprintEnabled) { SprintTick(DeltaTime); }
	if (config_superJumpChargingEnabled) { JumpTick(DeltaTime); }
}

RCO* USuperPioneerMovementComponent::GetRCO() {
	AFGPlayerController* controller = Cast<AFGPlayerController>(GetPlayer()->Controller);
	return (RCO*)controller->GetRemoteCallObjectOfClass(RCO::StaticClass());
}

AFGCharacterPlayer* USuperPioneerMovementComponent::GetPlayer() {
	// If the else block never triggers, this can probably all be replaced with: return static_cast<AFGCharacterPlayer*>(GetOwner())
	AFGCharacterPlayer* playerPtr = dynamic_cast<AFGCharacterPlayer*>(GetOwner());
	if (playerPtr) {
		return playerPtr;
	} else {
		UE_LOG(LogTemp, Error, TEXT("[SP!] Get owner cast failed!"))
		return playerPtr;
	}
}

UFGCharacterMovementComponent* USuperPioneerMovementComponent::GetPlayerMovementComponent() {
	return GetPlayer()->GetFGMovementComponent();
}

// Sprinting

void USuperPioneerMovementComponent::SuperSprintPressed() {
	//UE_LOG(LogTemp, Warning, TEXT("[SP] Super Sprint Pressed"))
	if (config_superSprintEnabled) {
		isSuperSprintPressed = true;
		if (eligibleForSprintResume) {
			return;
		}
		if (GetIsPlayerSprinting()) {
			wasSprintingBeforeSuperSprint = true;
			wasHoldingToSprintBeforeSuperSprint = GetPlayerMovementComponent()->mHoldToSprint;
		}
		else {
			Invoke_SprintPressed();
			wasSprintingBeforeSuperSprint = false;
			wasHoldingToSprintBeforeSuperSprint = GetPlayerMovementComponent()->mHoldToSprint;
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("[SP] HoldToSprint: %s"), (wasHoldingToSprintBeforeSuperSprint ? TEXT("true") : TEXT("false")))
}

void USuperPioneerMovementComponent::SuperSprintReleased() {
	//UE_LOG(LogTemp, Warning, TEXT("[SP] Super Sprint Released"))
	if (config_superSprintEnabled) {
		isSuperSprintPressed = false;
		if (!GetPlayerMovementComponent()->IsFalling()) {
			ResetSprintToDefaults();
		}
	}
}

void USuperPioneerMovementComponent::NormalSprintPressed() {
	isNormalSprintPressed = true;
}

void USuperPioneerMovementComponent::NormalSprintReleased() {
	isNormalSprintPressed = false;
}

void USuperPioneerMovementComponent::Invoke_SprintPressed() {
	GetPlayer()->SprintPressed();

	RCO* rco = GetRCO();
	if (rco && !isHost) {
		rco->ServerSprintPressed(GetPlayer());
	}
}

void USuperPioneerMovementComponent::Invoke_SprintReleased() {
	GetPlayer()->SprintReleased();

	RCO* rco = GetRCO();
	if (rco && !isHost) {
		rco->ServerSprintReleased(GetPlayer());
	}
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
			Invoke_SprintPressed();
		} else {
			Invoke_SprintReleased();
		}
	} else {
		// Toggle sprint enabled
		if (!wasSprintingBeforeSuperSprint && GetIsPlayerSprinting()) {
			Invoke_SprintPressed();
		}
	}
}

float USuperPioneerMovementComponent::CalculateSprintSpeed(float duration) {
	return std::min(float((1 + (config_superSprintAccelerationMultiplier * pow(duration, config_superSprintAccelerationEasing))) * defaultMaxSprintSpeed), config_superSprintMaxSpeed);
}

void USuperPioneerMovementComponent::SetPlayerSprintSpeed(float newSprintSpeed) {
	GetPlayerMovementComponent()->mMaxSprintSpeed = newSprintSpeed;

	RCO* rco = GetRCO();
	if (rco && !isHost) {
		rco->ServerSetSprintSpeed(GetPlayer(), newSprintSpeed);
	}
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
	if (!config_superJumpChargingEnabled && CheckIfJumpSafe()) {
		ApplyJumpModifiers();
	}
	isJumpPressed = true;
	isJumpPrimed = false;
	jumpHoldDuration = 0.0;
}

void USuperPioneerMovementComponent::JumpReleased() {
	if (config_superJumpChargingEnabled && CheckIfJumpSafe()) {
		ApplyJumpModifiers();
		isJumpPrimed = true;
		Invoke_Jump();
	} else {
		isJumpPrimed = false;
	}
	isJumpPressed = false;
	jumpHoldDuration = 0.0;
}

void USuperPioneerMovementComponent::ApplyJumpModifiers() {
	SetPlayerJumpZVelocity(CalculateJumpZVelocity());
	if (config_superJumpModificationsEnabled) {
		SetPlayerAirControl(CalculateAirControl());
		SetPlayerGravityScale(defaultGravityScale + (CalculateJumpMultipliers() - 1.0f) * config_gravityScalingMultiplier);
		UE_LOG(LogTemp, Warning, TEXT("[SP] ############ Jump Modifications ############"))
		UE_LOG(LogTemp, Warning, TEXT("[SP] Raw Multiplier:    %f"), CalculateJumpMultipliers())
		UE_LOG(LogTemp, Warning, TEXT("[SP] New JumpZVelocity: %f"), GetPlayerMovementComponent()->JumpZVelocity)
		UE_LOG(LogTemp, Warning, TEXT("[SP] New AirControl:    %f"), GetPlayerMovementComponent()->AirControl)
		UE_LOG(LogTemp, Warning, TEXT("[SP] New GravityScale:  %f"), GetPlayerMovementComponent()->GravityScale)
		UE_LOG(LogTemp, Warning, TEXT("[SP] ############################################"))
	}
}

bool USuperPioneerMovementComponent::CheckIfJumpSafe() {
	return GetPlayer()->CanJumpInternal_Implementation() && !GetPlayer()->IsMoveInputIgnored();
}

void USuperPioneerMovementComponent::Invoke_Jump() {
	GetPlayerMovementComponent()->DoJump(false);

	RCO* rco = GetRCO();
	if (rco && !isHost) {
		rco->ServerDoJump(GetPlayer());
	}
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
	if (isJumpPrimed || !config_superJumpChargingEnabled) {
		isJumpPrimed = false;
		return true;
	} else {
		return false;
	}
}

float USuperPioneerMovementComponent::CalculateJumpZVelocity() {
	float adjustedgravityScalingMultiplier;
	if (config_superJumpModificationsEnabled) {
		adjustedgravityScalingMultiplier = config_jumpMultiplierPerGravityScale * config_gravityScalingMultiplier;
	} else {
		adjustedgravityScalingMultiplier = 1.0f;
	}
	return defaultJumpZVelocity * ((CalculateJumpMultipliers() - 1.0f) * adjustedgravityScalingMultiplier + 1.0f);
}

float USuperPioneerMovementComponent::CalculateJumpMultipliers() {
	float speedMultiplier = 1.0f;
	if (config_superJumpModificationsEnabled) {
		speedMultiplier = lerp(1.0f, config_superJumpSpeedMultiplierMax, CalculateCurrentSpeedPercentOfMax());
	}
	float holdMultiplier = 1.0f;
	if (config_superJumpChargingEnabled) {
		holdMultiplier = lerp(1.0f, config_superJumpHoldMultiplierMax, CalculateCurrentJumpHoldPercentOfMax());
	}
	return speedMultiplier * holdMultiplier;
}

float USuperPioneerMovementComponent::CalculateCurrentJumpHoldPercentOfMax() {
	return std::clamp((jumpHoldDuration - config_superJumpHoldTimeMin) / std::max(config_superJumpHoldTimeMax - config_superJumpHoldTimeMin,0.1f), 0.0f, 1.0f);
}

void USuperPioneerMovementComponent::SetPlayerJumpZVelocity(float newZVelocity) {
	GetPlayerMovementComponent()->JumpZVelocity = newZVelocity;

	RCO* rco = GetRCO();
	if (rco && !isHost) {
		rco->ServerSetJumpZVelocity(GetPlayer(), newZVelocity);
	}
}

float USuperPioneerMovementComponent::GetPlayerJumpZVelocity() {
	return GetPlayerMovementComponent()->JumpZVelocity;
}

float USuperPioneerMovementComponent::CalculateAirControl() {
	return lerp(defaultAirControl, config_maxAirControl, CalculateCurrentSpeedPercentOfMax());
}

void USuperPioneerMovementComponent::SetPlayerAirControl(float newAirControl) {
	GetPlayerMovementComponent()->AirControl = newAirControl;

	RCO* rco = GetRCO();
	if (rco && !isHost) {
		rco->ServerSetAirControl(GetPlayer(), newAirControl);
	}
}

void USuperPioneerMovementComponent::SetPlayerGravityScale(float newGravityScale) {
	GetPlayerMovementComponent()->GravityScale = newGravityScale;

	RCO* rco = GetRCO();
	if (rco && !isHost) {
		rco->ServerSetGravityScale(GetPlayer(), newGravityScale);
	}
}

float USuperPioneerMovementComponent::lerp(float a, float b, float t) {
	return a * (1.0 - t) + (b * t);
}