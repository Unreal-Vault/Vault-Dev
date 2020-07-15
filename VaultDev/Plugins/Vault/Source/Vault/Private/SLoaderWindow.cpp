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

SLoaderWindow::SLoaderWindow()
{

}

SLoaderWindow::~SLoaderWindow()
{
#if WITH_EDITOR
	FEditorSupportDelegates::PrepareToCleanseEditorObject.RemoveAll(this);
#endif
}

void SLoaderWindow::Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab, const TSharedPtr<SWindow>& ConstructUnderWindow)
{
	PopulateBaseAssetList();

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
	
	for (auto AssetItem : FilteredAssetItems)
	{
		FDeveloperFilteringItemPtr DevTemp = MakeShareable(new FDeveloperFilteringItem);
		DevTemp->Developer = AssetItem->Author;
		DevTemp->bFilterflag = false;
		DevTemp->UseCount = 99;
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
						// Center content area
						SAssignNew(SearchBox, SSearchBox)
						.HintText(LOCTEXT("SearchBoxHintText", "Search..."))
						.OnTextChanged(this, &SLoaderWindow::OnSearchBoxChanged)
						.DelayChangeNotificationsWhileTyping(true)
						.Visibility(EVisibility::Visible)
						.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("AssetSearch")))
					]

					// Tile View
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
						[
							SAssignNew(TileView, STileView<TSharedPtr<FVaultMetadata>>)
							.ListItemsSource(&FilteredAssetItems)
							.OnGenerateTile(this, &SLoaderWindow::MakeTileViewWidget)
							//.OnContextMenuOpening()
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
						.FillWidth(1)
						[
							// Checkbox wrapper
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							[
								SNew(STextBlock)
								.Text(LOCTEXT("TileViewCheckLabel", "Tile View"))
							]

							+ SHorizontalBox::Slot()
							[
								SNew(SCheckBox)
							]
							+ SHorizontalBox::Slot()
							[
								SNew(STextBlock)
								.Text(LOCTEXT("ListViewCheckLabel", "List View"))
							]

							+ SHorizontalBox::Slot()
							[
								SNew(SCheckBox)
							]
						]

						+ SHorizontalBox::Slot()
							[
								// Scale Slider
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								[
									SNew(STextBlock)
									.Text(LOCTEXT("ScaleSliderLabel", "Thumbnail Scale"))
								]
								+ SHorizontalBox::Slot()
									[
										SNew(SSlider)
									]
							]
					]
					
				] // ~ Close Center Area Splitter

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
	// see asset thumbnail.cpp, line 1000 for thumbnail support

	TSharedPtr<STableRow<TSharedPtr<FVaultMetadata>>> TableRowWidget;
	SAssignNew(TableRowWidget, STableRow<TSharedPtr<FVaultMetadata>>, OwnerTable)
		.Style(FEditorStyle::Get(), "ContentBrowser.AssetListView.TableRow");


	TSharedRef<SAssetTileItem> Item = SNew(SAssetTileItem)
		.AssetItem(AssetItem)
		.ItemWidth(this, &SLoaderWindow::GetTileViewItemWidth);

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

}

float SLoaderWindow::GetTileViewItemBaseWidth() const
{
	return (TileViewThumbnailSize + TileViewThumbnailPadding * 2) * GetTileViewUserScale();// *FMath::Lerp(MinThumbnailScale, MaxThumbnailScale, GetThumbnailScale());
}

float SLoaderWindow::GetTileViewUserScale() const
{
	// # todo
	return 1;
}

FText SLoaderWindow::DisplayTotalAssetsInLibrary() const
{
	// #todo check this for performance
	// Running this function on tick, is this a problem
	int assetCount = FMetadataOps::FindAllMetadataInLibrary().Num();

	FText Display = FText::Format(LOCTEXT("displayassetcountlabel", "Total Assets in library: {0}"),assetCount);
	return Display;
}


float SLoaderWindow::GetTileViewItemWidth() const
{
	return GetTileViewItemBaseWidth() * FillScale;
}



#undef LOCTEXT_NAMESPACE