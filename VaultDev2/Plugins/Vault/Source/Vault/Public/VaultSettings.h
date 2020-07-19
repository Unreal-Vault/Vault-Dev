// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

// Info for the Vault Settings. Stored in JSON and assessable via this struct
struct FVaultSettings
{
public:

	/** Singleton Accessor */
	static FVaultSettings& Get();

	void Initialize();

	// Read Local Settings file return the json object
	TSharedPtr<FJsonObject> GetVaultLocalSettings();

	// Read Global Settings file using the local to find it, and return the json object
	TSharedPtr<FJsonObject> GetVaultGlobalSettings();

	bool SaveVaultTags(TSet<FString> NewTags);

	bool ReadVaultTags(TSet<FString>& OutTags);

	FString GetAssetLibraryRoot();

private:

	// Easy write to file
	bool WriteJsonObjectToFile(TSharedPtr<FJsonObject> JsonFile, FString FilepathFull);

	// Generates the default local settings file for new installs
	void GenerateBaseLocalSettingsFile();

	// Generates the default global settings file for new installs
	void GenerateBaseGlobalSettingsFile();

	// Generates the default tags file for new installs
	void GenerateBaseTagPoolFile();
	
	// Json Reusable Functions
	FString GetGlobalSettingsFilePathFull();

	// Json Reusable Functions
	FString GetGlobalTagsPoolFilePathFull();

	// Json Reusable Functions
	FString GetVaultPluginVersion();

	// Updates elements of the JSON files that should change on init.
	void UpdateVaultFiles();

};
