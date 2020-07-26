// Fill out your copyright notice in the Description page of Project Settings.

#include "SAssetPackTile.h"

#include "VaultSettings.h"
#include "SlateBasics.h"

#include "MetadataOps.h"
#include "ImageUtils.h"

#include "Engine/Texture2D.h"

#include "Styling/SlateBrush.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Images/SThrobber.h"

#define LOCTEXT_NAMESPACE "VaultListsDefinitions"


SAssetTileItem::SAssetTileItem() {}

SAssetTileItem::~SAssetTileItem()
{
	if (TextureResource)
	{
		TextureResource->ClearFlags(RF_Standalone);
	}
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SAssetTileItem::Construct(const FArguments& InArgs)
{
	AssetItem = InArgs._AssetItem;

	//FSlateBrush* ThumbBrush = new FSlateBrush();
	TSharedRef<SWidget> ThumbnailWidget = CreateTileThumbnail(AssetItem);
	
	// Clear Old
	this->ChildSlot [ SNullWidget::NullWidget ];
	
	TSharedPtr<SVerticalBox> StandardWidget =
		SNew(SVerticalBox)

		// Image Area
		+SVerticalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.FillHeight(0.9f)
		.Padding(FMargin(0.0f, 3.0f, 0.0f, 0.0f))
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			[
				// Optional Overlay Box to help additional meta later in pipe. 
				SNew(SOverlay)

				+SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					ThumbnailWidget
				]
			]
		]

		// File Name
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.AutoWidth()
			.Padding(FMargin(8.0f, 11.0f, 3.0f, 0.0f))
				[
					SNew(STextBlock)
					.Text(FText::FromName(InArgs._AssetItem->PackName.IsNone() ? TEXT("Unknown Pack") : AssetItem->PackName))
					.WrapTextAt(300)
					.Justification(ETextJustify::Left)
					//.TextStyle(F)
				]
		];
		
		ChildSlot
		[
			StandardWidget->AsShared()
		];


}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<SWidget> SAssetTileItem::CreateTileThumbnail(TSharedPtr<FVaultMetadata> Meta)
{
	const FString Root = FVaultSettings::Get().GetAssetLibraryRoot();
	const FString Pack = Meta->PackName.ToString();
	const FString Filepath = Root / Pack + TEXT(".png");

	if (FPaths::FileExists(Filepath) == false)
	{
		return SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage) 
				.Image(FEditorStyle::GetDefaultBrush())
			];
	}
	
	UTexture2D* ThumbTexture = FImageUtils::ImportFileAsTexture2D(Filepath);
	ThumbTexture->SetFlags(RF_Standalone);

	Brush = MakeShareable(new FSlateBrush());

	if (ThumbTexture)
	{
		Brush->SetResourceObject(ThumbTexture);
		Brush->DrawAs = ESlateBrushDrawType::Image;
		TextureResource = Brush->GetResourceObject();

	}

	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(Brush.Get())
			.Visibility(EVisibility::SelfHitTestInvisible)
		];
}

#undef LOCTEXT_NAMESPACE