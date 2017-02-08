// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.IO;

public class VrVisFrontend : ModuleRules
{
    private string ModulePath {
        get { return ModuleDirectory; }
    }

    private string ThirdPartyPath {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
    }

    public VrVisFrontend(TargetInfo Target)
	{

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        LoadSqlite3(Target);
        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }

    public bool LoadSqlite3(TargetInfo Target) {
        bool isApplied = false;

        if ((Target.Platform == UnrealTargetPlatform.Win64)) {
            isApplied = true;
            string LibraryPath = Path.Combine(ThirdPartyPath, "sqlite3", "lib");
            //string LibraryName = "sqlite3";
            if (Target.Platform == UnrealTargetPlatform.Win64) {
                LibraryPath = Path.Combine(LibraryPath, "x64");
            }
            PublicLibraryPaths.Add(LibraryPath);
            PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "SqliteLib.lib"));
            PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "sqlite3", "include"));
        }
        return isApplied;
    }

}
