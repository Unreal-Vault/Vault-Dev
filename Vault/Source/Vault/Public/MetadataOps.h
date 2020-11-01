// Copyright Daniel Orchard 2020

#pragma once

#include "CoreMinimal.h"
#include "VaultTypes.h"
#include "Dom/JsonObject.h"
#include "VaultSettings.h"

class VAULT_API FMetadataOps
{
public:
	static FVaultMetadata ReadMetadata(FString File);

	static bool WriteMetadata(FVaultMetadata& Metadata);

	static TArray<FVaultMetadata> FindAllMetadataInLibrary();

	static TSet<FString> GetAllTags();

	static TSet<FTagFilteringItem> GetAllTagFilters();

private:

	static FVaultMetadata ParseMetaJsonToVaultMetadata(TSharedPtr<FJsonObject> MetaFile);

	static TSharedPtr<FJsonObject> ParseMetadataToJson(FVaultMetadata Metadata);
};
