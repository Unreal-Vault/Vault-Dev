// Copyright Daniel Orchard 2020

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

	// Add new Vault Tags. User choice to opt their new tags into the global library
	bool SaveVaultTags(TSet<FString> NewTags);

	// Read Existing Tags from the JSON Tag file
	bool ReadVaultTags(TSet<FString>& OutTags);

	FText GetDefaultDeveloperName();

	// Get our Asset Library root path, defined in the global settings. 
	FString GetAssetLibraryRoot();

	// Json Reusable Functions
	FString GetGlobalSettingsFilePathFull();

	// Json Reusable Functions
	FString GetVaultPluginVersion();

	static const FString DefaultVaultSettingsFolder;
	static const FString DefaultGlobalsPath;

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
	FString GetGlobalTagsPoolFilePathFull();

	// Updates elements of the JSON files that should change on init.
	void UpdateVaultFiles();

	// Setup Wizard Helper Stuff
	bool IsEditorInitialized;
	FDelegateHandle LoadedDelegateHandle;
	void OnEditorLoaded(SWindow& SlateWindow, void* ViewportRHIPtr);

};
