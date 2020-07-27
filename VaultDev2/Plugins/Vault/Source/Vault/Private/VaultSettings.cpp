// Copyright Daniel Orchard 2020

#include "VaultSettings.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "JsonUtilities/Public/JsonUtilities.h"
#include "Serialization\JsonReader.h"
#include <Interfaces/IPluginManager.h>
#include "Misc/AssertionMacros.h"
#include "SVaultSetupWizard.h" // Our First time setup window

// For simplicity in changing keys and looking them up, here's all the keys

// Folder Names

// These 2 vars registered in header for access elsewhere.
const FString FVaultSettings::DefaultVaultSettingsFolder("Vault");
const FString FVaultSettings::DefaultGlobalsPath(FPlatformProcess::UserDir() + DefaultVaultSettingsFolder);

// Filenames
static const FString GlobalSettingsFilename = "VaultGlobalSettings.json";
static const FString GlobalTagPoolFilename = "VaultTags.json";
static const FString LocalSettingsFilename = "VaultLocalSettings.json";

// File paths
//static const FString DefaultGlobalsPath = FPlatformProcess::UserDir() + DefaultVaultSettingsFolder;
static const FString DefaultGlobalTagsPath = FPlatformProcess::UserDir() + FVaultSettings::DefaultVaultSettingsFolder;
static const FString VaultPluginRoot = IPluginManager::Get().FindPlugin(TEXT("Vault"))->GetBaseDir();
static const FString LocalSettingsFilePathFull = VaultPluginRoot / LocalSettingsFilename;
static const FString GlobalSettingsFilePathFull = FVaultSettings::DefaultGlobalsPath / GlobalSettingsFilename;

// Json Cloud, just to make it easy to update and refer back too:
static const FString TagsKey = "Tags";
static const FString TagArrayKey = "TagLibrary";
static const FString GlobalSettingsPathKey = "GlobalSettingsPath";
static const FString GlobalTagsPoolPathKey = "GlobalTagsPoolPath";
static const FString VaultVersionKey = "Version";
static const FString LibraryPath = "LibraryPath";

// Singleton Accessor
FVaultSettings& FVaultSettings::Get()
{
	static FVaultSettings VaultSettings;
	return VaultSettings;
}


// Init, set up files and folders etc. Called from Plugin startup
void FVaultSettings::Initialize()
{
	// Read Local Settings
	FString LocalSettingsRaw;

	bool bShouldRunUpdater = false;

	bool bLoadedLocalSettings = FFileHelper::LoadFileToString(LocalSettingsRaw, *LocalSettingsFilePathFull);

	if (bLoadedLocalSettings && LocalSettingsRaw.IsEmpty() == false)
	{
		bShouldRunUpdater = true;
	}
	else
	{
		// No Default Local Settings, generate a fresh copy with defaults
		GenerateBaseLocalSettingsFile();
	}

	// Global Settings Setup and Test
	FString GlobalSettingsRaw;
	bool bLoadedGlobalSettings = FFileHelper::LoadFileToString(GlobalSettingsRaw, *GetGlobalSettingsFilePathFull());

	if (bLoadedGlobalSettings && GlobalSettingsRaw.IsEmpty() == false)
	{
		bShouldRunUpdater = true;
	}
	else
	{
		GenerateBaseGlobalSettingsFile();
	}
	
	FString GlobalTagsRaw;
	bool bLoadedTagPool = FFileHelper::LoadFileToString(GlobalTagsRaw, *GetGlobalTagsPoolFilePathFull());

	if (bLoadedTagPool)
	{
		bShouldRunUpdater = true;
	}
	else
	{
		GenerateBaseTagPoolFile();
	}

	if (bShouldRunUpdater)
	{
		UpdateVaultFiles();
	}

	if (!bLoadedLocalSettings || !bLoadedGlobalSettings || !bLoadedTagPool)
	{
		IsEditorInitialized = false;
		FSlateRenderer* SlateRenderer = FSlateApplication::Get().GetRenderer();
		LoadedDelegateHandle = SlateRenderer->OnSlateWindowRendered().AddRaw(this, &FVaultSettings::OnEditorLoaded);



	}
	


}

// Get vault local
TSharedPtr<FJsonObject> FVaultSettings::GetVaultLocalSettings()
{
	FString LocalSettingsRaw;
	FFileHelper::LoadFileToString(LocalSettingsRaw, *LocalSettingsFilePathFull);
	TSharedPtr<FJsonObject> JsonLocalSettings = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(LocalSettingsRaw);
	FJsonSerializer::Deserialize(JsonReader, JsonLocalSettings);
	return JsonLocalSettings;
}

TSharedPtr<FJsonObject> FVaultSettings::GetVaultGlobalSettings()
{
	FString SettingsTextRaw;
	FFileHelper::LoadFileToString(SettingsTextRaw, *GetGlobalSettingsFilePathFull());
	TSharedPtr<FJsonObject> JsonSettings = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(SettingsTextRaw);
	FJsonSerializer::Deserialize(JsonReader, JsonSettings);
	return JsonSettings;
}

// Public - Get Asset Library Root Accessor
FString FVaultSettings::GetAssetLibraryRoot()
{
	TSharedPtr<FJsonObject> SettingsObj = GetVaultGlobalSettings();

	if (SettingsObj.IsValid())
	{
		return SettingsObj->GetStringField(LibraryPath);
	}
	return FString();
}

// Write any json file out to a file.
bool FVaultSettings::WriteJsonObjectToFile(TSharedPtr<FJsonObject> JsonFile, FString FilepathFull)
{
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonFile.ToSharedRef(), Writer);

	return FFileHelper::SaveStringToFile(OutputString, *FilepathFull);
}

void FVaultSettings::GenerateBaseLocalSettingsFile()
{
	TSharedPtr<FJsonObject> JsonLocalSettings = MakeShareable(new FJsonObject());

	JsonLocalSettings->SetStringField(VaultVersionKey, GetVaultPluginVersion());
	JsonLocalSettings->SetStringField(GlobalSettingsPathKey, (DefaultGlobalsPath / GlobalSettingsFilename));
	JsonLocalSettings->SetStringField(GlobalTagsPoolPathKey, (DefaultGlobalTagsPath / GlobalTagPoolFilename));
	JsonLocalSettings->SetBoolField(TEXT("ClearPackageListOnSuccessfulPackage"), false);


	FString TempPath = FPlatformMisc::GetEnvironmentVariable(TEXT("TEMP"));
	FPaths::NormalizeDirectoryName(TempPath);

	JsonLocalSettings->SetStringField(TEXT("PackageListStoragePath"), TempPath);
	JsonLocalSettings->SetStringField(TEXT("PackageLogPath"), TempPath);

	WriteJsonObjectToFile(JsonLocalSettings, LocalSettingsFilePathFull);






}

void FVaultSettings::GenerateBaseGlobalSettingsFile()
{
	TSharedPtr<FJsonObject> JsonGlobalSettings = MakeShareable(new FJsonObject());
	JsonGlobalSettings->SetStringField(VaultVersionKey, GetVaultPluginVersion());
	JsonGlobalSettings->SetStringField(LibraryPath, DefaultGlobalsPath);
	WriteJsonObjectToFile(JsonGlobalSettings, GetGlobalSettingsFilePathFull());
}

void FVaultSettings::GenerateBaseTagPoolFile()
{
	TSet<FString> PlaceholderTag = { "Environment", "Prop", "Character", "Testing", "Developer" };
	SaveVaultTags(PlaceholderTag);
}

FString FVaultSettings::GetGlobalSettingsFilePathFull()
{
	FString LocalSettingsRaw;
	FFileHelper::LoadFileToString(LocalSettingsRaw, *LocalSettingsFilePathFull);
	TSharedPtr<FJsonObject> JsonLocalSettings = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(LocalSettingsRaw);
	FJsonSerializer::Deserialize(JsonReader, JsonLocalSettings);
	FString SettingsPath = JsonLocalSettings->GetStringField(GlobalSettingsPathKey);

	if (SettingsPath.IsEmpty())
	{
		return (DefaultGlobalsPath / GlobalSettingsFilename);
	}
	return SettingsPath;
}

FString FVaultSettings::GetGlobalTagsPoolFilePathFull()
{
	// We get the global tags path from the local settings, so lets load that
	FString LocalSettingsRaw;
	FFileHelper::LoadFileToString(LocalSettingsRaw, *LocalSettingsFilePathFull);
	TSharedPtr<FJsonObject> JsonLocalSettings = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(LocalSettingsRaw);
	FJsonSerializer::Deserialize(JsonReader, JsonLocalSettings);
	return JsonLocalSettings->GetStringField(GlobalTagsPoolPathKey);
}

FString FVaultSettings::GetVaultPluginVersion()
{
	return IPluginManager::Get().FindPlugin(TEXT("Vault"))->GetDescriptor().VersionName;
}

void FVaultSettings::UpdateVaultFiles()
{
	TSharedPtr<FJsonObject> Local = GetVaultLocalSettings();
	TSharedPtr<FJsonObject> Global = GetVaultGlobalSettings();

	Local->SetStringField("Version", GetVaultPluginVersion());
	Global->SetStringField("Version", GetVaultPluginVersion());

	WriteJsonObjectToFile(Local, LocalSettingsFilePathFull);
	WriteJsonObjectToFile(Global, GetGlobalSettingsFilePathFull());


}

void FVaultSettings::OnEditorLoaded(SWindow& SlateWindow, void* ViewportRHIPtr)
{
	if (GEditor == nullptr)
	{
		return;
	}

	if (IsInGameThread())
	{
		FSlateRenderer* SlateRenderer = FSlateApplication::Get().GetRenderer();
		SlateRenderer->OnSlateWindowRendered().Remove(LoadedDelegateHandle);
	}

	if (IsEditorInitialized)
	{
		return;
	}
	IsEditorInitialized = true;


	// Generate our new users Message here:
	GEditor->EditorAddModalWindow(SNew(SVaultSetupWizard));
}

bool FVaultSettings::SaveVaultTags(TSet<FString> NewTags)
{
	// A set of new tags to save, pulls previous tags into it so the new ones can amend in.
	TSet<FString> TagsToSave;

	ReadVaultTags(TagsToSave);

	// Json Object to be written
	TSharedPtr<FJsonObject> JsonTags = MakeShareable(new FJsonObject());

	// Append our new tags to old. Since they are Sets, there wont be any duplicates
	TagsToSave.Append(NewTags);

	// Sort Alphabetically Lambda.
	TagsToSave.Sort([](const FString& A, const FString& B)
	{
		return A > B;
	});

	// Convert FString into JsonValues
	TArray<TSharedPtr<FJsonValue>> TagElements;
	for (FString Tag : TagsToSave)
	{
		TagElements.Add(MakeShareable(new FJsonValueString(Tag)));
	}

	// Store tags into an array
	JsonTags->SetArrayField(TagArrayKey, TagElements);

	return WriteJsonObjectToFile(JsonTags, GetGlobalTagsPoolFilePathFull());

}

bool FVaultSettings::ReadVaultTags(TSet<FString>& OutTags)
{
	OutTags.Empty();

	// Holder to read the file into
	FString JsonString;

	// We need to find our path to the vault tags file
	bool bLoadedFile = FFileHelper::LoadFileToString(JsonString, *GetGlobalTagsPoolFilePathFull());

	if (bLoadedFile)
	{
		TSharedPtr<FJsonObject> JsonTagsObject = MakeShareable(new FJsonObject());
		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

		if (FJsonSerializer::Deserialize(JsonReader, JsonTagsObject) && JsonTagsObject.IsValid())
		{
			TArray<TSharedPtr<FJsonValue>> TagsArray = JsonTagsObject->GetArrayField(TagArrayKey);

			for (int32 index = 0; index < TagsArray.Num(); index++)
			{
				OutTags.Add(TagsArray[index]->AsString());
			}
			return true;
		}
	}
	return false;
}
