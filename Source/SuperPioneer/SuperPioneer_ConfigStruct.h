#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "SuperPioneer_ConfigStruct.generated.h"

struct FSuperPioneer_ConfigStruct_superSprint;
struct FSuperPioneer_ConfigStruct_superJumpCharging;
struct FSuperPioneer_ConfigStruct_superJumpModifications;
struct FSuperPioneer_ConfigStruct_superSprint_keybindHint;

USTRUCT(BlueprintType)
struct FSuperPioneer_ConfigStruct_superSprint_keybindHint {
    GENERATED_BODY()
public:
};

USTRUCT(BlueprintType)
struct FSuperPioneer_ConfigStruct_superSprint {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    bool superSprintEnabled;

    UPROPERTY(BlueprintReadWrite)
    float superSprintMaxSpeed;

    UPROPERTY(BlueprintReadWrite)
    FSuperPioneer_ConfigStruct_superSprint_keybindHint keybindHint;
};

USTRUCT(BlueprintType)
struct FSuperPioneer_ConfigStruct_superJumpCharging {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    bool jumpChargingEnabled;

    UPROPERTY(BlueprintReadWrite)
    float superJumpHoldMultiplierMax;

    UPROPERTY(BlueprintReadWrite)
    float superJumpHoldTimeMin;

    UPROPERTY(BlueprintReadWrite)
    float superJumpHoldTimeMax;
};

USTRUCT(BlueprintType)
struct FSuperPioneer_ConfigStruct_superJumpModifications {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    bool jumpModificationsEnabled;

    UPROPERTY(BlueprintReadWrite)
    float superJumpSpeedMultiplierMax;

    UPROPERTY(BlueprintReadWrite)
    float maxAirControl;

    UPROPERTY(BlueprintReadWrite)
    float gravityScalingMultiplier;

    UPROPERTY(BlueprintReadWrite)
    float jumpMultiplierPerGravityScale;
};

/* Struct generated from Mod Configuration Asset '/SuperPioneer/SuperPioneer_Config' */
USTRUCT(BlueprintType)
struct FSuperPioneer_ConfigStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    FSuperPioneer_ConfigStruct_superSprint superSprint;

    UPROPERTY(BlueprintReadWrite)
    FSuperPioneer_ConfigStruct_superJumpCharging superJumpCharging;

    UPROPERTY(BlueprintReadWrite)
    FSuperPioneer_ConfigStruct_superJumpModifications superJumpModifications;

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FSuperPioneer_ConfigStruct GetActiveConfig() {
        FSuperPioneer_ConfigStruct ConfigStruct{};
        FConfigId ConfigId{"SuperPioneer", ""};
        UConfigManager* ConfigManager = GEngine->GetEngineSubsystem<UConfigManager>();
        ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FSuperPioneer_ConfigStruct::StaticStruct(), &ConfigStruct});
        return ConfigStruct;
    }
};

