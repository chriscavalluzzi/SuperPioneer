#include "SuperPioneerRemoteCallObject.h"

void USuperPioneerRemoteCallObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USuperPioneerRemoteCallObject, bDummy);
}

UFGCharacterMovementComponent* USuperPioneerRemoteCallObject::GetMovementComponent(AFGCharacterPlayer* actor) {
	TArray<UFGCharacterMovementComponent*> components;
	actor->GetComponents<UFGCharacterMovementComponent>(components);
	for (int i = 0; i < components.Num(); i++) {
		return components[i];
	}
	return nullptr;
}

void USuperPioneerRemoteCallObject::ServerSetSprintSpeed_Implementation(AFGCharacterPlayer* player, float newMaxSprintSpeed) {
	UFGCharacterMovementComponent* component = GetMovementComponent(player);
	if (component) {
		component->mMaxSprintSpeed = newMaxSprintSpeed;
	}
}

void USuperPioneerRemoteCallObject::ServerSetJumpZVelocity_Implementation(AFGCharacterPlayer* player, float newZVelocity) {
	UFGCharacterMovementComponent* component = GetMovementComponent(player);
	if (component) {
		component->JumpZVelocity = newZVelocity;
	}
}

void USuperPioneerRemoteCallObject::ServerSetAirControl_Implementation(AFGCharacterPlayer* player, float newAirControl) {
	UFGCharacterMovementComponent* component = GetMovementComponent(player);
	if (component) {
		component->AirControl = newAirControl;
	}
}

void USuperPioneerRemoteCallObject::ServerSetGravityScale_Implementation(AFGCharacterPlayer* player, float newGravityScale) {
	UFGCharacterMovementComponent* component = GetMovementComponent(player);
	if (component) {
		component->GravityScale = newGravityScale;
	}
}

void USuperPioneerRemoteCallObject::ServerSprintPressed_Implementation(AFGCharacterPlayer* player) {
	player->SprintPressed();
}

void USuperPioneerRemoteCallObject::ServerSprintReleased_Implementation(AFGCharacterPlayer* player) {
	player->SprintReleased();
}

void USuperPioneerRemoteCallObject::ServerDoJump_Implementation(AFGCharacterPlayer* player) {
	UFGCharacterMovementComponent* component = GetMovementComponent(player);
	if (component) {
		component->DoJump(false);
	}
}