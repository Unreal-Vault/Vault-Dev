// Fill out your copyright notice in the Description page of Project Settings.

#include "SLoaderWindow.h"

#include "Vault.h"
#include "EditorSupportDelegates.h" // this doesnt really work atm, need a better clean up or way of migrating maps.
#include "VaultSettings.h"
#include "MetadataOps.h"
#include <ImageUtils.h>
#include "VaultTypes.h"
#include "SAssetPackTile.h"
#include <EditorStyleSet.h>

#include "PakFileUtilities.h"
#include "EditorFramework/AssetImportData.h"
#include "AssetImportTask.h"
#include "AssetToolsModule.h"

#include "VaultStyle.h"


#define LOCTEXT_NAMESPACE "SVaultLoader"

const int32 SLoaderWindow::THUMBNAIL_BASE_HEIGHT = 415;
const int32 SLoaderWindow::THUMBNAIL_BASE_WIDTH = 415;
const int32 SLoaderWindow::TILE_BASE_HEIGHT = 465;
const int32 SLoaderWindow::TILE_BASE_WIDTH = 415;

namespace VaultColumnNames
{
	static const FName TagCheckedColumnName(TEXT("Flag"));
	static const FName TagNameColumnName(TEXT("Tag Name"));
	static const FName TagCounterColumnName(TEXT("Used"));
};

class VAULT_API STagFilterRow : public SMultiColumnTableRow<FTagFilteringItemPtr>
{
public:

	SLATE_BEGIN_ARGS(STagFilterRow) {}
	SLATE_ARGUMENT(FTagFilteringItemPtr, TagData)
	SLATE_ARGUMENT(TSharedPtr<SLoaderWindow>, ParentWindow)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTableView)
	{
		TagData = InArgs._TagData;
		ParentWindow = InArgs._ParentWindow;

		SMultiColumnTableRow<FTagFilteringItemPtr>::Construct(FSuperRowType::FArguments().Padding(1.0f), OwnerTableView);
	}

	void OnCheckBoxStateChanged(ECheckBoxState NewCheckedState)
	{
		const bool Filter = NewCheckedState == ECheckBoxState::Checked;
		ParentWindow->ModifyActiveTagFilters(TagData->Tag, Filter);
	}


	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		static const FMargin ColumnItemPadding(5, 0, 5, 0);

		if (ColumnName == VaultColumnNames::TagCheckedColumnName)
		{
			return SNew(SCheckBox)
				.IsChecked(false)
				.OnCheckStateChanged(this, &STagFilterRow::OnCheckBoxStateChanged);
		}
		else if (ColumnName == VaultColumnNames::TagNameColumnName)
		{
			return SNew(STextBlock)
				.Text(FText::FromString(TagData->Tag));
		}
		else if (ColumnName == VaultColumnNames::TagCounterColumnName)
		{
			return SNew(STextBlock)
				.Text(FText::FromString(FString::FromInt(TagData->UseCount)));
		}

		else
		{
			return SNullWidget::NullWidget;
		}
	}

private:

	FTagFilteringItemPtr TagData;
	TSharedPtr<SLoaderWindow> ParentWindow;

};

class VAULT_API SDeveloperFilterRow : public SMultiColumnTableRow<FDeveloperFilteringItemPtr>
{
public:

	SLATE_BEGIN_ARGS(SDeveloperFilterRow) {}

	SLATE_ARGUMENT(FDeveloperFilteringItemPtr, Entry)
	SLATE_ARGUMENT(TSharedPtr<SLoaderWindow>, ParentWindow)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTableView)
	{
		Entry = InArgs._Entry;
		ParentWindow = InArgs._ParentWindow;
		SMultiColumnTableRow<FDeveloperFilteringItemPtr>::Construct(FSuperRowType::FArguments().Padding(1.0f), OwnerTableView);
	}

	
	void OnCheckBoxStateChanged(ECheckBoxState NewCheckedState)
	{
		const bool Filter = NewCheckedState == ECheckBoxState::Checked;
		ParentWindow->ModifyActiveDevFilters(Entry->Developer, Filter);
	}


	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		static const FMargin ColumnItemPadding(5, 0, 5, 0);

		if (ColumnName == VaultColumnNames::TagCheckedColumnName)
		{
			return SNew(SCheckBox)
				.IsChecked(false)
				.OnCheckStateChanged(this, &SDeveloperFilterRow::OnCheckBoxStateChanged);
		}
		else if (ColumnName == VaultColumnNames::TagNameColumnName)
		{
			return SNew(STextBlock)
				.Text(FText::FromName(Entry->Developer));
		}
		else if (ColumnName == VaultColumnNames::TagCounterColumnName)
		{
			return SNew(STextBlock)
				.Text(FText::FromString(FString::FromInt(Entry->UseCount)));
		}
		else
		{
			return SNullWidget::NullWidget;
		}
	}

private:
	FDeveloperFilteringItemPtr Entry;
	TSharedPtr<SLoaderWindow> ParentWindow;
};

SLoaderWindow::SLoaderWindow() {}

SLoaderWindow::~SLoaderWindow()
{
#if WITH_EDITOR
	FEditorSupportDelegates::PrepareToCleanseEditorObject.RemoveAll(this);
#endif
}

void SLoaderWindow::Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab, const TSharedPtr<SWindow>& ConstructUnderWindow)
{
	RefreshAvailableFiles();
	PopulateBaseAssetList();
	PopulateTagArray();
	PopulateDeveloperNameArray();
	
	// Construct the Holder for the Metadata List
	MetadataWidget = SNew(SVerticalBox);

	// Set the Default Scale for Sliders. 
	TileUserScale = 0.5;

	const float TILE_SCALED_WIDTH = TILE_BASE_WIDTH * TileUserScale;
	const float TILE_SCALED_HEIGHT = TILE_BASE_HEIGHT * TileUserScale;


	// Main Widget
	TSharedRef<SVerticalBox> LoaderRoot = SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			// Primary 3 Boxes go in Here
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(SSplitter)
				.Orientation(Orient_Horizontal)
			
				+SSplitter::Slot()
				.Value(0.2f)
				[
					SNew(SBorder)
					.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
					.Padding(FMargin(4.0f, 4.0f))
					[
						// Left Sidebar!
						SNew(SVerticalBox)
						+SVerticalBox::Slot()
						.Padding(0,3,0,5)
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("VaultLoaderSidebarHeaderLabel", "FILTERING"))
							.TextStyle(FVaultStyle::Get(), "MetaTitleText")
						]

					// Asset List Amount
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Top)
					.Padding(0,0,0,5)
					[
						SNew(STextBlock)
						.Text(this, &SLoaderWindow::DisplayTotalAssetsInLibrary)
					]

					// Tag filtering
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SListView<FTagFilteringItemPtr>)
						.SelectionMode(ESelectionMode::Single)
						.ListItemsSource(&TagCloud)
						.OnGenerateRow(this, &SLoaderWindow::MakeTagFilterViewWidget)
						.HeaderRow
						(
							SNew(SHeaderRow)
							+ SHeaderRow::Column(VaultColumnNames::TagCheckedColumnName)
							.DefaultLabel(LOCTEXT("FilteringBoolLabel", "Filter"))
							.FixedWidth(40.0f)

							+ SHeaderRow::Column(VaultColumnNames::TagNameColumnName)
							.DefaultLabel(LOCTEXT("TagFilteringTagNameLabel", "Tags"))

							+ SHeaderRow::Column(VaultColumnNames::TagCounterColumnName)
							.DefaultLabel(LOCTEXT("TagFilteringCounterLabel", "Used"))
						)

					]

					// Developer Filtering
					+ SVerticalBox::Slot()
					.AutoHeight()
						[
							SNew(SListView<FDeveloperFilteringItemPtr>)
							.SelectionMode(ESelectionMode::Single)
							.ListItemsSource(&DeveloperCloud)
							.OnGenerateRow(this, &SLoaderWindow::MakeDeveloperFilterViewWidget)
							.HeaderRow
							(
								SNew(SHeaderRow)
								+ SHeaderRow::Column(VaultColumnNames::TagCheckedColumnName)
								.DefaultLabel(LOCTEXT("FilteringBoolLabel", "Filter"))
								.FixedWidth(40.0f)

								+ SHeaderRow::Column(VaultColumnNames::TagNameColumnName)
								.DefaultLabel(LOCTEXT("TagFilteringTagNameLabel", "Tags"))

								+ SHeaderRow::Column(VaultColumnNames::TagCounterColumnName)
								.DefaultLabel(LOCTEXT("TagFilteringCounterLabel", "Used"))
							)
						]

						// Misc Filtering
						+SVerticalBox::Slot()

						// Spacer
						+ SVerticalBox::Slot()
						[
							SNew(SSpacer)
						]

					// Selected Asset metadata
					+ SVerticalBox::Slot()
					] // close SBorder
					] // ~Close Left Splitter Area
							

				// Center Area!
				+SSplitter::Slot()
				.Value(0.6f)
				[
					SNew(SBorder)
					.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
					.Padding(FMargin(4.0f, 4.0f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(1)
							.Padding(FMargin(0,3,0,0))
							[
								// Center content area
								SAssignNew(SearchBox, SSearchBox)
								.HintText(LOCTEXT("SearchBoxHintText", "Search..."))
								.OnTextChanged(this, &SLoaderWindow::OnSearchBoxChanged)
								.OnTextCommitted(this, &SLoaderWindow::OnSearchBoxCommitted)
								.DelayChangeNotificationsWhileTyping(false)
								.Visibility(EVisibility::Visible)
								.Style(FVaultStyle::Get(), "AssetSearchBar")
								.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("AssetSearch")))
							]

							+ SHorizontalBox::Slot()
							.Padding(FMargin(15.f,0.f, 5.f, 0.f))
							.AutoWidth()
							[
								SAssignNew(StrictSearchCheckBox, SCheckBox)
								.Style(FCoreStyle::Get(), "ToggleButtonCheckbox")
								.Padding(FMargin( 5.f,0.f ))
								.ToolTipText(LOCTEXT("StrictSearchToolTip", "Search only the Pack Names"))
								[
									SNew(SBox)
									.VAlign(VAlign_Center)
									.HAlign(HAlign_Center)
									.Padding(FMargin(4.f,2.f))
									[
										SNew(STextBlock)
										.Text(LOCTEXT("StrictSearchCheckBox", "Strict Search"))
									]
								]
							]
						]

						// Tile View
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
							[
								SNew(SBox)
								.Padding(FMargin(5,5,5,5))
								[
									SAssignNew(TileView, STileView<TSharedPtr<FVaultMetadata>>)
									.ItemWidth(TILE_SCALED_WIDTH)
									.ItemHeight(TILE_SCALED_HEIGHT)
									.ItemAlignment(EListItemAlignment::EvenlyDistributed)
									.ListItemsSource(&FilteredAssetItems)
									.OnGenerateTile(this, &SLoaderWindow::MakeTileViewWidget)
									.SelectionMode(ESelectionMode::Single)
									.OnSelectionChanged(this, &SLoaderWindow::OnAssetTileSelectionChanged)
									.OnMouseButtonDoubleClick(this, &SLoaderWindow::OnAssetTileDoubleClicked)
									.OnContextMenuOpening(this, &SLoaderWindow::OnAssetTileContextMenuOpened)
								]
							
							]
						
						// Bottom Bar
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SButton)
								.Text(LOCTEXT("RefreshLibraryScreenBtnLbl", "Refresh Library"))
								.OnClicked(this, &SLoaderWindow::OnRefreshLibraryClicked)
							]
							+ SHorizontalBox::Slot()
								[
									SNew(SSpacer)
								]
							+ SHorizontalBox::Slot()
								[
									// Scale Slider
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.AutoWidth()
									.HAlign(HAlign_Right)
									.Padding(FMargin(0, 0, 0, 0))
									[
										SNew(STextBlock)
										.Text(LOCTEXT("ScaleSliderLabel", "Thumbnail Scale"))
										.Justification(ETextJustify::Right)
									]
									+ SHorizontalBox::Slot()
										[
											SAssignNew(UserScaleSlider, SSlider)
											.Value(TileUserScale)
											.MinValue(0.2)
											.OnValueChanged(this, &SLoaderWindow::OnThumbnailSliderValueChanged)
										]
								]
						]
					] // close border for center area
				] // ~ Close Center Area Splitter
				

				// Metadata Zone
				+ SSplitter::Slot()
					.Value(0.2f)
					[
						SNew(SBorder)
						.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
						.Padding(FMargin(4.0f, 4.0f))
						[
							// Left Sidebar!
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(STextBlock)
								.Text(LOCTEXT("VaultLoaderRightSidebarHeaderLabel", "METADATA"))
								.TextStyle(FVaultStyle::Get(), "MetaTitleText")
					
							]

							+ SVerticalBox::Slot()
							.FillHeight(1)
							[
								SNew(SBox)
								[
									MetadataWidget.ToSharedRef()
								]
							]
						]
					]
				] // ~ hbox			
				]; // ~ LoaderRoot
	ChildSlot
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
			.Padding(FMargin(2.f,2.f))
			[
				LoaderRoot
			]
		];

}

void SLoaderWindow::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{

}

void SLoaderWindow::PopulateBaseAssetList()
{
	FilteredAssetItems.Empty();

	for (FVaultMetadata Meta : MetaFilesCache)
	{
		FilteredAssetItems.Add(MakeShareable(new FVaultMetadata(Meta)));
	}
}

// Only shows tags that are actually used, no empty tags will appear. 
void SLoaderWindow::PopulateTagArray()
{
	// Empty Tag Container
	TagCloud.Empty();

	// Create a map version of the array for more efficient searching.
	TMap<FString, FTagFilteringItemPtr> TagCloudMap;

	// For each Asset in our global list of assets...
	for (auto Asset : MetaFilesCache)
	{
		// Get each tag belonging to that asset...
		for (const FString AssetTag : Asset.Tags)
		{
			// If we already have a tag stored for it, increment the use counter
			if (TagCloudMap.Contains(AssetTag))
			{
				TagCloudMap.Find(AssetTag)->Get()->UseCount++;
			}

			// otherwise, add a new tag to our list. 
			else
			{

				FTagFilteringItemPtr TagTemp = MakeShareable(new FTagFilteringItem);
				TagTemp->Tag = AssetTag;
				TagTemp->UseCount = 1;
				TagCloudMap.Add(AssetTag, TagTemp);
			}
		}
	}

	// The Map version is easier to work with during generation, but since we have to use an Array, we convert our cloud map into an array now:
	TagCloudMap.GenerateValueArray(TagCloud);

}

void SLoaderWindow::PopulateDeveloperNameArray()
{
	// Developer Array
	DeveloperCloud.Empty();

	TMap<FName, int32> DevAssetCounter;

	for (auto AssetItem : FilteredAssetItems)
	{
		if (DevAssetCounter.Contains(AssetItem->Author))
		{
			int Count = *DevAssetCounter.Find(AssetItem->Author);
			Count++;
			DevAssetCounter.Add(AssetItem->Author, Count);
		}
		else
		{
			DevAssetCounter.Add(AssetItem->Author, 1);
		}
	}

	for (auto dev : DevAssetCounter)
	{
		FDeveloperFilteringItemPtr DevTemp = MakeShareable(new FDeveloperFilteringItem);
		DevTemp->Developer = dev.Key;
		DevTemp->UseCount = dev.Value;
		DeveloperCloud.AddUnique(DevTemp);
	}
}

TSharedRef<ITableRow> SLoaderWindow::MakeTileViewWidget(TSharedPtr<FVaultMetadata> AssetItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FVaultMetadata>>, OwnerTable)
		.Style(FEditorStyle::Get(), "ContentBrowser.AssetListView.TableRow")
		.Padding(FMargin(5.0f, 5.0f, 5.0f, 25.0f))
		[
			SNew(SAssetTileItem)
			.AssetItem(AssetItem)
		];
}

TSharedRef<ITableRow> SLoaderWindow::MakeTagFilterViewWidget(FTagFilteringItemPtr inTag, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STagFilterRow, OwnerTable)
		.TagData(inTag)
		.ParentWindow(SharedThis(this));

}

TSharedRef<ITableRow> SLoaderWindow::MakeDeveloperFilterViewWidget(FDeveloperFilteringItemPtr Entry, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SDeveloperFilterRow, OwnerTable)
		.Entry(Entry)
		.ParentWindow(SharedThis(this));
}

void SLoaderWindow::OnAssetTileSelectionChanged(TSharedPtr<FVaultMetadata> InItem, ESelectInfo::Type SelectInfo)
{
	// Checks if anything is selected
	if (TileView->GetNumItemsSelected() > 0)
	{
		ConstructMetadataWidget(InItem);
		return;
	}

	// If no selection, clear the active metadata.
	MetadataWidget->ClearChildren();

}

void SLoaderWindow::OnAssetTileDoubleClicked(TSharedPtr<FVaultMetadata> InItem)
{
	// #todo Add Item to Project on Double Click
	LoadAssetPackIntoProject(InItem);
}

TSharedPtr<SWidget> SLoaderWindow::OnAssetTileContextMenuOpened()
{

	// Lets check we have stuff selected, we only want the context menu on selected items. No need to open on a blank area
	
	if (TileView->GetNumItemsSelected() == 0)
	{
		return SNullWidget::NullWidget;
	}
	
	// Store our selected item for any future operations.
	TSharedPtr<FVaultMetadata> SelectedAsset = TileView->GetSelectedItems()[0];

	FMenuBuilder MenuBuilder(true, nullptr, nullptr, true);

	static const FName FilterSelectionHook("AssetContextMenu");

	MenuBuilder.BeginSection(FilterSelectionHook, LOCTEXT("AssetContextMenuLabel", "Vault Asset"));
	{
		MenuBuilder.AddMenuEntry(LOCTEXT("ACM_AddToProjectLabel", "Add To Project"), FText::GetEmpty(), FSlateIcon(), 
			FUIAction(FExecuteAction::CreateLambda([this, SelectedAsset]()
				{
					LoadAssetPackIntoProject(SelectedAsset);
				
				}), 
				FCanExecuteAction(),
				FGetActionCheckState(),
				FIsActionButtonVisible()));


		MenuBuilder.AddMenuEntry(LOCTEXT("ACM_EditVaultAssetDetailsLabel", "Edit Asset"), FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, SelectedAsset]()
				{
					const FString LibraryPath = FVaultSettings::Get().GetAssetLibraryRoot();
					const FString MetaFilePath = LibraryPath / SelectedAsset->PackName.ToString() + ".meta";

					// Rather than provide a tonne of edit options in engine and a load of extra UI support, for the time being lets just open the file in a text editor
					FPlatformProcess::LaunchFileInDefaultExternalApplication(*MetaFilePath);

				}),
				FCanExecuteAction(),
					FGetActionCheckState(),
					FIsActionButtonVisible()));


		MenuBuilder.AddMenuEntry(LOCTEXT("ACM_DeleteVaultAssetLabel", "Delete Asset"), FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, SelectedAsset]()
				{
					// Open a Msg dialog to confirm deletion
					const EAppReturnType::Type Confirmation = FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("DeleteViaContextMsg", "Are you sure you want to delete this asset from the Vault Library \nThis option cannot be undone." ));
					if (Confirmation == EAppReturnType::Yes)
					{
						// Delete Pack. Handles all the UI stuff from here as well as the file deletes.
						DeleteAssetPack(SelectedAsset);
					}
				}),
				FCanExecuteAction(),
					FGetActionCheckState(),
					FIsActionButtonVisible()));

	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();

}

void SLoaderWindow::OnSearchBoxChanged(const FText& inSearchText)
{
	//FilteredAssetItems.Empty();

	// If its now empty, it was probably cleared or backspaced through, so we need to reapply just the filter based results.
	if (inSearchText.IsEmpty())
	{
		UpdateFilteredAssets();
		return;
	}

	// Store Strict Search - This controls if we only search pack name, or various data entries.
	const bool bStrictSearch = StrictSearchCheckBox->GetCheckedState() == ECheckBoxState::Checked;

	const FString SearchString = inSearchText.ToString();
	
	// Holder for the newly filtered Results:
	TArray<TSharedPtr<FVaultMetadata>> SearchMatchingEntries;

	// Instead of searching raw meta, we search the filtered results, so this respects the tag and dev filters first, and we search within that.
	for (auto Meta : FilteredAssetItems)
	{
		if (Meta->PackName.ToString().Contains(SearchString))
		{
			SearchMatchingEntries.Add(Meta);
			continue;
		}
		
		if (bStrictSearch == false)
		{
			if (Meta->Author.ToString().Contains(SearchString) || Meta->Description.Contains(SearchString))
			{
				SearchMatchingEntries.Add(Meta);
			}
		}
	}

	FilteredAssetItems = SearchMatchingEntries;
	TileView->RebuildList();
	TileView->ScrollToTop();

}

void SLoaderWindow::OnSearchBoxCommitted(const FText& InFilterText, ETextCommit::Type CommitType)
{
	OnSearchBoxChanged(InFilterText);
}

void SLoaderWindow::ConstructMetadataWidget(TSharedPtr<FVaultMetadata> AssetMeta)
{

	// Safety Catches for Null Assets. Should never occur.
	if (!MetadataWidget.IsValid())
	{
		UE_LOG(LogVault, Error, TEXT("Error - Metadata Ptr is Null."));
		return;
	}

	if (!AssetMeta.IsValid() || !AssetMeta->IsMetaValid())
	{
		UE_LOG(LogVault, Error, TEXT("Error - Metadata Incoming Data is Null."));
		return;
	}

	// Padding between words
	const FMargin WordPadding = FMargin(0.f,12.f,0.f,0.f);

	MetadataWidget->ClearChildren();
	
	// Pack Name
	MetadataWidget->AddSlot()
		.AutoHeight()
		.Padding(WordPadding)
	[
		SNew(STextBlock)
			.Text(FText::FromName(AssetMeta->PackName))
	];

	// Description
	MetadataWidget->AddSlot()
		.AutoHeight()

		.Padding(WordPadding)
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			//.Text(FText::Format(LOCTEXT("Meta_DescLbl", "Description: \n{0}"), FText::FromString(AssetMeta->Description)))
			.Text(FText::FromString(AssetMeta->Description))
		];

	// Author
	MetadataWidget->AddSlot()
		.AutoHeight()
		.Padding(WordPadding)
		[
			SNew(STextBlock)
			.Text(FText::Format(LOCTEXT("Meta_AuthorLbl", "Author: {0}"), FText::FromName(AssetMeta->Author)))
		];



	// Creation Date
	MetadataWidget->AddSlot()
		.AutoHeight()
		.Padding(WordPadding)
		[
			SNew(STextBlock)
			.Text(FText::Format(LOCTEXT("Meta_CreationLbl", "Created: {0}"), FText::FromString(AssetMeta->CreationDate.ToString())))
		];

	// Last Modified Date
	MetadataWidget->AddSlot()
		.AutoHeight()
		.Padding(WordPadding)
		[
			SNew(STextBlock)
			.Text(FText::Format(LOCTEXT("Meta_LastModifiedLbl", "Last Modified: {0}"), FText::FromString(AssetMeta->LastModified.ToString())))
		];

	// Tags List - Header
	MetadataWidget->AddSlot()
		.AutoHeight()
		.Padding(WordPadding)
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			//.Text(FText::Format(LOCTEXT("Meta_TagsLbl", "Tags: {0}"), FText::FromString(FString::Join(AssetMeta->Tags.Array(), TEXT(",")))))
			.Text(LOCTEXT("Meta_TagsLbl", "Tags:"))
		];

	// Tags, Per Tag. Instead of using Join, we add them as separate lines for ease of reading
	for (auto MyTag : AssetMeta->Tags)
	{
		MyTag.TrimStartAndEndInline();
		const FText TagName = FText::FromString(MyTag);

		//MyTag.RemoveSpacesInline();
		MetadataWidget->AddSlot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::Format(LOCTEXT("PerTagKeyFor{0}", "- {1}"), TagName, TagName))
			];
	}

	

	// Object List 
	MetadataWidget->AddSlot()
		.AutoHeight()
		.Padding(WordPadding)
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(FText::Format(LOCTEXT("Meta_FilesLbl", "Files: {0}"), FText::FromString(FString::Join(AssetMeta->ObjectsInPack.Array(), TEXT(",")))))
		];

}

void SLoaderWindow::LoadAssetPackIntoProject(TSharedPtr<FVaultMetadata> InPack)
{
	// Root Directory
	const FString LibraryPath = FVaultSettings::Get().GetAssetLibraryRoot();

	// All files live in same directory, so we just do some string mods to get the pack file that matches the meta file.
	const FString AssetToImportTemp = LibraryPath / InPack->PackName.ToString() + ".upack";

	// UPacks import natively with Unreal, so no need to try to use the PakUtilities, better to use the native importer and let Unreal handle the Pak concepts. 
	FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");

	// Set up our Import Task
	UAssetImportTask* Task = NewObject<UAssetImportTask>();
	Task->AddToRoot();
	Task->bAutomated = true;
	Task->bReplaceExisting = true;
	Task->bSave = true;
	Task->Filename = AssetToImportTemp;
	Task->DestinationPath = "/Game";

	TArray<UAssetImportTask*> Tasks;
	Tasks.Add(Task);

	AssetToolsModule.Get().ImportAssetTasks(Tasks);

	// Allow GC to collect
	Task->RemoveFromRoot();

}

void SLoaderWindow::DeleteAssetPack(TSharedPtr<FVaultMetadata> InPack)
{
	// Confirmation will have occurred already for this operation (Might be changed in future to have confirmation here)
	UE_LOG(LogVault, Display, TEXT("Deleting File(s) from Vault: %s"), *InPack->PackName.ToString());

	const FString LibraryPath = FVaultSettings::Get().GetAssetLibraryRoot();
	const FString FilePathAbsNoExt = LibraryPath / InPack->PackName.ToString();
	const FString AbsThumbnailPath = FilePathAbsNoExt + ".png";
	const FString AbsMetaPath = FilePathAbsNoExt + ".meta";
	const FString AbsPackPath = FilePathAbsNoExt + ".upack";

	IFileManager::Get().Delete(*AbsThumbnailPath, true);
	IFileManager::Get().Delete(*AbsMetaPath, true);
	IFileManager::Get().Delete(*AbsPackPath, true);


	MetaFilesCache.Remove(*InPack);
	RefreshLibrary();
	//UpdateFilteredAssets();
	//TileView->RebuildList();

}

void SLoaderWindow::RefreshAvailableFiles()
{
	MetaFilesCache = FMetadataOps::FindAllMetadataInLibrary();
}

// Applies the List of filters all together.
void SLoaderWindow::UpdateFilteredAssets()
{
	FilteredAssetItems.Empty();

	// Special Condition to check if all boxes are cleared:
	if (ActiveTagFilters.Num() == 0 && ActiveDevFilters.Num() == 0)
	{
		PopulateBaseAssetList();
		TileView->RebuildList();
		TileView->ScrollToTop();
		return;
	}

	for (auto Asset : MetaFilesCache)
	{
		// Apply all filtered Tags
		for (auto UserTag : Asset.Tags)
		{
			if (ActiveTagFilters.Contains(UserTag))
			{
				FilteredAssetItems.Add(MakeShareable(new FVaultMetadata(Asset)));
				break;
			}
		}

		// Apply All Developer Tags
		if (ActiveDevFilters.Contains(Asset.Author))
		{
			FilteredAssetItems.Add(MakeShareable(new FVaultMetadata(Asset)));
			continue;
		}
	}

	TileView->RebuildList();
	TileView->ScrollToTop();

}

void SLoaderWindow::OnThumbnailSliderValueChanged(float Value)
{
	TileUserScale = Value;
	TileView->SetItemWidth(TILE_BASE_WIDTH * TileUserScale);
	TileView->SetItemHeight(TILE_BASE_HEIGHT * TileUserScale);
	TileView->RebuildList();
}

FText SLoaderWindow::DisplayTotalAssetsInLibrary() const
{
	int assetCount = FMetadataOps::FindAllMetadataInLibrary().Num();

	FText Display = FText::Format(LOCTEXT("displayassetcountlabel", "Total Assets in library: {0}"),assetCount);
	return Display;
}

FReply SLoaderWindow::OnRefreshLibraryClicked()
{
	RefreshLibrary();
	return FReply::Handled();
}

void SLoaderWindow::RefreshLibrary()
{
	RefreshAvailableFiles();
	PopulateTagArray();
	PopulateDeveloperNameArray();

	//SearchBox->AdvanceSearch//
	//TileView->RebuildList();
	UpdateFilteredAssets();
}

void SLoaderWindow::ModifyActiveTagFilters(FString TagModified, bool bFilterThis)
{
	UE_LOG(LogVault, Display, TEXT("Enabling Tag Filter For %s"), *TagModified);
	
	if (bFilterThis)
	{
		// Push our Active Tag into our Set of Tags currently being searched
		ActiveTagFilters.Add(TagModified);
		UpdateFilteredAssets();
		return;
	}

	ActiveTagFilters.Remove(TagModified);
	UpdateFilteredAssets();
}

void SLoaderWindow::ModifyActiveDevFilters(FName DevModified, bool bFilterThis)
{
	UE_LOG(LogVault, Display, TEXT("Enabling Dev Filter %s"), *DevModified.ToString());

	if (bFilterThis)
	{
		ActiveDevFilters.Add(DevModified);
		UpdateFilteredAssets();
		return;
	}

	ActiveDevFilters.Remove(DevModified);
	UpdateFilteredAssets();
}

#undef LOCTEXT_NAMESPACE