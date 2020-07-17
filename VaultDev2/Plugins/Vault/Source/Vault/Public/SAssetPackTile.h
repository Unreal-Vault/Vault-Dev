// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "SlateFwd.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SWidget.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"
#include "EditorStyleSet.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STileView.h"
#include "VaultTypes.h"


class VAULT_API SAssetPackItem : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAssetPackItem) {}

	/** Data for the asset this item represents */
	SLATE_ARGUMENT(TSharedPtr<FVaultMetadata>, AssetItem)

	SLATE_END_ARGS()

	SAssetPackItem();
	~SAssetPackItem();
	TSharedPtr<FVaultMetadata> AssetItem;
	void Construct(const FArguments& InArgs);

};



class VAULT_API SAssetListItem : public SAssetPackItem
{
public:

	SLATE_BEGIN_ARGS(SAssetListItem)
		: _ItemWidth(16)
	{
	}

	/** Item to use for populating */
	SLATE_ARGUMENT(TSharedPtr<FVaultMetadata>, AssetItem)

	SLATE_ATTRIBUTE(float, ItemWidth)

	SLATE_END_ARGS()


	SAssetListItem();

	~SAssetListItem();

	void Construct(const FArguments& InArgs);

private:


	TAttribute<float> ItemWidth;
};

// Tile Child of SAssetPackItem
class VAULT_API SAssetTileItem : public SAssetPackItem
{
public:

	SLATE_BEGIN_ARGS(SAssetTileItem)
		: _ItemWidth(16)
	{}

	/** Item to use for populating */
	SLATE_ARGUMENT(TSharedPtr<FVaultMetadata>, AssetItem)

	SLATE_ATTRIBUTE(float, ItemWidth)

	SLATE_END_ARGS()


	SAssetTileItem();

	~SAssetTileItem();

	void Construct(const FArguments& InArgs);

	FOptionalSize GetThumbnailBoxSize() const;

	TSharedRef<SWidget> CreateTileThumbnail(TSharedPtr<FVaultMetadata> Meta, FSlateBrush* InBrush);

	FReply OnAssetTileClicked(const FGeometry& Geom, const FPointerEvent& PointerEvent);


private:


	TAttribute<float> ItemWidth;

};