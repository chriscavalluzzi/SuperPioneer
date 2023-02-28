

#pragma once

#include "FGCharacterPlayer.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SuperSprintManager.generated.h"

/**
 * 
 */
UCLASS()
class SUPERPIONEER_API USuperSprintManager : public UObject
{
	GENERATED_BODY()

	AFGCharacterPlayer* localPlayer;
	UInputComponent* inputComponent;

public:
	USuperSprintManager();

	void Setup(AFGCharacterPlayer* _localPlayer, UInputComponent* _inputComponent);

	void SprintPressed();

};