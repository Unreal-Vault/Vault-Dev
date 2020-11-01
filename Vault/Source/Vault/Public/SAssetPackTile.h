// Copyright Daniel Orchard 2020

#pragma once

#include "CoreMinimal.h"
#include "SlateFwd.h"
#include "VaultTypes.h"

class VAULT_API SAssetTileItem : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SAssetTileItem) {}

	/** Item to use for populating */
	SLATE_ARGUMENT(TSharedPtr<FVaultMetadata>, AssetItem)

	SLATE_END_ARGS()

	~SAssetTileItem();

	// On Construction
	void Construct(const FArguments& InArgs);

	// Create the Tile Thumbnail, Returns Widget ready to use
	TSharedRef<SWidget> CreateTileThumbnail(TSharedPtr<FVaultMetadata> Meta);

private:

	// Asset ref passed in on Construct by the Table Generator
	TSharedPtr<FVaultMetadata> AssetItem;

	// Holds the Thumbnail Brush (SlateBrush)
	TSharedPtr<FSlateBrush> Brush;

	/** Stores our resource for the texture used to clear that flags that keep it from GC */
	UObject* TextureResource;
	
};