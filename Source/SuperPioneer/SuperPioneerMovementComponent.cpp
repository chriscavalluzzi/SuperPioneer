#include "SuperPioneerMovementComponent.h"
#include <algorithm>
#include <cmath>
#include <string>
#include "SuperPioneerRemoteCallObject.h"
#include "FGPlayerController.h"
#include "FGCharacterPlayer.h"
#include "FGCharacterMovementComponent.h"

USuperPioneerMovementComponent::USuperPioneerMovementComponent() {
  UE_LOG(LogTemp, Warning, TEXT("[SP] Starting SP Movement Component Construction"))
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
	UE_LOG(LogTemp,Warning,TEXT("[SP] Starting SP Movement Component Setup (isHost: %s)"), (_isHost ? TEXT("true") : TEXT("false")))

	this->localPlayer = _localPlayer;
	this->inputComponent = _inputComponent;
	this->isHost = _isHost;

	defaultMaxSprintSpeed = GetPlayerMovementComponent()->mMaxSprintSpeed;
	defaultMaxStepHeight = GetPlayerMovementComponent()->MaxStepHeight;
	defaultGroundFriction = GetPlayerMovementComponent()->GroundFriction;
	defaultJumpZVelocity = GetPlayerMovementComponent()->JumpZVelocity;
	defaultAirControl = GetPlayerMovementComponent()->AirControl;
	defaultGravityScale = GetPlayerMovementComponent()->GravityScale;

	ReloadConfig();

	BindActions();
}

void USuperPioneerMovementComponent::BindActions() {
	inputComponent->BindAction(superSprintCommandName, EInputEvent::IE_Pressed, this, &USuperPioneerMovementComponent::SuperSprintPressed);
	inputComponent->BindAction(superSprintCommandName, EInputEvent::IE_Released, this, &USuperPioneerMovementComponent::SuperSprintReleased);
	inputComponent->BindAction("Jump_Drift", EInputEvent::IE_Pressed, this, &USuperPioneerMovementComponent::JumpPressed);
	inputComponent->BindAction("Jump_Drift", EInputEvent::IE_Released, this, &USuperPioneerMovementComponent::JumpReleased);
	inputComponent->BindAction("ToggleSprint", EInputEvent::IE_Pressed, this, &USuperPioneerMovementComponent::NormalSprintPressed);
	inputComponent->BindAction("ToggleSprint", EInputEvent::IE_Released, this, &USuperPioneerMovementComponent::NormalSprintReleased);
}

void USuperPioneerMovementComponent::ReloadConfig() {
	if (!isDestroyed && IsValid(GetPlayer()) && IsValid(GetPlayer()->Controller)) {

		FSuperPioneer_ConfigStruct SPConfig = FSuperPioneer_ConfigStruct::GetActiveConfig();

		config_superSprintEnabled = SPConfig.superSprint.superSprintEnabled;
		config_superSprintMaxSpeed = SPConfig.superSprint.superSprintMaxSpeedMps * 100.f;
		config_superSprintAccelerationEasing = SPConfig.superSprint.superSprintAccelerationEasing;
		config_superSprintAccelerationMultiplier = SPConfig.superSprint.superSprintAccelerationMultiplier;
		config_superSprintGroundFriction = SPConfig.superSprint.superSprintGroundFriction;
		config_superSprintMaxStepHeight = SPConfig.superSprint.superSprintMaxStepHeight * 100.0f;

		config_superJumpChargingEnabled = SPConfig.superJumpCharging.jumpChargingEnabled;
		config_superJumpHoldMultiplierMax = SPConfig.superJumpCharging.superJumpHoldMultiplierMax;
		config_superJumpHoldTimeMin = SPConfig.superJumpCharging.superJumpHoldTimeMin;
		config_superJumpHoldTimeMax = std::max(SPConfig.superJumpCharging.superJumpHoldTimeMax, config_superJumpHoldTimeMin);

		config_superJumpModificationsEnabled = SPConfig.superJumpModifications.jumpModificationsEnabled;
		config_superJumpSpeedMultiplierMax = SPConfig.superJumpModifications.superJumpSpeedMultiplierMax;
		config_maxAirControl = SPConfig.superJumpModifications.maxAirControl;
		config_gravityScalingMultiplier = SPConfig.superJumpModifications.gravityScalingMultiplier;
		config_jumpMultiplierPerGravityScale = SPConfig.superJumpModifications.jumpMultiplierPerGravityScale;
		config_swimmingJumpMultiplier = SPConfig.superJumpModifications.swimmingJumpMultiplier;

		config_disableFallDamage = SPConfig.other.disableFallDamage;

		if (!config_superSprintEnabled) {
			SetPlayerSprintSpeed(defaultMaxSprintSpeed);
			SetPlayerDeceleration(defaultGroundFriction);
		} else {
			SetPlayerDeceleration(config_superSprintGroundFriction);
		}

		RCO* rco = GetRCO();
		if (rco) {
			rco->ServerSetFallDamageDisabled(config_disableFallDamage);
		}

	}
}

void USuperPioneerMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	CheckForActionRebind();
	if (config_superSprintEnabled) { SprintTick(DeltaTime); }
	if (config_superJumpChargingEnabled) { JumpTick(DeltaTime); }
}

void USuperPioneerMovementComponent::CheckForActionRebind() {
	if (!needToRebindActions && (!IsValid(inputComponent) || inputComponent != GetPlayer()->InputComponent)) {
		needToRebindActions = true;
	} else if (needToRebindActions && IsValid(GetPlayer()->InputComponent)) {
		inputComponent = GetPlayer()->InputComponent;
		BindActions();
		needToRebindActions = false;
	}
}

RCO* USuperPioneerMovementComponent::GetRCO() {
	AFGPlayerController* controller = Cast<AFGPlayerController>(GetPlayer()->Controller);
	return (RCO*)controller->GetRemoteCallObjectOfClass(RCO::StaticClass());
}

AFGCharacterPlayer* USuperPioneerMovementComponent::GetPlayer() {
	return dynamic_cast<AFGCharacterPlayer*>(GetOwner());
}

UFGCharacterMovementComponent* USuperPioneerMovementComponent::GetPlayerMovementComponent() {
	return GetPlayer()->GetFGMovementComponent();
}

void USuperPioneerMovementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	isDestroyed = true;
}

// Sprinting

void USuperPioneerMovementComponent::SuperSprintPressed() {
	if (config_superSprintEnabled) {
		SetPlayerDeceleration(config_superSprintGroundFriction);
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
}

void USuperPioneerMovementComponent::SuperSprintReleased() {
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
		SetPlayerMaxStepHeight(CalculateMaxStepHeight(sprintDuration));
	}
	if (!isFalling && GetPlayerMovementComponent()->IsFalling()) {
		isFalling = true;
		OnFalling();
	}
}

void USuperPioneerMovementComponent::ResetSprintToDefaults() {
	SetPlayerSprintSpeed(defaultMaxSprintSpeed);
	SetPlayerMaxStepHeight(defaultMaxStepHeight);
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

float USuperPioneerMovementComponent::CalculateMaxStepHeight(float duration) {
	return lerp(defaultMaxStepHeight, config_superSprintMaxStepHeight, CalculateCurrentSpeedPercentOfMax());
}

void USuperPioneerMovementComponent::SetPlayerSprintSpeed(float newSprintSpeed) {
	GetPlayerMovementComponent()->mMaxSprintSpeed = newSprintSpeed;

	RCO* rco = GetRCO();
	if (rco && !isHost) {
		rco->ServerSetSprintSpeed(GetPlayer(), newSprintSpeed);
	}
}

void USuperPioneerMovementComponent::SetPlayerMaxStepHeight(float newMaxStepHeight) {
	GetPlayerMovementComponent()->MaxStepHeight = newMaxStepHeight;

	RCO* rco = GetRCO();
	if (rco && !isHost) {
		rco->ServerSetMaxStepHeight(GetPlayer(), newMaxStepHeight);
	}
}

void USuperPioneerMovementComponent::SetPlayerDeceleration(float newGroundFriction) {
	GetPlayerMovementComponent()->GroundFriction = newGroundFriction;

	RCO* rco = GetRCO();
	if (rco && !isHost) {
		rco->ServerSetDeceleration(GetPlayer(), newGroundFriction);
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
	if (GetPlayerMovementComponent()->IsSwimming()) {
		if (config_superJumpModificationsEnabled) {
			SetPlayerJumpZVelocity(defaultJumpZVelocity * config_swimmingJumpMultiplier);
		} else {
			SetPlayerJumpZVelocity(defaultJumpZVelocity);
		}
		SetPlayerAirControl(defaultAirControl);
		SetPlayerGravityScale(defaultGravityScale);
	} else if (!config_superJumpChargingEnabled && CheckIfJumpSafe()) {
		ApplyJumpModifiers();
	}
	isJumpPressed = true;
	isJumpPrimed = false;
	jumpHoldDuration = 0.0;
}

void USuperPioneerMovementComponent::JumpReleased() {
	if (config_superJumpChargingEnabled && !GetPlayerMovementComponent()->IsSwimming() && CheckIfJumpSafe()) {
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
		// Allow sprint to continue
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
	if (isJumpPrimed || !config_superJumpChargingEnabled || GetPlayerMovementComponent()->IsSwimming()) {
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