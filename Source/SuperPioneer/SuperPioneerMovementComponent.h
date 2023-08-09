#pragma once

#define SPLOG(LogString, ...) UE_LOG(LogTemp,Warning,TEXT(LogString),__VA_ARGS__)
#define SPLOG_BOOL(LogString, TestBool) SPLOG(LogString, TestBool ? TEXT("true") : TEXT("false"))

#include "FGCharacterPlayer.h"
#include "Input/FGInputMappingContext.h"
#include "SuperPioneer_ConfigStruct.h"
#include "SuperPioneerRemoteCallObject.h"
#include "SuperPioneerAnimBlueprint.h"
#include "SuperPioneerHUD.h"
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
	void CheckForHoverPackLand(EMovementMode previousMovementMode, uint8 previousCustomMode, EMovementMode newMovementMode, uint8 newCustomMode);
	void OnFalling();
	UFUNCTION()
	void OnActiveEquipmentChanged();
	void OnEquipmentEquipped(AFGCharacterPlayer* player, AFGEquipment* equipment);
	void SwitchCameraMode(ECameraMode newMode);
	bool AttemptGroundSlam();

protected:

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

private:

	// General

	AFGCharacterPlayer* localPlayer;
	UInputComponent* inputComponent;
	USuperPioneerHUD* reticleHUD;

	UPROPERTY(EditAnywhere, Category = "Input")
	TSoftObjectPtr<UFGInputMappingContext> SPMovementInputMapping;

	const char* reticleHUDWidgetName = "SuperPioneerReticleHUD";
	const char* superSprintCommandName = "SuperPioneer.SuperSprint";
	bool isHost;
	bool isDestroyed = false;
	bool needToRebindActions = false;
	bool isUIBuilt = false;

	void Reset();
	void BindActions(AFGCharacterPlayer* player);
	void CheckForActionRebind();
	void AddReticleHUD();
	void CheckForReticleHUDRebind();
	void SetIsFalling(bool newIsFalling);
	bool IsSafeToAllowPowers();
	RCO* GetRCO();
	AFGCharacterPlayer* GetPlayer();
	UFGCharacterMovementComponent* GetPlayerMovementComponent();
	USkeletalMeshComponent* GetMesh1P();
	virtual void BeginPlay();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

	// Animation

	UClass* vanillaAnimClass;
	UClass* customAnimClass;
	USuperPioneerAnimBlueprint* customAnimInstance;

	UPROPERTY()
	USkeletalMeshComponent* customSkeletalMesh;

	bool config_animationsEnabled;

	void CheckForCustomAnimationToggle();
	void SetupCustomAnimationComponent();
	void DestroyCustomAnimationComponent();
	void ReparentEquipment(USceneComponent* newParent = nullptr);
	void ChangeCustomAnimationState(ESPAnimState newState);
	void CustomAnimationTick(float deltaTime);
	void CaptureVanillaPose();
	void CaptureActiveEquipment();
	void SwitchVisibleMesh(USkeletalMeshComponent* customSkeletalMesh);

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

	bool IsSafeToAllowSuperSprinting();
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
	bool config_superJumpChargingUIEnabled;
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

	bool IsSafeToAllowSuperJumping();
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
	void UpdateJumpChargeIndicator();

	// Ground Slam

	bool config_groundSlamEnabled;
	bool config_groundSlamUIEnabled;
	float config_groundSlamMaxAngle;
	float config_groundSlamInitialVelocity;
	float config_groundSlamAcceleration;
	float config_groundSlamGroundFriction;

	bool isGroundSlamming;
	bool isCrouchPressed;
	FVector groundSlamDirection;
	bool isGroundSlamIndicatorVisible;

	bool IsSafeToAllowGroundSlam();
	bool IsEligibleForGroundSlam();
	void GroundSlamTick(float deltaTime);
	void UpdateGroundSlamIndicator();
	void GroundSlamLaunch(FVector vector);
	void GroundSlamAddForce(FVector force);
	void CrouchReleased();

	// Other

	bool config_disableFallDamage;

	bool isHoverPackEquipped;
	bool wasHoverPackHoveringLastTick;

	// Utilities

	static float lerp(float a, float b, float t);
	UUserWidget* GetGameUI();
	bool IsHoverPackHovering();

};