#include "SuperPioneerRemoteCallObject.h"

void USuperPioneerRemoteCallObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USuperPioneerRemoteCallObject, bDummy);
}

UFGCharacterMovementComponent* USuperPioneerRemoteCallObject::GetMovementComponent(AActor* actor) {
	TArray<UFGCharacterMovementComponent*> components;
	actor->GetComponents<UFGCharacterMovementComponent>(components);
	for (int i = 0; i < components.Num(); i++) {
		return components[i];
	}
	return nullptr;
}

void USuperPioneerRemoteCallObject::ServerSetSprintSpeed_Implementation(AActor* player, float newMaxSprintSpeed) {
	UFGCharacterMovementComponent* component = GetMovementComponent(player);
	UE_LOG(LogTemp, Warning, TEXT("[SP] >>>>>>>>>> RECEIVED: %f"), newMaxSprintSpeed)
	if (component) {
		component->mMaxSprintSpeed = newMaxSprintSpeed;
		UE_LOG(LogTemp, Warning, TEXT("[SP] >>>>>>>>>> APPLIED"))
	}
}

bool USuperPioneerRemoteCallObject::ServerSetSprintSpeed_Validate(AActor* player, float newMaxSprintSpeed) {
  return true;
}