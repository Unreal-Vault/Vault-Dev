// Copyright Daniel Orchard 2020

#include "SVaultRootPanel.h"
#include "EditorStyleSet.h"
#include "Vault.h"

#include "SPublisherWindow.h"
#include "SLoaderWindow.h"

#define LOCTEXT_NAMESPACE "FVaultOperationChoce"

static const FName AssetPublisherTabId("Library Publisher");
static const FName AssetLoaderTabId("Library Browser");

void SVaultRootPanel::Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab, const TSharedPtr<SWindow>& ConstructUnderWindow)
{
	TabManager = FGlobalTabmanager::Get()->NewTabManager(ConstructUnderMajorTab);
	TabManager->RegisterTabSpawner(AssetLoaderTabId, FOnSpawnTab::CreateRaw(this, &SVaultRootPanel::HandleTabManagerSpawnTab, AssetLoaderTabId));
	TabManager->RegisterTabSpawner(AssetPublisherTabId, FOnSpawnTab::CreateRaw(this, &SVaultRootPanel::HandleTabManagerSpawnTab, AssetPublisherTabId));

	// Tab Layout
	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("VaultOperationsLayout")->AddArea(
		FTabManager::NewPrimaryArea()->SetOrientation(Orient_Horizontal)->Split(
			FTabManager::NewStack()
			->AddTab(AssetPublisherTabId, ETabState::OpenedTab)
			->AddTab(AssetLoaderTabId, ETabState::OpenedTab)
			->SetHideTabWell(false)
			->SetSizeCoefficient(0.75f)
			->SetForegroundTab(AssetLoaderTabId)
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
		// This options stops the editor closing while window open, so lets not use that.
		//.OnCanCloseTab_Lambda([]()
		//	{
		//		return false;
		//	}
		//);

	if (TabIdentifier == AssetPublisherTabId)
	{
		DockTab->SetContent(SNew(SPublisherWindow));
		//DockTab->SetLabel(LOCTEXT("PublisherTabLabel", "Asset Publisher"));
	}

	else if (TabIdentifier == AssetLoaderTabId)
	{
		TSharedRef<SLoaderWindow> LoaderWidget = SNew(SLoaderWindow, DockTab, Args.GetOwnerWindow());

		DockTab->SetContent(LoaderWidget);
		//DockTab->SetLabel(LOCTEXT("LoaderTabLabel", "Asset Browser"));
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