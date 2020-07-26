// Fill out your copyright notice in the Description page of Project Settings.

#include "AssetPublisher.h"
#include "PakFileUtilities.h"
#include "VaultSettings.h"
#include "Misc/FileHelper.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "MetadataOps.h"
#include "Vault.h"


bool UAssetPublisher::PackageSelected(TSet<FString> PackageObjects, FVaultMetadata& Meta)
{
	UpdateSystemMeta(Meta);

	bool bWipePackageList = FVaultSettings::Get().GetVaultLocalSettings()->GetBoolField(TEXT("ClearPackageListOnSuccessfulPackage"));
	// #todo store in a better place, make name procedural, so it can be kept for archive and even analysis by loader.

	// Dated Filename
	const FString PackageListFilename = TEXT("VaultPackageList_") + Meta.LastModified.ToString() + TEXT(".txt");

	FString ListDirectory;
	bool bFoundListDirectory = FVaultSettings::Get().GetVaultLocalSettings()->TryGetStringField(TEXT("PackageListStoragePath"), ListDirectory);

	if (!bFoundListDirectory || !FPaths::DirectoryExists(ListDirectory))
	{
		FString TempPath = FGenericPlatformMisc::GetEnvironmentVariable(TEXT("TEMP"));
		FPaths::NormalizeDirectoryName(TempPath);
		ListDirectory = TempPath;
		UE_LOG(LogVault, Error, TEXT("Unable to use PackageListStoragePath, storing file instead to : %s"), *TempPath);
	}

	FString TextDocFull = ListDirectory / PackageListFilename;
	FPaths::NormalizeDirectoryName(TextDocFull);
	UE_LOG(LogVault, Display, TEXT("Writing File List: %s"), *TextDocFull);
	
	FFileHelper::SaveStringArrayToFile(PackageObjects.Array(), *TextDocFull);
	
	const FString Root = FVaultSettings::Get().GetAssetLibraryRoot();
	
	const FString Filename = Meta.PackName.ToString() + TEXT(".upack");

	// Wrap Our Path in Quotes for use in Command-Line
	const FString Quote(TEXT("\""));
	const FString PackFilePath = Quote + (Root / Filename) + Quote;
	
	// Convert String to parsable command. Ensures path is wrapped in quotes in case of spaces in name
	const FString Command = FString::Printf(TEXT("%s -create=%s"), *PackFilePath, *TextDocFull);

	UE_LOG(LogVault, Display, TEXT("Running Pack Command: %s"), *Command);
	bool bRanPak = ExecuteUnrealPak(*Command);

	if (!bRanPak)
	{
		return false;
	}

	// Metadata Writing

	FMetadataOps::WriteMetadata(Meta);

	return true;

}

void UAssetPublisher::UpdateSystemMeta(FVaultMetadata& Metadata)
{
	Metadata.MachineID = FGenericPlatformMisc::GetLoginId();
}

