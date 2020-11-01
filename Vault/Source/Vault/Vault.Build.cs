// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Vault : ModuleRules
{
	public Vault(ReadOnlyTargetRules Target) : base(Target)
	{
		bLegacyPublicIncludePaths = false; // Omits Sub-Folders from public include paths to reduce compiler command line length. (Previously: true).
		ShadowVariableWarningLevel = WarningLevel.Error; // Treats shadowed variable warnings as errors. (Previously: WarningLevel.Warning).
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"UnrealEd"
			});
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"AppFramework", // colour pickers maybe
				"Projects",
				"InputCore", // custom menus and stuff
				"LevelEditor",
				"EditorStyle",
				"Slate",
				"SlateCore",
				"Json",
				"JsonUtilities",
				"PropertyEditor", // Customised Editor Properties
				"AssetRegistry",
				"PakFileUtilities",
				"DesktopPlatform",
				"ImageWriteQueue"
			});
	}
}
