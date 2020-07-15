// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "Vault.h"
#include "VaultTypes.h"
#include "VaultStyle.h"
#include "VaultCommands.h"
#include "Misc/MessageDialog.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "OperationChoiceWindow.h"
#include "MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "VaultSettings.h"
#include "SPublisherWindow.h"
#include "AssetLoader.h"
#include "AssetPublisher.h"

#include "LevelEditor.h"

static const FName VaultTabName("VaultOperations");
static const FName VaultPublisherName("VaultPublisher");
static const FName VaultLoaderName("VaultLoader");

#define LOCTEXT_NAMESPACE "FVaultModule"
DEFINE_LOG_CATEGORY(LogVault);

void FVaultModule::StartupModule()
{
	// This code will execute after your module is loaded into memory
	
	FVaultStyle::Initialize();
	FVaultStyle::ReloadTextures();

	FVaultCommands::Register();

	FVaultSettings::Get().Initialize();


	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FVaultCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FVaultModule::SpawnOperationsTab),
		FCanExecuteAction());
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FVaultModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FVaultModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}

	// Setup Operations Tab
	const FText OperationsWindowTitle = LOCTEXT("OperationsWindowTitleLabel", "Vault");
	const FText VaultWindowTooltip = LOCTEXT("VaultWindowTooltipLabel", "Vault Operations");


	// Init tabs
	TSharedRef<FGlobalTabmanager> TabManager = FGlobalTabmanager::Get();

	TabManager->RegisterNomadTabSpawner(VaultTabName, FOnSpawnTab::CreateRaw(this, &FVaultModule::SpawnOperationTab))
		.SetDisplayName(OperationsWindowTitle)
		.SetTooltipText(VaultWindowTooltip);



}

void FVaultModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FVaultStyle::Shutdown();

	FVaultCommands::Unregister();

	TSharedRef<FGlobalTabmanager> TabManager = FGlobalTabmanager::Get();
	TabManager->UnregisterTabSpawner(VaultTabName);
}

void FVaultModule::SpawnOperationsTab()
{


	TSharedRef<FGlobalTabmanager> TabManager = FGlobalTabmanager::Get();
	TabManager->InvokeTab(VaultTabName);

	// Since we need a instance of the UAssetPublisher to be able to set instance changes on:



}

FVaultModule& FVaultModule::Get()
{
	static const FName VaultModuleName = "Vault";
	return FModuleManager::LoadModuleChecked<FVaultModule>(VaultModuleName);
}


void FVaultModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FVaultCommands::Get().PluginAction);
}



TSharedRef<SDockTab> FVaultModule::SpawnOperationTab(const FSpawnTabArgs& TabSpawnArgs)
{
	const TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.TabRole(ETabRole::MajorTab);

	TSharedRef<SOperationsChoice> OperationsWidget = SNew(SOperationsChoice, SpawnedTab, TabSpawnArgs.GetOwnerWindow());

	SpawnedTab->SetContent(OperationsWidget);

	return SpawnedTab;

}


void FVaultModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FVaultCommands::Get().PluginAction);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVaultModule, Vault)