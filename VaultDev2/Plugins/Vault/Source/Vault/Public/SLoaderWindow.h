// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Slate.h"
#include "SlateExtras.h"
#include "VaultTypes.h"

typedef TSharedPtr<FTagFilteringItem> FTagFilteringItemPtr;
typedef TSharedPtr<FDeveloperFilteringItem> FDeveloperFilteringItemPtr;

class VAULT_API SLoaderWindow : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SLoaderWindow)
	{
	}

	SLATE_END_ARGS()


	SLoaderWindow();

	~SLoaderWindow();

	void Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab, const TSharedPtr<SWindow>& ConstructUnderWindow);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	void HandleMapChange(UObject* Object);

	void PopulateBaseAssetList();


	// Static Base Values
	static const int32 THUMBNAIL_BASE_HEIGHT;
	static const int32 THUMBNAIL_BASE_WIDTH;
	static const int32 TILE_BASE_HEIGHT;
	static const int32 TILE_BASE_WIDTH;




	// Construction Functions for the Main Areas of the UI
	//ConstructAssetTiles();





	// Tables

	TSharedRef<ITableRow> MakeTileViewWidget(TSharedPtr<FVaultMetadata> AssetItem, const TSharedRef<STableViewBase>& OwnerTable);

	TSharedRef<ITableRow> MakeListViewWidget(TSharedPtr<FVaultMetadata> AssetItem, const TSharedRef<STableViewBase>& OwnerTable);

	TSharedRef<ITableRow> MakeTagFilterViewWidget(FTagFilteringItemPtr inTag, const TSharedRef<STableViewBase>& OwnerTable) const;

	TSharedRef<ITableRow> MakeDeveloperFilterViewWidget(FDeveloperFilteringItemPtr Entry, const TSharedRef<STableViewBase>& OwnerTable) const;

	TSharedRef<ITableRow> MakeMetaFilterViewWidget(TSharedPtr<FVaultMetadata> AssetItem, const TSharedRef<STableViewBase>& OwnerTable);

	// Scale Slider
	TSharedPtr<SSlider> UserScaleSlider;
	TSharedPtr<SCheckBox> StrictSearchCheckBox;


	TArray<FTagFilteringItemPtr> TagCloud;

	TArray<FDeveloperFilteringItemPtr> DeveloperCloud;


	TSharedPtr<SSearchBox> SearchBox;

	void OnSearchBoxChanged(const FText& inSearchText);

	void OnSearchBoxCommitted(const FText& InFilterText, ETextCommit::Type CommitType);

	TArray<TSharedPtr<FVaultMetadata>> FilteredAssetItems;

	float TileUserScale = 1.0f;

	void OnThumbnailSliderValueChanged(float Value);



	TSharedPtr<STileView<TSharedPtr<FVaultMetadata>>> TileView;

	FText DisplayTotalAssetsInLibrary() const;


};



