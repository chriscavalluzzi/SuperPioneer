#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "SuperPioneer_ConfigStruct.generated.h"

struct FSuperPioneer_ConfigStruct_superJump;
struct FSuperPioneer_ConfigStruct_superSprint;

USTRUCT(BlueprintType)
struct FSuperPioneer_ConfigStruct_superJump {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    float superJumpSpeedMultiplierMax;
};

USTRUCT(BlueprintType)
struct FSuperPioneer_ConfigStruct_superSprint {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    float superSprintMaxSpeed;
};

/* Struct generated from Mod Configuration Asset '/SuperPioneer/SuperPioneer_Config' */
USTRUCT(BlueprintType)
struct FSuperPioneer_ConfigStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    FSuperPioneer_ConfigStruct_superJump superJump;

    UPROPERTY(BlueprintReadWrite)
    FSuperPioneer_ConfigStruct_superSprint superSprint;

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FSuperPioneer_ConfigStruct GetActiveConfig() {
        FSuperPioneer_ConfigStruct ConfigStruct{};
        FConfigId ConfigId{"SuperPioneer", ""};
        UConfigManager* ConfigManager = GEngine->GetEngineSubsystem<UConfigManager>();
        ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FSuperPioneer_ConfigStruct::StaticStruct(), &ConfigStruct});
        return ConfigStruct;
    }
};

