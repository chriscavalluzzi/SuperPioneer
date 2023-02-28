


#include "SuperSprintManager.h"
#include "FGCharacterPlayer.h"

USuperSprintManager::USuperSprintManager() {
  UE_LOG(LogTemp, Warning, TEXT("[SP] Sprint Manager Construction"))
};

void USuperSprintManager::Setup(AFGCharacterPlayer* _localPlayer, UInputComponent* _inputComponent) {
  UE_LOG(LogTemp,Warning,TEXT("[SP] Starting Manager Setup"))
  this->localPlayer = _localPlayer;
  this->inputComponent = _inputComponent;
  _inputComponent->BindAction("SuperPioneer.SuperSprint", EInputEvent::IE_Pressed, this, &USuperSprintManager::SprintPressed);
}

void USuperSprintManager::SprintPressed() {
  UE_LOG(LogTemp, Warning, TEXT("[SP] SUPER SPRINT PRESSED!!!!!!!!!!"))
}