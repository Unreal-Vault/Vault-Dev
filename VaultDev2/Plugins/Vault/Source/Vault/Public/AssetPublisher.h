// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include <GameFramework/Actor.h>
#include "VaultTypes.h"
#include "AssetPublisher.generated.h"

UCLASS()
class VAULT_API UAssetPublisher : public UObject
{
public:

	GENERATED_BODY()

	// Package Step, Called from UI
	static bool PackageSelected(TSet<FString> PackageObjects, FVaultMetadata& Meta);

	/** Callback that fires after Publishing has completed on an Asset */
	DECLARE_DELEGATE(FOnVaultPackagingCompleted);
	//static FOnVaultPackagingCompleted& OnVaultPackagingCompleted() { return OnVaultPackagingCompletedDelegate; }
	static FOnVaultPackagingCompleted OnVaultPackagingCompletedDelegate;
private:

	static void UpdateSystemMeta(FVaultMetadata& Metadata);

	
	
};




