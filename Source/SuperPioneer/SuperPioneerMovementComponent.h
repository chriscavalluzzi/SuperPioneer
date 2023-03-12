#pragma once

#include "FGCharacterPlayer.h"
#include "SuperPioneer_ConfigStruct.h"
#include "SuperPioneerRemoteCallObject.h"
#include "UI/FGGameUI.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SuperPioneerMovementComponent.generated.h"

using RCO = USuperPioneerRemoteCallObject;

UCLASS()
class SUPERPIONEER_API USuperPioneerMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	USuperPioneerMovementComponent();
	void Setup(AFGCharacterPlayer* _localPlayer, UInputComponent* _inputComponent, bool isHost);
	void ReloadConfig();
	bool CheckAndConsumeJump();
	void OnLanded();
	void OnFalling();

protected:

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

private:

	// General

	AFGCharacterPlayer* localPlayer;
	UInputComponent* inputComponent;
	UUserWidget* reticleHUD;
	const char* superSprintCommandName = "SuperPioneer.SuperSprint";
	bool isHost;
	bool isDestroyed = false;
	bool needToRebindActions = false;

	void Reset();
	void BindActions();
	void CheckForActionRebind();
	void AddReticleHUD();
	RCO* GetRCO();
	AFGCharacterPlayer* GetPlayer();
	UFGCharacterMovementComponent* GetPlayerMovementComponent();
	virtual void BeginPlay();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

	// Sprinting

	bool config_superSprintEnabled;
	float config_superSprintMaxSpeed;
	float config_superSprintAccelerationEasing;
	float config_superSprintAccelerationMultiplier;
	float config_superSprintGroundFriction;
	float config_superSprintMaxStepHeight;

	bool isSuperSprintPressed;
	bool isNormalSprintPressed;
	bool isSuperSprinting;
	bool wasSprintingBeforeSuperSprint;
	bool wasNormalSprintingWhenGrounded;
	bool eligibleForSprintResume;
	bool needToRestartSuperSprint;
	float sprintDuration;
	float defaultMaxSprintSpeed;
	float defaultMaxStepHeight;
	float defaultGroundFriction;

	void SuperSprintPressed();
	void SuperSprintReleased();
	void NormalSprintPressed();
	void NormalSprintReleased();
	void StartSuperSprint();
	void EndSuperSprint();
	void Invoke_SprintPressed();
	void Invoke_SprintReleased();
	void SprintTick(float deltaTime);
	void ResetSprintToDefaults();
	float CalculateSprintSpeed(float duration);
	float CalculateMaxStepHeight(float duration);
	void SetPlayerSprintSpeed(float newSprintSpeed);
	void SetPlayerMaxStepHeight(float newMaxStepHeight);
	void SetPlayerDeceleration(float newGroundFriction);
	float GetPlayerCurrentSprintSpeed();
	bool GetIsPlayerSprinting();
	bool GetIsHoldToSprintEnabled();
	float CalculateCurrentSpeedPercentOfMax();

	// Jumping

	bool config_superJumpChargingEnabled;
	float config_superJumpHoldMultiplierMax;
	float config_superJumpHoldTimeMin;
	float config_superJumpHoldTimeMax;
	bool config_superJumpModificationsEnabled;
	float config_superJumpSpeedMultiplierMax;
	float config_maxAirControl;
	float config_gravityScalingMultiplier;
	float config_jumpMultiplierPerGravityScale;
	float config_swimmingJumpMultiplier;

	float defaultJumpZVelocity;
	float defaultAirControl;
	float defaultGravityScale;
	bool isJumpPressed;
	bool isJumpPrimed;
	bool isFalling;
	float jumpHoldDuration;
	bool isJumpChargeIndicatorVisible;

	void JumpPressed();
	void JumpReleased();
	void ApplyJumpModifiers();
	void ResetJumpModifiers();
	bool CheckIfJumpSafe();
	void Invoke_Jump();
	void JumpTick(float deltaTime);
	float CalculateJumpZVelocity();
	float CalculateJumpMultipliers();
	float CalculateCurrentJumpHoldPercentOfMax();
	void SetPlayerJumpZVelocity(float newZVelocity);
	float GetPlayerJumpZVelocity();
	float CalculateAirControl();
	void SetPlayerAirControl(float newAirControl);
	void SetPlayerGravityScale(float newGravityScale);
	UWidget* GetJumpChargeIndicator();
	UWidget* GetJumpChargeIndicatorCurrent();
	void UpdateJumpChargeIndicator();

	// Ground Slam

	float config_groundSlamMaxAngle;
	float config_groundSlamInitialVelocity;
	float config_groundSlamAcceleration;

	bool isGroundSlamming;
	FVector groundSlamDirection;
	bool isGroundSlamIndicatorVisible;

	void GroundSlamPressed();
	bool IsEligibleForGroundSlam();
	void GroundSlamTick(float deltaTime);
	UWidget* GetGroundSlamIndicator();
	void UpdateGroundSlamIndicator();

	// Other
	bool config_disableFallDamage;

	// Utilities

	static float lerp(float a, float b, float t);
	template<class C>	UWidget* GetUIElementByName(char* name);
};