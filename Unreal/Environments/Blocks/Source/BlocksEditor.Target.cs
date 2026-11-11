// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class BlocksEditorTarget : TargetRules
{
	public BlocksEditorTarget(TargetInfo Target) : base(Target)
	{
	    DefaultBuildSettings = BuildSettingsVersion.V5;
        Type = TargetType.Editor;
		ExtraModuleNames.AddRange(new string[] { "Blocks" });
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        //bUseUnityBuild = false;
        //bUsePCHFiles = false;
        // The bundled rpclib msgpack preprocessor headers do not support the
        // conforming MSVC preprocessor, which UBT enables by default in UE 5.5
        if (Target.Platform == UnrealTargetPlatform.Win64) {
            AdditionalCompilerArguments = "/Zc:preprocessor-";
            bOverrideBuildEnvironment = true;
        }
    }
}
