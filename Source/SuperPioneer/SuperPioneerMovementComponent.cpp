#include "SuperPioneerMovementComponent.h"
#include <algorithm>
#include <cmath>
#include <string>
#include "SuperPioneerRemoteCallObject.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "FGInputSettings.h"
#include "FGPlayerController.h"
#include "FGCharacterPlayer.h"
#include "Equipment/FGHoverPack.h"
#include "FGCharacterMovementComponent.h"

USuperPioneerMovementComponent::USuperPioneerMovementComponent() {
  UE_LOG(LogTemp, Warning, TEXT("[SP] Starting SP Movement Component Construction"))
	PrimaryComponentTick.bCanEverTick = true;
	Reset();

	static ConstructorHelpers::FObjectFinder<UClass> anim(TEXT("Class'/SuperPioneer/Animations/SuperPioneer1PAnimation.SuperPioneer1PAnimation_C'"));
	customAnimClass = anim.Object;
};

void USuperPioneerMovementComponent::Reset() {
	isSuperSprintPressed = false;
	isNormalSprintPressed = false;
	isSuperSprinting = false;
	wasSprintingBeforeSuperSprint = false;
	wasNormalSprintingWhenGrounded = false;
	eligibleForSprintResume = false;
	needToRestartSuperSprint = false;
	isJumpPressed = false;
	isJumpPrimed = false;
	isGroundSlamming = false;
	isGroundSlamIndicatorVisible = false;
	isCrouchPressed = false;
	isHoverPackEquipped = false;
	wasHoverPackHoveringLastTick = false;
	jumpHoldDuration = 0.0;
	sprintDuration = 0.0;
	SetIsFalling(false);
}

void USuperPioneerMovementComponent::Setup(AFGCharacterPlayer* _localPlayer, UInputComponent* _inputComponent, bool _isHost) {
	UE_LOG(LogTemp,Warning,TEXT("[SP] Starting SP Movement Component Setup (isHost: %s)"), (_isHost ? TEXT("true") : TEXT("false")))

	this->localPlayer = _localPlayer;
	this->inputComponent = _inputComponent;
	this->isHost = _isHost;

	isUIBuilt = false;
	if (AController* controller = _localPlayer->Controller) {
		if (AFGPlayerController* playerController = Cast<AFGPlayerController>(controller)) {
			if (AFGHUD* hud = playerController->GetHUD<AFGHUD>()) {
				if (hud->GetGameUI()) {
					UE_LOG(LogTemp, Warning, TEXT("[SP] Existing HUD detected, will rebind instead of creating"))
					isUIBuilt = true;
				}
			}
		}
	}

	defaultMaxSprintSpeed = GetPlayerMovementComponent()->mMaxSprintSpeed;
	defaultMaxStepHeight = GetPlayerMovementComponent()->MaxStepHeight;
	defaultGroundFriction = GetPlayerMovementComponent()->GroundFriction;
	defaultJumpZVelocity = GetPlayerMovementComponent()->JumpZVelocity;
	defaultAirControl = GetPlayerMovementComponent()->AirControl;
	defaultGravityScale = GetPlayerMovementComponent()->GravityScale;

	vanillaAnimClass = GetMesh1P()->GetAnimClass();

	ReloadConfig();

	BindActions(_localPlayer);
}

void USuperPioneerMovementComponent::BindActions(AFGCharacterPlayer* player) {

	UE_LOG(LogTemp, Warning, TEXT("[SP] Binding input actions..."))

	// Vanilla actions

	const UFGInputSettings* inputSettings = UFGInputSettings::Get();
	UInputAction* sprintAction = inputSettings->GetInputActionForTag(FGameplayTag::RequestGameplayTag(TEXT("Input.PlayerMovement.Sprint")));
	UInputAction* crouchAction = inputSettings->GetInputActionForTag(FGameplayTag::RequestGameplayTag(TEXT("Input.PlayerMovement.Crouch")));
	UInputAction* jumpAction = inputSettings->GetInputActionForTag(FGameplayTag::RequestGameplayTag(TEXT("Input.PlayerMovement.Jump")));

	UEnhancedInputComponent* enhancedInput = Cast<UEnhancedInputComponent>(this->inputComponent);
	enhancedInput->BindAction(jumpAction, ETriggerEvent::Started, this, &USuperPioneerMovementComponent::JumpPressed);
	enhancedInput->BindAction(jumpAction, ETriggerEvent::Completed, this, &USuperPioneerMovementComponent::JumpReleased);
	enhancedInput->BindAction(sprintAction, ETriggerEvent::Started, this, &USuperPioneerMovementComponent::NormalSprintPressed);
	enhancedInput->BindAction(sprintAction, ETriggerEvent::Completed, this, &USuperPioneerMovementComponent::NormalSprintReleased);
	enhancedInput->BindAction(crouchAction, ETriggerEvent::Completed, this, &USuperPioneerMovementComponent::CrouchReleased);

	// Custom actions

	// Load movement mapping context
	FSoftObjectPath SPMovementInputMappingPath(TEXT("/Script/FactoryGame.FGInputMappingContext'/SuperPioneer/Input/MC_SuperPioneerPlayerMovement.MC_SuperPioneerPlayerMovement'"));
	UFGInputMappingContext* SPMovementInputMapping = Cast<UFGInputMappingContext>(SPMovementInputMappingPath.ResolveObject());
	if (SPMovementInputMapping == nullptr) {
		SPMovementInputMapping = CastChecked<UFGInputMappingContext>(SPMovementInputMappingPath.TryLoad());
	}

	// Add mapping context to player
	if (ULocalPlayer* localPlayerCast = Cast<ULocalPlayer>(player)) {
		if (UEnhancedInputLocalPlayerSubsystem* InputSystem = localPlayerCast->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()) {
			if (SPMovementInputMapping) {
				InputSystem->AddMappingContext(SPMovementInputMapping, 0);
			}
		}
	}

	// Bind action functions
	const UInputAction* superSprintAction = SPMovementInputMapping->GetMapping(0).Action.Get();
	enhancedInput->BindAction(superSprintAction, ETriggerEvent::Started, this, &USuperPioneerMovementComponent::SuperSprintPressed);
	enhancedInput->BindAction(superSprintAction, ETriggerEvent::Completed, this, &USuperPioneerMovementComponent::SuperSprintReleased);
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
		config_superJumpChargingUIEnabled = SPConfig.superJumpCharging.jumpChargingUIEnabled;
		config_superJumpHoldMultiplierMax = SPConfig.superJumpCharging.superJumpHoldMultiplierMax;
		config_superJumpHoldTimeMin = SPConfig.superJumpCharging.superJumpHoldTimeMin;
		config_superJumpHoldTimeMax = std::max(SPConfig.superJumpCharging.superJumpHoldTimeMax, config_superJumpHoldTimeMin);

		config_superJumpModificationsEnabled = SPConfig.superJumpModifications.jumpModificationsEnabled;
		config_superJumpSpeedMultiplierMax = SPConfig.superJumpModifications.superJumpSpeedMultiplierMax;
		config_maxAirControl = SPConfig.superJumpModifications.maxAirControl;
		config_gravityScalingMultiplier = SPConfig.superJumpModifications.gravityScalingMultiplier;
		config_jumpMultiplierPerGravityScale = SPConfig.superJumpModifications.jumpMultiplierPerGravityScale;
		config_swimmingJumpMultiplier = SPConfig.superJumpModifications.swimmingJumpMultiplier;

		config_groundSlamEnabled = SPConfig.groundSlam.groundSlamEnabled;
		config_groundSlamUIEnabled = SPConfig.groundSlam.groundSlamUIEnabled;
		config_groundSlamMaxAngle = SPConfig.groundSlam.groundSlamMaxAngle;
		config_groundSlamInitialVelocity = SPConfig.groundSlam.groundSlamInitialVelocity * 100.0f;
		config_groundSlamAcceleration = SPConfig.groundSlam.groundSlamAcceleration * 100.f;
		config_groundSlamGroundFriction = SPConfig.groundSlam.groundSlamGroundFriction;

		config_disableFallDamage = SPConfig.general.disableFallDamage;
		config_animationsEnabled = SPConfig.general.animationsEnabled;
		CheckForCustomAnimationToggle();

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
	CustomAnimationTick(DeltaTime);
	CheckForActionRebind();
	CheckForReticleHUDRebind();

	SprintTick(DeltaTime);
	JumpTick(DeltaTime);
	GroundSlamTick(DeltaTime);

	wasHoverPackHoveringLastTick = IsHoverPackHovering();
}

void USuperPioneerMovementComponent::CheckForActionRebind() {
	if (!needToRebindActions && (!IsValid(inputComponent) || inputComponent != GetPlayer()->InputComponent)) {
		needToRebindActions = true;
		Reset();
		ReparentEquipment();
	} else if (needToRebindActions && IsValid(GetPlayer()->InputComponent)) {
		inputComponent = GetPlayer()->InputComponent;
		BindActions(GetPlayer());
		needToRebindActions = false;
	}
}

void USuperPioneerMovementComponent::AddReticleHUD() {
	if (!isUIBuilt) {
		UE_LOG(LogTemp, Warning, TEXT("[SP] Attempting to create reticle HUD..."))
		FStringClassReference groundSlamWidgetClassRef(TEXT("WidgetBlueprint'/SuperPioneer/SuperPioneerReticleHUD.SuperPioneerReticleHUD_C'"));
		if (UClass* groundSlamWidgetClass = groundSlamWidgetClassRef.TryLoadClass<UUserWidget>()) {
			if(UUserWidget* gameUI = GetGameUI()) {
				if (UCanvasPanel* parentWidget = gameUI->WidgetTree->FindWidget<UCanvasPanel>("StandardUI")) {
					reticleHUD = CreateWidget<USuperPioneerHUD>(((UGameEngine*)GEngine)->GameInstance, groundSlamWidgetClass, reticleHUDWidgetName);
					reticleHUD->SetRenderTransformPivot(FVector2D(0.5, 0.5));
					UPanelSlot* slot = parentWidget->AddChild(reticleHUD);
					if (UCanvasPanelSlot* panelSlot = Cast<UCanvasPanelSlot>(slot)) {
						panelSlot->SetAnchors(FAnchors(0.0, 0.0, 1.0, 1.0));
						panelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
						panelSlot->SetSize(gameUI->GetDesiredSize());
						UE_LOG(LogTemp, Warning, TEXT("[SP] Reticle HUD created successfully."))
						return;
					}
				}
			}
		}
		UE_LOG(LogTemp, Error, TEXT("[SP] Reticle HUD creation failed!"))
	}
}

void USuperPioneerMovementComponent::CheckForReticleHUDRebind() {
	if (!reticleHUD) {
			if (UUserWidget* gameUI = GetGameUI()) {
					if (USuperPioneerHUD* existingReticleHUD = gameUI->WidgetTree->FindWidget<USuperPioneerHUD>(reticleHUDWidgetName)) {
						reticleHUD = existingReticleHUD;
						isJumpChargeIndicatorVisible = true;
						UpdateJumpChargeIndicator();
						isGroundSlamIndicatorVisible = true;
						UpdateGroundSlamIndicator();
					}
			}
	}
}

void USuperPioneerMovementComponent::SetIsFalling(bool newIsFalling) {
	isFalling = newIsFalling;
	if (customAnimInstance) {
		customAnimInstance->isFalling = newIsFalling;
	}
}

bool USuperPioneerMovementComponent::IsSafeToAllowPowers() {
	return !GetPlayer()->IsMoveInputIgnored() && !IsHoverPackHovering();
}

RCO* USuperPioneerMovementComponent::GetRCO() {
	AController* controller = GetPlayer()->Controller;
	if (controller) {
		AFGPlayerController* playerController = Cast<AFGPlayerController>(controller);
		if (playerController) {
			return (RCO*)playerController->GetRemoteCallObjectOfClass(RCO::StaticClass());
		}
	}
	return nullptr;
}

AFGCharacterPlayer* USuperPioneerMovementComponent::GetPlayer() {
	return dynamic_cast<AFGCharacterPlayer*>(GetOwner());
}

UFGCharacterMovementComponent* USuperPioneerMovementComponent::GetPlayerMovementComponent() {
	return GetPlayer()->GetFGMovementComponent();
}

USkeletalMeshComponent* USuperPioneerMovementComponent::GetMesh1P() {
	return GetPlayer()->GetMesh1P();
}

void USuperPioneerMovementComponent::BeginPlay() {
	AddReticleHUD();
	CheckForCustomAnimationToggle();
	ReparentEquipment();

	GetPlayer()->OnEquipmentEquipped.AddUObject(this, &USuperPioneerMovementComponent::OnEquipmentEquipped);
	GetPlayer()->OnEquipmentUnequipped.AddUObject(this, &USuperPioneerMovementComponent::OnEquipmentEquipped);
}

void USuperPioneerMovementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	isDestroyed = true;
}

// Animation

void USuperPioneerMovementComponent::CheckForCustomAnimationToggle() {
	bool eligableForCustomAnimations = config_animationsEnabled && !isHoverPackEquipped;
	if (!customAnimInstance && eligableForCustomAnimations) {
		SetupCustomAnimationComponent();
	} else if (customAnimInstance && !eligableForCustomAnimations) {
		DestroyCustomAnimationComponent();
	}
}

void USuperPioneerMovementComponent::SetupCustomAnimationComponent() {
	if (customSkeletalMesh || !config_animationsEnabled) return;

	USkeletalMeshComponent* mesh1P = GetMesh1P();

	// Build custom SkeletalMeshComponent
	customSkeletalMesh = DuplicateObject<USkeletalMeshComponent>(mesh1P, mesh1P->GetOuter(), "SPSkeletalMeshComponent"); // Make sure customSkeletalMesh is a UPROPERTY to avoid GC
	customSkeletalMesh->AnimScriptInstance = nullptr; // Breaks link to original AnimScriptInstance, ensures it not cleared during SetAnimClass
	customSkeletalMesh->SetAnimClass(customAnimClass);
	customSkeletalMesh->RegisterComponent();
	customAnimInstance = Cast<USuperPioneerAnimBlueprint>(customSkeletalMesh->GetAnimInstance());

	// Hide original SkeletalMeshComponent (but make sure it keeps updating)
	mesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	mesh1P->SetVisibility(false);

	CaptureVanillaPose();
	ReparentEquipment(customSkeletalMesh);
	ChangeCustomAnimationState(ESPAnimState::VANILLA);
}

void USuperPioneerMovementComponent::DestroyCustomAnimationComponent() {
	if (!customSkeletalMesh) return;

	customAnimInstance = nullptr;

	GetMesh1P()->SetVisibility(true);
	ReparentEquipment(GetPlayer()->GetMainMesh());

	customSkeletalMesh->DestroyComponent();
	customSkeletalMesh = nullptr;
}

void USuperPioneerMovementComponent::SwitchCameraMode(ECameraMode newMode) {
	if (customSkeletalMesh) {
		if (newMode == ECameraMode::ECM_FirstPerson) {
				customSkeletalMesh->SetVisibility(true);
		}
		else if (newMode == ECameraMode::ECM_ThirdPerson) {
			customSkeletalMesh->SetVisibility(false);
		}
	}
	ReparentEquipment();
}

void USuperPioneerMovementComponent::OnActiveEquipmentChanged() {
	ReparentEquipment();
}

void USuperPioneerMovementComponent::OnEquipmentEquipped(AFGCharacterPlayer* player, AFGEquipment* equipment) {
	ReparentEquipment();
}

void USuperPioneerMovementComponent::ReparentEquipment(USceneComponent* newParent) {
	if (!newParent) {
		if (customSkeletalMesh && GetPlayer()->GetMainMesh() == GetPlayer()->GetMesh1P()) {
			newParent = customSkeletalMesh;
		}
		else {
			newParent = GetPlayer()->GetMainMesh();
		}
	}
	TArray< AFGEquipment* > equipments = GetPlayer()->GetActiveEquipments();
	for (int i = 0; i < equipments.Num(); i++) {
		AFGEquipment* equipment = equipments[i];
		if (IsValid(equipment)) {
			FName socketName = equipment->GetAttachParentSocketName();
			if (socketName != NAME_None) {
				equipment->AttachToComponent(newParent, FAttachmentTransformRules::KeepRelativeTransform, socketName);
			}
		}
	}
	CaptureActiveEquipment();
}

void USuperPioneerMovementComponent::ChangeCustomAnimationState(ESPAnimState newState) {
	if (customAnimInstance && newState != customAnimInstance->animState) {
		CaptureActiveEquipment();
		customAnimInstance->animState = newState;
	}
}

void USuperPioneerMovementComponent::CustomAnimationTick(float deltaTime) {
	if (customSkeletalMesh) {
		CaptureVanillaPose();
		customSkeletalMesh->SetRelativeTransform(GetMesh1P()->GetRelativeTransform());
	}
}

void USuperPioneerMovementComponent::CaptureVanillaPose() {
	if (customAnimInstance && GetMesh1P() && GetMesh1P()->GetAnimInstance()) {
		FPoseSnapshot vanillaPoseSnapshot;
		GetMesh1P()->GetAnimInstance()->SnapshotPose(vanillaPoseSnapshot);
		customAnimInstance->vanillaPose = vanillaPoseSnapshot;
	}
}

void USuperPioneerMovementComponent::CaptureActiveEquipment() {
	AFGEquipment* bodyEquipment = GetPlayer()->GetEquipmentInSlot(EEquipmentSlot::ES_BACK);
	isHoverPackEquipped = bodyEquipment && bodyEquipment->GetClass()->IsChildOf(AFGHoverPack::StaticClass());
	CheckForCustomAnimationToggle();

	if (customAnimInstance) {
		if (AFGEquipment* armEquipment = GetPlayer()->GetEquipmentInSlot(EEquipmentSlot::ES_ARMS)) {
			customAnimInstance->armsEquipmentClass = armEquipment->GetClass();
			customAnimInstance->armsEquipmentActive = true;
		}
		else {
			customAnimInstance->armsEquipmentActive = false;
		}
		customAnimInstance->isBuildGunEquipped = GetPlayer()->IsBuildGunEquipped();
	}
}

// Sprinting

bool USuperPioneerMovementComponent::IsSafeToAllowSuperSprinting() {
	return IsSafeToAllowPowers();
}

void USuperPioneerMovementComponent::SuperSprintPressed() {
	if (config_superSprintEnabled) {
		isSuperSprintPressed = true;
		if (eligibleForSprintResume) {
			return;
		}
		if (!isFalling) {
			StartSuperSprint();
		}
	}
}

void USuperPioneerMovementComponent::SuperSprintReleased() {
	if (config_superSprintEnabled) {
		isSuperSprintPressed = false;
		if (!GetPlayerMovementComponent()->IsFalling()) {
			EndSuperSprint();
		}
	}
}

void USuperPioneerMovementComponent::NormalSprintPressed() {
	isNormalSprintPressed = true;
	if (!GetIsHoldToSprintEnabled() && isSuperSprinting && isSuperSprintPressed) {
		needToRestartSuperSprint = true;
	}
}

void USuperPioneerMovementComponent::NormalSprintReleased() {
	isNormalSprintPressed = false;
	if (GetIsHoldToSprintEnabled() && isSuperSprinting && isSuperSprintPressed) {
		needToRestartSuperSprint = true;
	}
}

void USuperPioneerMovementComponent::StartSuperSprint() {
	if (IsSafeToAllowSuperSprinting()) {
		SetPlayerDeceleration(config_superSprintGroundFriction);
		if (!isFalling) {
			wasSprintingBeforeSuperSprint = GetIsPlayerSprinting();
		}
		if (!wasSprintingBeforeSuperSprint || GetIsHoldToSprintEnabled()) {
			Invoke_SprintPressed();
		}
		isSuperSprinting = true;
	}
}

void USuperPioneerMovementComponent::EndSuperSprint() {
	ResetSprintToDefaults();
	if (GetIsHoldToSprintEnabled()) {
		// Hold to sprint enabled
		if (isNormalSprintPressed) {
			Invoke_SprintPressed();
		} else {
			Invoke_SprintReleased();
		}
	}
	else {
		// Toggle sprint enabled
		if (!wasSprintingBeforeSuperSprint) {
			Invoke_SprintPressed();
		}
	}
	isSuperSprinting = false;
}

void USuperPioneerMovementComponent::Invoke_SprintPressed() {
	GetPlayer()->Input_Sprint(true);

	RCO* rco = GetRCO();
	if (rco && !isHost) {
		rco->ServerSprintPressed(GetPlayer());
	}
}

void USuperPioneerMovementComponent::Invoke_SprintReleased() {
	GetPlayer()->Input_Sprint(false);

	RCO* rco = GetRCO();
	if (rco && !isHost) {
		rco->ServerSprintReleased(GetPlayer());
	}
}

void USuperPioneerMovementComponent::OnFalling() {
	SetIsFalling(true);
	wasSprintingBeforeSuperSprint = wasNormalSprintingWhenGrounded;
	eligibleForSprintResume = isSuperSprintPressed;
}

void USuperPioneerMovementComponent::SprintTick(float deltaTime) {
	if (config_superSprintEnabled) {
		if (!isSuperSprinting && !GetPlayerMovementComponent()->IsFalling()) {
			wasNormalSprintingWhenGrounded = GetIsPlayerSprinting();
		}
		if (isSuperSprintPressed && IsSafeToAllowSuperSprinting()) {
			sprintDuration += deltaTime;
			SetPlayerSprintSpeed(CalculateSprintSpeed(sprintDuration));
			SetPlayerMaxStepHeight(CalculateMaxStepHeight(sprintDuration));
		}
		if (!isFalling && GetPlayerMovementComponent()->IsFalling()) {
			OnFalling();
		}
		if (needToRestartSuperSprint) {
			Invoke_SprintPressed();
			needToRestartSuperSprint = false;
		}
	}
}

void USuperPioneerMovementComponent::ResetSprintToDefaults() {
	SetPlayerSprintSpeed(defaultMaxSprintSpeed);
	SetPlayerMaxStepHeight(defaultMaxStepHeight);
	sprintDuration = 0.0;
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
	return GetPlayerMovementComponent()->Velocity.Size();
}

bool USuperPioneerMovementComponent::GetIsPlayerSprinting() {
	return GetPlayerMovementComponent()->GetIsSprinting();
}

bool USuperPioneerMovementComponent::GetIsHoldToSprintEnabled() {
	return GetPlayerMovementComponent()->mHoldToSprint;
}

float USuperPioneerMovementComponent::CalculateCurrentSpeedPercentOfMax() {
	float currentSpeed = GetPlayerCurrentSprintSpeed();
	float minSpeed = defaultMaxSprintSpeed;
	float maxSpeed = config_superSprintMaxSpeed;
	return std::clamp((currentSpeed - minSpeed) / std::max((maxSpeed - minSpeed),0.1f), 0.0f, 1.0f);
}

// Jumping

bool USuperPioneerMovementComponent::IsSafeToAllowSuperJumping() {
	return IsSafeToAllowPowers() && (!isFalling || !isHoverPackEquipped);
}

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
	GetPlayer()->Jump();
}

void USuperPioneerMovementComponent::JumpReleased() {
	if (config_superJumpChargingEnabled && !GetPlayerMovementComponent()->IsSwimming() && CheckIfJumpSafe()) {
		if (customAnimInstance) {
			customAnimInstance->jumpMagnitude = CalculateCurrentJumpHoldPercentOfMax();
		}
		if (isHoverPackEquipped) {
			ChangeCustomAnimationState(ESPAnimState::VANILLA);
		} else {
			ChangeCustomAnimationState(ESPAnimState::JUMP_LEAPING);
		}
		ApplyJumpModifiers();
		isJumpPrimed = true;
		GetPlayer()->Jump();
		Invoke_Jump();
	} else {
		ChangeCustomAnimationState(ESPAnimState::VANILLA);
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

void USuperPioneerMovementComponent::ResetJumpModifiers() {
	SetPlayerJumpZVelocity(defaultJumpZVelocity);
	SetPlayerAirControl(defaultAirControl);
	SetPlayerGravityScale(defaultGravityScale);
}

bool USuperPioneerMovementComponent::CheckIfJumpSafe() {
	return GetPlayer()->CanJumpInternal_Implementation() && IsSafeToAllowSuperJumping();
}

void USuperPioneerMovementComponent::Invoke_Jump() {
	GetPlayerMovementComponent()->DoJump(false);

	RCO* rco = GetRCO();
	if (rco && !isHost) {
		rco->ServerDoJump(GetPlayer());
	}
}

void USuperPioneerMovementComponent::OnLanded() {
	bool wasGroundSlamming = isGroundSlamming;
	ResetJumpModifiers();
	ChangeCustomAnimationState(ESPAnimState::VANILLA);
	if (eligibleForSprintResume && isSuperSprintPressed) {
		// Allow sprint to continue
	}	else if (isSuperSprintPressed) {
		StartSuperSprint();
	} else {
		EndSuperSprint();
	}
	if (isGroundSlamming && !isSuperSprinting) {
		SetPlayerDeceleration(config_groundSlamGroundFriction);
	}
	eligibleForSprintResume = false;
	SetIsFalling(false);
	isGroundSlamming = false;
	if (wasGroundSlamming && isCrouchPressed) {
		GetPlayer()->Input_Crouch(true);
	}
}

void USuperPioneerMovementComponent::CheckForHoverPackLand(EMovementMode previousMovementMode, uint8 previousCustomMode, EMovementMode newMovementMode, uint8 newCustomMode) {
	if (previousMovementMode == EMovementMode::MOVE_Custom && previousCustomMode == 4 && newMovementMode == EMovementMode::MOVE_Walking) {
		OnLanded();
	}
}

void USuperPioneerMovementComponent::JumpTick(float deltaTime) {
	if (config_superJumpChargingEnabled) {
		if (isJumpPressed && IsSafeToAllowSuperJumping()) {
			jumpHoldDuration += deltaTime;
			if (customAnimInstance && jumpHoldDuration > config_superJumpHoldTimeMin && (!isFalling || !isHoverPackEquipped)) {
				ChangeCustomAnimationState(ESPAnimState::JUMP_CHARGING);
				customAnimInstance->maxJumpChargeDuration = config_superJumpHoldTimeMax - config_superJumpHoldTimeMin;
			}
		}
		UpdateJumpChargeIndicator();
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

void USuperPioneerMovementComponent::UpdateJumpChargeIndicator() {
	if (config_superJumpChargingUIEnabled && !isJumpChargeIndicatorVisible && isJumpPressed && jumpHoldDuration > config_superJumpHoldTimeMin && IsSafeToAllowSuperJumping()) {
		if (reticleHUD) {
			reticleHUD->ShowJumpChargeIndicator();
			reticleHUD->SetJumpChargeIndicatorValue(CalculateCurrentJumpHoldPercentOfMax());
			isJumpChargeIndicatorVisible = true;
		}
	} else if (isJumpChargeIndicatorVisible && (!isJumpPressed || !IsValid(inputComponent) || !IsSafeToAllowSuperJumping())) {
		if (reticleHUD) {
			reticleHUD->HideJumpChargeIndicator();
			isJumpChargeIndicatorVisible = false;
		}
	} else if (isJumpChargeIndicatorVisible) {
		if (reticleHUD) {
			reticleHUD->SetJumpChargeIndicatorValue(CalculateCurrentJumpHoldPercentOfMax());
		}
	}
}

// Ground Slam

bool USuperPioneerMovementComponent::IsSafeToAllowGroundSlam() {
	return IsSafeToAllowPowers() && !wasHoverPackHoveringLastTick;
}

bool USuperPioneerMovementComponent::AttemptGroundSlam() {
	if (!IsSafeToAllowGroundSlam()) { return false; }
	isCrouchPressed = true;
	if (config_groundSlamEnabled) {
		FVector v = GetPlayer()->GetCameraComponentForwardVector();
		if (IsEligibleForGroundSlam()) {
			groundSlamDirection = GetPlayer()->GetCameraComponentForwardVector();
			if (isGroundSlamming) {
				GroundSlamLaunch(groundSlamDirection * GetPlayerMovementComponent()->Velocity.Size());
			} else {
				SetPlayerGravityScale(0.0f);
				GroundSlamLaunch(groundSlamDirection * config_groundSlamInitialVelocity);
				isGroundSlamming = true;
			}
			if (reticleHUD) {
				reticleHUD->ActivateGroundSlam();
			}
			ChangeCustomAnimationState(ESPAnimState::SLAM_FLYING);
			if (customAnimInstance) {
				customAnimInstance->groundSlamVector = GetPlayer()->GetCameraComponentForwardVector();
			}
		}
	}
	return isGroundSlamming;
}

bool USuperPioneerMovementComponent::IsEligibleForGroundSlam() {
	return isFalling && IsSafeToAllowGroundSlam() && (GetPlayer()->GetCameraComponentForwardVector().Z < -1.0f + (config_groundSlamMaxAngle / 90.0f));
}

void USuperPioneerMovementComponent::GroundSlamTick(float deltaTime) {
	if (config_groundSlamEnabled) {
		if (isGroundSlamming) {
			if (IsHoverPackHovering()) {
				OnLanded(); // Hover started, cancel ground slam
			} else {
				GroundSlamAddForce(groundSlamDirection * config_groundSlamAcceleration);
			}
		}
		UpdateGroundSlamIndicator();
	}
}

void USuperPioneerMovementComponent::UpdateGroundSlamIndicator() {
	if (config_groundSlamUIEnabled && !isGroundSlamIndicatorVisible && IsEligibleForGroundSlam()) {
		if (reticleHUD) {
			reticleHUD->ShowGroundSlamIndicator();
			isGroundSlamIndicatorVisible = true;
		}
	} else if (isGroundSlamIndicatorVisible && (!IsEligibleForGroundSlam() || !IsValid(inputComponent))) {
		if (reticleHUD) {
			reticleHUD->HideGroundSlamIndicator();
			isGroundSlamIndicatorVisible = false;
		}
	}
}

void USuperPioneerMovementComponent::GroundSlamLaunch(FVector vector) {
	GetPlayerMovementComponent()->Launch(vector);

	RCO* rco = GetRCO();
	if (rco && !isHost) {
		rco->ServerLaunch(GetPlayer(), vector);
	}
}

void USuperPioneerMovementComponent::GroundSlamAddForce(FVector force) {
	GetPlayerMovementComponent()->AddForce(force);

	RCO* rco = GetRCO();
	if (rco && !isHost) {
		rco->ServerAddForce(GetPlayer(), force);
	}
}

void USuperPioneerMovementComponent::CrouchReleased() {
	isCrouchPressed = false;
}

// Utilities

float USuperPioneerMovementComponent::lerp(float a, float b, float t) {
	return a * (1.0 - t) + (b * t);
}

UUserWidget* USuperPioneerMovementComponent::GetGameUI() {
	if (AController* controller = GetPlayer()->Controller) {
		if (AFGPlayerController* playerController = Cast<AFGPlayerController>(controller)) {
			if (AFGHUD* hud = playerController->GetHUD<AFGHUD>()) {
				return hud->GetGameUI();
			}
		}
	}
	return nullptr;
}

bool USuperPioneerMovementComponent::IsHoverPackHovering() {
	if (isHoverPackEquipped) {
		if (AFGEquipment* bodyEquipment = GetPlayer()->GetEquipmentInSlot(EEquipmentSlot::ES_BACK)) {
			if (AFGHoverPack* hoverPackEquipment = Cast<AFGHoverPack>(bodyEquipment)) {
				return hoverPackEquipment && (hoverPackEquipment->GetCurrentHoverMode() == EHoverPackMode::HPM_Hover);
			}
		}
	}
	return false;
}