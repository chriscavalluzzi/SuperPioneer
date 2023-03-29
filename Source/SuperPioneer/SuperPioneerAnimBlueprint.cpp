#include "SuperPioneerAnimBlueprint.h"

FPoseSnapshot USuperPioneerAnimBlueprint::GetVanillaPose() const {
  FPoseSnapshot snapshot;
  if (vanillaAnimInstance) {
    vanillaAnimInstance->SnapshotPose(snapshot);
  }
  return snapshot;
}