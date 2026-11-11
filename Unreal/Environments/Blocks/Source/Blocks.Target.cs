// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class BlocksTarget : TargetRules
{
	public BlocksTarget(TargetInfo Target) : base(Target)
	{
        DefaultBuildSettings = BuildSettingsVersion.V5;
        Type = TargetType.Game;
		ExtraModuleNames.AddRange(new string[] { "Blocks" });
		//bUseUnityBuild = false;
		if (Target.Platform == UnrealTargetPlatform.Linux)
			bUsePCHFiles = false;
		// The bundled rpclib msgpack preprocessor headers do not support the
		// conforming MSVC preprocessor, which UBT enables by default in UE 5.5
		if (Target.Platform == UnrealTargetPlatform.Win64) {
			AdditionalCompilerArguments = "/Zc:preprocessor-";
			bOverrideBuildEnvironment = true;
		}
	}
}
