// Copyright Daniel Orchard 2020

#include "SVaultRootPanel.h"
#include "EditorStyleSet.h"
#include "Vault.h"

#include "SPublisherWindow.h"
#include "SLoaderWindow.h"
#include "SSettingsWindow.h"
#include "SBatchPublisherWindow.h"

#define LOCTEXT_NAMESPACE "FVaultRootPanelNamespace"

static const FName AssetPublisherTabId("Asset Publisher");
static const FName AssetBrowserTabId("Asset Browser");
static const FName BatchPublisherTabId("Batch Publisher");
static const FName VaultSettingsTabId("Settings");

void SVaultRootPanel::Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab, const TSharedPtr<SWindow>& ConstructUnderWindow)
{
	TabManager = FGlobalTabmanager::Get()->NewTabManager(ConstructUnderMajorTab);
	TabManager->RegisterTabSpawner(AssetBrowserTabId, FOnSpawnTab::CreateRaw(this, &SVaultRootPanel::HandleTabManagerSpawnTab, AssetBrowserTabId));
	TabManager->RegisterTabSpawner(AssetPublisherTabId, FOnSpawnTab::CreateRaw(this, &SVaultRootPanel::HandleTabManagerSpawnTab, AssetPublisherTabId));
	TabManager->RegisterTabSpawner(BatchPublisherTabId, FOnSpawnTab::CreateRaw(this, &SVaultRootPanel::HandleTabManagerSpawnTab, BatchPublisherTabId));
	TabManager->RegisterTabSpawner(VaultSettingsTabId, FOnSpawnTab::CreateRaw(this, &SVaultRootPanel::HandleTabManagerSpawnTab, VaultSettingsTabId));

	// Tab Layout
	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("VaultOperationsLayout")->AddArea(
		FTabManager::NewPrimaryArea()->SetOrientation(Orient_Horizontal)->Split(
			FTabManager::NewStack()
			->AddTab(AssetBrowserTabId, ETabState::OpenedTab)
			->AddTab(AssetPublisherTabId, ETabState::OpenedTab)
			->AddTab(BatchPublisherTabId, ETabState::OpenedTab)
			->AddTab(VaultSettingsTabId, ETabState::OpenedTab)
			->SetHideTabWell(false)
			->SetSizeCoefficient(0.75f)
			->SetForegroundTab(AssetBrowserTabId)
		)
	);


	FMenuBarBuilder MenuBarBuilder = FMenuBarBuilder(TSharedPtr<FUICommandList>());
	MenuBarBuilder.AddPullDownMenu(LOCTEXT("WindowMenuLabel", "Windows"), FText::GetEmpty(), FNewMenuDelegate::CreateStatic(&SVaultRootPanel::FillWindowMenu, TabManager), "Window");

	ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				MenuBarBuilder.MakeWidget()
			]

			+ SVerticalBox::Slot()
		    .FillHeight(1.f)
			.Padding(0)
			[
				TabManager->RestoreFrom(Layout, ConstructUnderWindow).ToSharedRef()
			]
		];
}

TSharedRef<SDockTab> SVaultRootPanel::HandleTabManagerSpawnTab(const FSpawnTabArgs& Args, FName TabIdentifier) const
{
	TSharedPtr<SWidget> TabWidget = SNullWidget::NullWidget;

	TSharedRef<SDockTab> DockTab = SNew(SDockTab)
		.TabRole(ETabRole::PanelTab);

	if (TabIdentifier == AssetPublisherTabId)
	{
		DockTab->SetContent(SNew(SPublisherWindow));
	}

	else if (TabIdentifier == AssetBrowserTabId)
	{
		TSharedRef<SLoaderWindow> LoaderWidget = SNew(SLoaderWindow, DockTab, Args.GetOwnerWindow());

		DockTab->SetContent(LoaderWidget);
	}
	else if (TabIdentifier == VaultSettingsTabId)
	{
		DockTab->SetContent(SNew(SSettingsWindow));
	}
	else if (TabIdentifier == BatchPublisherTabId)
	{
		DockTab->SetContent(SNew(SBatchPublisherWindow));
	}


	


	return DockTab;
}

void SVaultRootPanel::FillWindowMenu(FMenuBuilder& MenuBuilder, const TSharedPtr<FTabManager> TabManager)
{
	if (!TabManager.IsValid())
	{
		return;
	}

	TabManager->PopulateLocalTabSpawnerMenu(MenuBuilder);
}

#undef LOCTEXT_NAMESPACE