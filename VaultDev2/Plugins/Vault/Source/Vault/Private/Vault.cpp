// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "Vault.h"

#include "VaultTypes.h"
#include "VaultStyle.h"
#include "Misc/MessageDialog.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "SVaultRootPanel.h"
#include "MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "VaultSettings.h"
#include "SPublisherWindow.h"
#include "AssetPublisher.h"
#include "LevelEditor.h"

static const FName VaultTabName("VaultOperations");
static const FName VaultPublisherName("VaultPublisher");
static const FName VaultLoaderName("VaultLoader");

#define LOCTEXT_NAMESPACE "FVaultModule"
DEFINE_LOG_CATEGORY(LogVault);

class FVaultCommands : public TCommands<FVaultCommands>
{
public:
	FVaultCommands()
		: TCommands<FVaultCommands>(
			TEXT("Vault"),
			LOCTEXT("Vault", "Vault Plugin"),
			NAME_None,
			FVaultStyle::GetStyleSetName())
	{}

	virtual void RegisterCommands() override;
	TSharedPtr< FUICommandInfo > PluginAction;
};


// Create and Register our Commands. We only use the one so no need for a separate cpp/h file.
void FVaultCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "Vault", "Open the Vault", EUserInterfaceActionType::Button, FInputGesture());
}


void FVaultModule::StartupModule()
{
	// Init our styles
	FVaultStyle::Initialize();

	// Reload Textures
	FVaultStyle::ReloadTextures();

	// Register our Vault Commands into the Engine
	FVaultCommands::Register();

	// Init the Settings system
	FVaultSettings::Get().Initialize();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FVaultCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FVaultModule::SpawnOperationsTab),
		FCanExecuteAction());
	
	// Store a Ref to the Level Editor.
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
	const FText VaultBasePanelWindowTitle = LOCTEXT("OperationsWindowTitleLabel", "The Vault");
	const FText VaultBasePanelWindowTooltip = LOCTEXT("VaultWindowTooltipLabel", "Vault Operations");

	// Init tabs
	TSharedRef<FGlobalTabmanager> TabManager = FGlobalTabmanager::Get();

	TabManager->RegisterNomadTabSpawner(VaultTabName, FOnSpawnTab::CreateRaw(this, &FVaultModule::SpawnOperationTab))
		.SetDisplayName(VaultBasePanelWindowTitle)
		.SetTooltipText(VaultBasePanelWindowTooltip);

}

void FVaultModule::ShutdownModule()
{
	FVaultStyle::Shutdown();
	FVaultCommands::Unregister();
	TSharedRef<FGlobalTabmanager> TabManager = FGlobalTabmanager::Get();
	TabManager->UnregisterTabSpawner(VaultTabName);
}

void FVaultModule::SpawnOperationsTab()
{
	TSharedRef<FGlobalTabmanager> TabManager = FGlobalTabmanager::Get();
	TabManager->InvokeTab(VaultTabName);
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

	TSharedRef<SVaultRootPanel> VaultBasePanelWidget = SNew(SVaultRootPanel, SpawnedTab, TabSpawnArgs.GetOwnerWindow());

	SpawnedTab->SetContent(VaultBasePanelWidget);

	return SpawnedTab;
}


void FVaultModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FVaultCommands::Get().PluginAction);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVaultModule, Vault)