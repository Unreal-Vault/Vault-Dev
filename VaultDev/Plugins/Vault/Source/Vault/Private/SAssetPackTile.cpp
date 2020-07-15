// Fill out your copyright notice in the Description page of Project Settings.

#include "SAssetPackTile.h"
#include "Slate.h"
#include "SlateExtras.h"
#include "Internationalization\BreakIterator.h"
#include "MetadataOps.h"
#include "ImageUtils.h"
#include "VaultSettings.h"
#include "Engine/Texture2D.h"

#define LOCTEXT_NAMESPACE "VaultListsDefinitions"

SAssetPackItem::SAssetPackItem()
{
	
}

SAssetPackItem::~SAssetPackItem()
{

}

void SAssetPackItem::Construct(const FArguments& InArgs)
{
	AssetItem = InArgs._AssetItem;
}



// ------------------- list ---------------------------
SAssetListItem::SAssetListItem()
{

}

SAssetListItem::~SAssetListItem()
{

}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SAssetListItem::Construct(const FArguments& InArgs)
{
	// Pass Asset to parent super
	SAssetPackItem::Construct(SAssetPackItem::FArguments()
		.AssetItem(InArgs._AssetItem)
	);


}


// -------------------- tile ---------------------------
SAssetTileItem::SAssetTileItem()
{

}

SAssetTileItem::~SAssetTileItem()
{

}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SAssetTileItem::Construct(const FArguments& InArgs)
{
	// Pass Asset to parent super
	SAssetPackItem::Construct(SAssetPackItem::FArguments()
		.AssetItem(InArgs._AssetItem)
	);

	ItemWidth = InArgs._ItemWidth;

	FSlateBrush* ThumbBrush = new FSlateBrush();
	TSharedRef<SWidget> ThumbnailWidget = CreateTileThumbnail(AssetItem, ThumbBrush);


	

	UE_LOG(LogTemp, Warning, TEXT("Asset Tile Processing (%s)"), *AssetItem->PackName.ToString());


	ChildSlot
		[
			SNew(SBorder)
			.Padding(0)		
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SNew(SBox)
					.Padding(5.0f)
					.WidthOverride(this, &SAssetTileItem::GetThumbnailBoxSize)
					.HeightOverride(this, &SAssetTileItem::GetThumbnailBoxSize)
					[
						SNew(SBorder)
						.Padding(1.f)
						[				
							ThumbnailWidget
						]
					]
				]

				+ SVerticalBox::Slot()
				.Padding(FMargin(1.f, 0))
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.FillHeight(1.0f)
				[
		
					SNew(STextBlock)
					.Text(FText::FromName(InArgs._AssetItem->PackName.IsNone() ? TEXT("Pack") : AssetItem->PackName))
					.LineBreakPolicy(FBreakIterator::CreateCamelCaseBreakIterator())
		
		
				]
			]
		
		];
}

FOptionalSize SAssetTileItem::GetThumbnailBoxSize() const
{
	return FOptionalSize(ItemWidth.Get());
}

TSharedRef<SWidget> SAssetTileItem::CreateTileThumbnail(TSharedPtr<FVaultMetadata> Meta, FSlateBrush* InBrush)
{
	
		
	const FString Root = FVaultSettings::Get().GetAssetLibraryRoot();
	const FString Pack = Meta->PackName.ToString();
	const FString Filepath = Root / Pack + TEXT(".png");

	if (FPaths::FileExists(Filepath) == false)
	{
		return SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage) .Image(FEditorStyle::GetDefaultBrush())
			];
	}
	
	UTexture2D* ThumbTexture2D = FImageUtils::ImportFileAsTexture2D(Filepath);


	if (ThumbTexture2D)
	{
		InBrush->SetResourceObject(ThumbTexture2D);
		InBrush->DrawAs = ESlateBrushDrawType::Image;

	}

	TSharedRef<SOverlay> ItemContentsOverlay = SNew(SOverlay);

	ItemContentsOverlay->AddSlot()
		[
			SNew(SImage)
			.Image(InBrush)

		];

	ItemContentsOverlay->AddSlot()
		[
			SNew(STextBlock)
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(FSlateColor(FLinearColor::Green))
			.Text(FText::FromName(Meta->PackName))

		];

	return ItemContentsOverlay;


}




#undef LOCTEXT_NAMESPACE