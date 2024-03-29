#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "SuperPioneer_ConfigStruct.generated.h"

struct FSuperPioneer_ConfigStruct_general;
struct FSuperPioneer_ConfigStruct_superSprint;
struct FSuperPioneer_ConfigStruct_superJumpCharging;
struct FSuperPioneer_ConfigStruct_superJumpModifications;
struct FSuperPioneer_ConfigStruct_groundSlam;

USTRUCT(BlueprintType)
struct FSuperPioneer_ConfigStruct_general {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    bool animationsEnabled{};

    UPROPERTY(BlueprintReadWrite)
    bool disableFallDamage{};
};

USTRUCT(BlueprintType)
struct FSuperPioneer_ConfigStruct_superSprint {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    bool superSprintEnabled{};

    UPROPERTY(BlueprintReadWrite)
    float superSprintMaxSpeedMps{};

    UPROPERTY(BlueprintReadWrite)
    float superSprintAccelerationEasing{};

    UPROPERTY(BlueprintReadWrite)
    float superSprintAccelerationMultiplier{};

    UPROPERTY(BlueprintReadWrite)
    float superSprintGroundFriction{};

    UPROPERTY(BlueprintReadWrite)
    float superSprintMaxStepHeight{};
};

USTRUCT(BlueprintType)
struct FSuperPioneer_ConfigStruct_superJumpCharging {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    bool jumpChargingEnabled{};

    UPROPERTY(BlueprintReadWrite)
    bool jumpChargingUIEnabled{};

    UPROPERTY(BlueprintReadWrite)
    float superJumpHoldMultiplierMax{};

    UPROPERTY(BlueprintReadWrite)
    float superJumpHoldTimeMin{};

    UPROPERTY(BlueprintReadWrite)
    float superJumpHoldTimeMax{};
};

USTRUCT(BlueprintType)
struct FSuperPioneer_ConfigStruct_superJumpModifications {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    bool jumpModificationsEnabled{};

    UPROPERTY(BlueprintReadWrite)
    float superJumpSpeedMultiplierMax{};

    UPROPERTY(BlueprintReadWrite)
    float maxAirControl{};

    UPROPERTY(BlueprintReadWrite)
    float gravityScalingMultiplier{};

    UPROPERTY(BlueprintReadWrite)
    float jumpMultiplierPerGravityScale{};

    UPROPERTY(BlueprintReadWrite)
    float swimmingJumpMultiplier{};
};

USTRUCT(BlueprintType)
struct FSuperPioneer_ConfigStruct_groundSlam {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    bool groundSlamEnabled{};

    UPROPERTY(BlueprintReadWrite)
    bool groundSlamUIEnabled{};

    UPROPERTY(BlueprintReadWrite)
    float groundSlamMaxAngle{};

    UPROPERTY(BlueprintReadWrite)
    float groundSlamInitialVelocity{};

    UPROPERTY(BlueprintReadWrite)
    float groundSlamAccelerationV2{};

    UPROPERTY(BlueprintReadWrite)
    float groundSlamMaxSpeed{};

    UPROPERTY(BlueprintReadWrite)
    float groundSlamGroundFriction{};
};

/* Struct generated from Mod Configuration Asset '/SuperPioneer/SuperPioneer_Config' */
USTRUCT(BlueprintType)
struct FSuperPioneer_ConfigStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    FSuperPioneer_ConfigStruct_general general{};

    UPROPERTY(BlueprintReadWrite)
    FSuperPioneer_ConfigStruct_superSprint superSprint{};

    UPROPERTY(BlueprintReadWrite)
    FSuperPioneer_ConfigStruct_superJumpCharging superJumpCharging{};

    UPROPERTY(BlueprintReadWrite)
    FSuperPioneer_ConfigStruct_superJumpModifications superJumpModifications{};

    UPROPERTY(BlueprintReadWrite)
    FSuperPioneer_ConfigStruct_groundSlam groundSlam{};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FSuperPioneer_ConfigStruct GetActiveConfig(UObject* WorldContext) {
        FSuperPioneer_ConfigStruct ConfigStruct{};
        FConfigId ConfigId{"SuperPioneer", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FSuperPioneer_ConfigStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

