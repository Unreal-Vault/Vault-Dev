// Fill out your copyright notice in the Description page of Project Settings.

#include "SLoaderWindow.h"
#include "EditorSupportDelegates.h"
#include "VaultSettings.h"
#include "MetadataOps.h"
#include <ImageUtils.h>
#include "VaultTypes.h"
#include "AssetViewItem.h"
#include "SAssetPackTile.h"
#include <EditorStyleSet.h>

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
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTableView)
	{
		TagData = InArgs._TagData;

		SMultiColumnTableRow<FTagFilteringItemPtr>::Construct(FSuperRowType::FArguments().Padding(1.0f), OwnerTableView);
	}


	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		static const FMargin ColumnItemPadding(5, 0, 5, 0);

		if (ColumnName == VaultColumnNames::TagCheckedColumnName)
		{
			return SNew(SCheckBox);
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

};

class VAULT_API SDeveloperFilterRow : public SMultiColumnTableRow<FDeveloperFilteringItemPtr>
{
public:

	SLATE_BEGIN_ARGS(SDeveloperFilterRow) {}
	SLATE_ARGUMENT(FDeveloperFilteringItemPtr, Entry)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTableView)
	{
		Entry = InArgs._Entry;

		SMultiColumnTableRow<FDeveloperFilteringItemPtr>::Construct(FSuperRowType::FArguments().Padding(1.0f), OwnerTableView);
	}


	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		static const FMargin ColumnItemPadding(5, 0, 5, 0);

		if (ColumnName == VaultColumnNames::TagCheckedColumnName)
		{
			return SNew(SCheckBox);
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
	PopulateBaseAssetList();

	// Set the Default Scale for Sliders. 
	TileUserScale = 0.5;

	const float TILE_SCALED_WIDTH = TILE_BASE_WIDTH * TileUserScale;
	const float TILE_SCALED_HEIGHT = TILE_BASE_HEIGHT * TileUserScale;



	// Set up tag array
	TSet<FString> TagCloudTemp;
	FVaultSettings::Get().ReadVaultTags(TagCloudTemp);
	TagCloud.Empty();
	for (FString InTag : TagCloudTemp)
	{
		FTagFilteringItemPtr TagTemp = MakeShareable(new FTagFilteringItem);
		TagTemp->Tag = InTag;
		TagTemp->bFilterflag = false;
		TagTemp->UseCount = 99;
		
		TagCloud.Add(TagTemp);
	}

	// Developer Array
	DeveloperCloud.Empty();


	TMap<FName, int32> DevAssetCounter;
	
	for (auto AssetItem : FilteredAssetItems)
	{
		if (DevAssetCounter.Contains(AssetItem->Author))
		{
			int Count = *DevAssetCounter.Find(AssetItem->Author);
			DevAssetCounter.Add(AssetItem->Author, Count++);

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
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("VaultLoaderSidebarHeaderLabel", "VAULT LOADER"))
						]

					// Asset List Amount
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Top)
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
							.DelayChangeNotificationsWhileTyping(false)
							.Visibility(EVisibility::Visible)
							.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("AssetSearch")))
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("StrictSearchCheckBox", "Strict Search"))
							.Justification(ETextJustify::Right)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SAssignNew(StrictSearchCheckBox, SCheckBox)
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
								//.OnContextMenuOpening()
							]
							
						]
						
					// Bottom Bar
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("selectedURLLabel", "Selected Asset URL"))
						]

						+ SHorizontalBox::Slot()
							[
								// Scale Slider
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.Padding(FMargin(0, 3, 0, 0))
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
								.Text(LOCTEXT("VaultLoaderRightSidebarHeaderLabel", "Metadata"))
					
							]
						]
					]
		

		

					



				] // ~ hbox			
				]; // ~ LoaderRoot
	ChildSlot
		[
			LoaderRoot
		];


#if WITH_EDITOR
	FEditorSupportDelegates::PrepareToCleanseEditorObject.AddRaw(this, &SLoaderWindow::HandleMapChange);
#endif


}

void SLoaderWindow::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{

}

void SLoaderWindow::HandleMapChange(UObject* Object)
{

}

void SLoaderWindow::PopulateBaseAssetList()
{
	FilteredAssetItems.Empty();

	TArray<FVaultMetadata> MetaFiles;

	MetaFiles = FMetadataOps::FindAllMetadataInLibrary();

	for (FVaultMetadata Meta : MetaFiles)
	{
		FilteredAssetItems.Add(MakeShareable(new FVaultMetadata(Meta)));
	}
}


TSharedRef<ITableRow> SLoaderWindow::MakeTileViewWidget(TSharedPtr<FVaultMetadata> AssetItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	TSharedPtr<STableRow<TSharedPtr<FVaultMetadata>>> TableRowWidget;

	SAssignNew(TableRowWidget, STableRow<TSharedPtr<FVaultMetadata>>, OwnerTable)
		.Style(FEditorStyle::Get(), "ContentBrowser.AssetListView.TableRow")
		.Padding(FMargin(2.0f, 0.0f, 2.0f, 25.0f));


	TSharedRef<SAssetTileItem> Item = SNew(SAssetTileItem)
		.AssetItem(AssetItem);

	TableRowWidget->SetContent(Item);

	return TableRowWidget.ToSharedRef();
}


TSharedRef<ITableRow> SLoaderWindow::MakeTagFilterViewWidget(FTagFilteringItemPtr inTag, const TSharedRef<STableViewBase>& OwnerTable) const
{
	return SNew(STagFilterRow, OwnerTable)
		.TagData(inTag);

}

TSharedRef<ITableRow> SLoaderWindow::MakeDeveloperFilterViewWidget(FDeveloperFilteringItemPtr Entry, const TSharedRef<STableViewBase>& OwnerTable) const
{
	return SNew(SDeveloperFilterRow, OwnerTable)
		.Entry(Entry);
}

void SLoaderWindow::OnSearchBoxChanged(const FText& inSearchText)
{

	if (inSearchText.IsEmpty())
	{
		return;
	}



	FilteredAssetItems.Empty();
	TArray<FVaultMetadata> MetaFiles;

	MetaFiles = FMetadataOps::FindAllMetadataInLibrary();

	// Store Strict Search - This controls if we only search pack name, or various data entries.
	const bool bStrictSearch = StrictSearchCheckBox->GetCheckedState() == ECheckBoxState::Checked;

	const FString SearchString = inSearchText.ToString();

	for (FVaultMetadata Meta : MetaFiles)
	{
		if (Meta.PackName.ToString().Contains(SearchString))
		{
			FilteredAssetItems.Add(MakeShareable(new FVaultMetadata(Meta)));
			continue;
		}
		
		if (bStrictSearch == false)
		{
			if (Meta.Author.ToString().Contains(SearchString) || Meta.Description.Contains(SearchString))
			{
				FilteredAssetItems.Add(MakeShareable(new FVaultMetadata(Meta)));
			}
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
	// #todo check this for performance
	// Running this function on tick, is this a problem
	int assetCount = FMetadataOps::FindAllMetadataInLibrary().Num();

	FText Display = FText::Format(LOCTEXT("displayassetcountlabel", "Total Assets in library: {0}"),assetCount);
	return Display;
}




#undef LOCTEXT_NAMESPACE