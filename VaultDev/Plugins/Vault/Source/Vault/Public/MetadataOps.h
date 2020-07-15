// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VaultTypes.h"
#include "Dom/JsonObject.h"
#include "VaultSettings.h"

/**
 * 
 */
class VAULT_API FMetadataOps
{
public:
	FMetadataOps();

	static FVaultMetadata ReadMetadata(FString File);

	static bool WriteMetadata(FVaultMetadata& Metadata);

	static TArray<FVaultMetadata> FindAllMetadataInLibrary();

	static TSet<FString> GetAllTags();

	static TSet<FTagFilteringItem> GetAllTagFilters();

private:

	static FVaultMetadata ParseMetaJsonToVaultMetadata(TSharedPtr<FJsonObject> MetaFile);

	static TSharedPtr<FJsonObject> ParseMetadataToJson(FVaultMetadata Metadata);

	~FMetadataOps();
};
