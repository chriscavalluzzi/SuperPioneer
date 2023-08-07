using UnrealBuildTool;
using System.IO;
using System;

public class SuperPioneer : ModuleRules
{
    public SuperPioneer(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // FactoryGame transitive dependencies
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject",
            "Engine",
            "DeveloperSettings",
            "PhysicsCore",
            "InputCore",
            "OnlineSubsystem", "OnlineSubsystemUtils", "OnlineSubsystemNull",
            "SignificanceManager",
            "GeometryCollectionEngine",
            "ChaosVehiclesCore", "ChaosVehicles", "ChaosSolverEngine",
            "AnimGraphRuntime",
            "AkAudio",
            "AssetRegistry",
            "NavigationSystem",
            "ReplicationGraph",
            "AIModule",
            "GameplayTasks",
            "SlateCore", "Slate", "UMG",
            "InstancedSplines",
            "RenderCore",
            "CinematicCamera",
            "Foliage",
            "Niagara",
            "EnhancedInput",
            "GameplayCameras",
            "TemplateSequence",
            "NetCore",
            "GameplayTags",
            "AnimGraphRuntime",
            "Json",
        });

        // FactoryGame plugins
        PublicDependencyModuleNames.AddRange(new[] {
            "AbstractInstance",
            "InstancedSplinesComponent",
            "SignificanceISPC"
        });

        // Header stubs
        PublicDependencyModuleNames.AddRange(new[] {
            "DummyHeaders",
        });

        if (Target.Type == TargetRules.TargetType.Editor) {
			PublicDependencyModuleNames.AddRange(new string[] {"OnlineBlueprintSupport", "AnimGraph"});
		}
        PublicDependencyModuleNames.AddRange(new string[] {"FactoryGame", "SML"});
    }
}