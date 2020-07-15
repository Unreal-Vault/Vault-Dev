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


	// Tables

	TSharedRef<ITableRow> MakeTileViewWidget(TSharedPtr<FVaultMetadata> AssetItem, const TSharedRef<STableViewBase>& OwnerTable);

	TSharedRef<ITableRow> MakeListViewWidget(TSharedPtr<FVaultMetadata> AssetItem, const TSharedRef<STableViewBase>& OwnerTable);

	TSharedRef<ITableRow> MakeTagFilterViewWidget(FTagFilteringItemPtr inTag, const TSharedRef<STableViewBase>& OwnerTable) const;

	TSharedRef<ITableRow> MakeDeveloperFilterViewWidget(FDeveloperFilteringItemPtr Entry, const TSharedRef<STableViewBase>& OwnerTable) const;

	TSharedRef<ITableRow> MakeMetaFilterViewWidget(TSharedPtr<FVaultMetadata> AssetItem, const TSharedRef<STableViewBase>& OwnerTable);


	TArray<FTagFilteringItemPtr> TagCloud;

	TArray<FDeveloperFilteringItemPtr> DeveloperCloud;


	TSharedPtr<SSearchBox> SearchBox;

	void OnSearchBoxChanged(const FText& inSearchText);

	TArray<TSharedPtr<FVaultMetadata>> FilteredAssetItems;

	// neds to be swapped with local settings data
	float TileResolution = 256.0f;


	float GetTileViewItemWidth() const;

	float GetTileViewItemBaseWidth() const;

	float GetTileViewUserScale() const;

	float FillScale = 1;

	float TileViewThumbnailSize = 128.0;
	float TileViewThumbnailPadding = 5.f;

	TSharedPtr<STileView<TSharedPtr<FVaultMetadata>>> TileView;

	FText DisplayTotalAssetsInLibrary() const;


};



