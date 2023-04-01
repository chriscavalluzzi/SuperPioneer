#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Equipment/FGEquipment.h"
#include "SuperPioneerAnimBlueprint.generated.h"

UENUM(BlueprintType)
enum class ESPAnimState : uint8 {
	VANILLA = 0			UMETA(DisplayName = "Vanilla"),
	JUMP_CHARGING		UMETA(DisplayName = "Jump - Charging"),
	JUMP_LEAPING		UMETA(DisplayName = "Jump - Leaping"),
	SLAM_FLYING			UMETA(DisplayName = "Slam"),
};

UCLASS()
class SUPERPIONEER_API USuperPioneerAnimBlueprint : public UAnimInstance
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FPoseSnapshot vanillaPose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESPAnimState animState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool armsEquipmentActive;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf< AFGEquipment > armsEquipmentClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float maxJumpChargeDuration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float jumpMagnitude;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector groundSlamVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool isFalling;

};