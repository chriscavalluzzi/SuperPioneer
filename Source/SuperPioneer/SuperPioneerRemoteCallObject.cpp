#include "SuperPioneerRemoteCallObject.h"
#include "Net/UnrealNetwork.h"

void USuperPioneerRemoteCallObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USuperPioneerRemoteCallObject, bDummy);
	DOREPLIFETIME(USuperPioneerRemoteCallObject, fallDamageDisabled);
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

void USuperPioneerRemoteCallObject::ServerSetMaxStepHeight_Implementation(AFGCharacterPlayer* player, float newMaxStepHeight) {
	UFGCharacterMovementComponent* component = GetMovementComponent(player);
	if (component) {
		component->MaxStepHeight = newMaxStepHeight;
	}
}

void USuperPioneerRemoteCallObject::ServerSetDeceleration_Implementation(AFGCharacterPlayer* player, float newGroundFriction) {
	UFGCharacterMovementComponent* component = GetMovementComponent(player);
	if (component) {
		component->GroundFriction = newGroundFriction;
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

void USuperPioneerRemoteCallObject::ServerSetFallDamageDisabled_Implementation(bool newFallDamageDisabled) {
	this->fallDamageDisabled = newFallDamageDisabled;
}

void USuperPioneerRemoteCallObject::ServerSprintPressed_Implementation(AFGCharacterPlayer* player) {
	player->Input_Sprint(true);
}

void USuperPioneerRemoteCallObject::ServerSprintReleased_Implementation(AFGCharacterPlayer* player) {
	player->Input_Sprint(false);
}

void USuperPioneerRemoteCallObject::ServerDoJump_Implementation(AFGCharacterPlayer* player) {
	UFGCharacterMovementComponent* component = GetMovementComponent(player);
	if (component) {
		component->DoJump(false);
	}
}

void USuperPioneerRemoteCallObject::ServerLaunch_Implementation(AFGCharacterPlayer* player, FVector const& vector) {
	UFGCharacterMovementComponent* component = GetMovementComponent(player);
	if (component) {
		component->Launch(vector);
	}
}

void USuperPioneerRemoteCallObject::ServerAddForce_Implementation(AFGCharacterPlayer* player, FVector force) {
	UFGCharacterMovementComponent* component = GetMovementComponent(player);
	if (component) {
		component->AddForce(force);
	}
}

bool USuperPioneerRemoteCallObject::GetFallDamageDisabled() {
	return fallDamageDisabled;
}