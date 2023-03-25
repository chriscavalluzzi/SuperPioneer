#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SuperPioneerAnimBlueprint.generated.h"

UENUM(BlueprintType)
enum class ESPAnimState : uint8 {
	RETURN = 0			UMETA(DisplayName = "Return to regular BP"),
	JUMP_CHARGING		UMETA(DisplayName = "Jump - Charging"),
	JUMP_LEAPING		UMETA(DisplayName = "Jump - Leaping"),
	SLAM_PREP				UMETA(DisplayName = "Slam - Prep (not currently used)"),
	SLAM_FLYING			UMETA(DisplayName = "Slam - Flying"),
	SLAM_LANDING		UMETA(DisplayName = "Slam - Landing"),
};

UCLASS()
class SUPERPIONEER_API USuperPioneerAnimBlueprint : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPoseSnapshot entryPose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESPAnimState animState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector groundSlamVector;
};