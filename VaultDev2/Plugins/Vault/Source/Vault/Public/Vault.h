// Copyright Daniel Orchard 2020

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "SlateBasics.h"
#include "VaultTypes.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVault, Log, All);

class FToolBarBuilder;
class FMenuBuilder;
class UAssetPublisher;

class FVaultModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command. */
	void SpawnOperationsTab();

	// Get Module function
	static FVaultModule& Get();

	UAssetPublisher* GetAssetPublisherInstance() { return AssetPublisherInstance; }
	
private:

	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

	TSharedRef<SDockTab> CreateVaultMajorTab(const FSpawnTabArgs& TabSpawnArgs);

	TSharedPtr<class FUICommandList> PluginCommands;

	UAssetPublisher* AssetPublisherInstance;
};
