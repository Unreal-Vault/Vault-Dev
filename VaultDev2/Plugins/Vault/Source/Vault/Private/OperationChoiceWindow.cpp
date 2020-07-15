// Fill out your copyright notice in the Description page of Project Settings.

#include "OperationChoiceWindow.h"
#include "EditorStyleSet.h"
#include "Vault.h"

#include "SPublisherWindow.h"
#include "AssetLoader.h"
#include "SLoaderWindow.h"

#define LOCTEXT_NAMESPACE "FVaultOperationChoce"

static const FName AssetPublisherTabId("AssetPublisher");
static const FName AssetLoaderTabId("AssetLoader");

OperationChoiceWindow::OperationChoiceWindow()
{
}

OperationChoiceWindow::~OperationChoiceWindow()
{
}

SOperationsChoice::SOperationsChoice()
{

}

void SOperationsChoice::Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab, const TSharedPtr<SWindow>& ConstructUnderWindow)
{

	TabManager = FGlobalTabmanager::Get()->NewTabManager(ConstructUnderMajorTab);

	TabManager->RegisterTabSpawner(AssetPublisherTabId, FOnSpawnTab::CreateRaw(this, &SOperationsChoice::HandleTabManagerSpawnTab, AssetPublisherTabId));
	
	TabManager->RegisterTabSpawner(AssetLoaderTabId, FOnSpawnTab::CreateRaw(this, &SOperationsChoice::HandleTabManagerSpawnTab, AssetLoaderTabId));

	// Tab Layout
	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("VaultOperationsLayout")->AddArea(
		FTabManager::NewPrimaryArea()->SetOrientation(Orient_Horizontal)->Split(
			FTabManager::NewStack()
			->AddTab(AssetPublisherTabId, ETabState::OpenedTab)
			->AddTab(AssetLoaderTabId, ETabState::OpenedTab)
			->SetHideTabWell(false)
			->SetSizeCoefficient(0.75f)
			->SetForegroundTab(AssetPublisherTabId)
		)
	);

	FMenuBarBuilder MenuBarBuilder = FMenuBarBuilder(TSharedPtr<FUICommandList>());

	MenuBarBuilder.AddPullDownMenu(LOCTEXT("WindowMenuLabel", "Windows"), FText::GetEmpty(), FNewMenuDelegate::CreateStatic(&SOperationsChoice::FillWindowMenu, TabManager), "Window");



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
		//.AutoHeight()
		//[

		//]

		[
			TabManager->RestoreFrom(Layout, ConstructUnderWindow).ToSharedRef()

		]
		];

}



TSharedRef<SDockTab> SOperationsChoice::HandleTabManagerSpawnTab(const FSpawnTabArgs& Args, FName TabIdentifier) const
{
	TSharedPtr<SWidget> TabWidget = SNullWidget::NullWidget;

	TSharedRef<SDockTab> DockTab = SNew(SDockTab)
		.TabRole(ETabRole::PanelTab);

	if (TabIdentifier == AssetPublisherTabId)
	{
		DockTab->SetContent(SNew(SPublisherWindow));
	}

	else if (TabIdentifier == AssetLoaderTabId)
	{
		TSharedRef<SLoaderWindow> LoaderWidget = SNew(SLoaderWindow, DockTab, Args.GetOwnerWindow());

		DockTab->SetContent(LoaderWidget);
	}

	return DockTab;

}

void SOperationsChoice::FillWindowMenu(FMenuBuilder& MenuBuilder, const TSharedPtr<FTabManager> TabManager)
{
	if (!TabManager.IsValid())
	{
		return;
	}

#if !WITH_EDITOR
	FGlobalTabmanager::Get()->PopulateTabSpawnerMenu(MenuBuilder, WorkspaceMenu::GetMenuStructure().GetStructureRoot());
#endif //!WITH_EDITOR

	TabManager->PopulateLocalTabSpawnerMenu(MenuBuilder);
}



#undef LOCTEXT_NAMESPACE