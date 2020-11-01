// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VaultScriptingLibrary.generated.h"

/**
 * Master file for blueprint / python functions to access various parts of the Vault.
 */
UCLASS()
class VAULT_API UVaultScriptingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	// This function attempts to publish the asset and fill in as much metadata as possible. This is not a recommend function as it cant do a very good job from the barebones info available.
	//UFUNCTION()
	//static void PublishAssetAutomated(UObject* Asset);

	//static void PublishAsset(UObject* Asset, FString PackageName, FString PublisherName, TArray<FString> Tags);
	//
};
