#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SuperPioneerAnimBlueprint.generated.h"

UENUM(BlueprintType)
enum class ESPAnimState : uint8 {
	VANILLA = 0			UMETA(DisplayName = "Vanilla"),
	JUMP_CHARGING		UMETA(DisplayName = "Jump - Charging"),
	JUMP_LEAPING		UMETA(DisplayName = "Jump - Leaping"),
	SLAM_FLYING			UMETA(DisplayName = "Slam - Flying"),
	SLAM_LANDING		UMETA(DisplayName = "Slam - Landing"),
};

UCLASS()
class SUPERPIONEER_API USuperPioneerAnimBlueprint : public UAnimInstance
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESPAnimState animState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector groundSlamVector;

	UAnimInstance* vanillaAnimInstance;

	UFUNCTION(BlueprintPure)
	FPoseSnapshot GetVanillaPose() const;
};