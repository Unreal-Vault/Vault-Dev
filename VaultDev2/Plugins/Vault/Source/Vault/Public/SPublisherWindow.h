// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Slate.h"
#include "SlateExtras.h"
#include "VaultTypes.h"
#include <IDetailsView.h>
#include "Misc/TextFilterExpressionEvaluator.h"
#include <Engine/GameViewportClient.h>
#include "VaultOutputLog.h"


class VAULT_API SPublisherWindow : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SPublisherWindow) {}
	SLATE_END_ARGS()

	// Construct Widget
	void Construct(const FArguments& InArgs);

	~SPublisherWindow();

	// Construct our Thumbnail Widget
	TSharedPtr<SWidget> ConstructThumbnailWidget();

	TSharedPtr<SWidget> ThumbnailWidget;

	// Capture Thumbnail from the Screen
	FReply OnCaptureImageFromViewport();

	// Create Thumbnail from a File
	FReply OnCaptureImageFromFile();

	// Get Image Brush for SImage, or return null if not valid
	const FSlateBrush* GetThumbnailImage() const;


	//void DestroyThumbnail();

	// Viewport Shot, whether captured or loaded. Passed into our SlateBrush, this ref is mainly for checking streaming status
	UTexture2D* ShotTexture;

	// SImage Widget for our Thumbnail display
	TSharedPtr<SImage> ThumbnailImage;

	// Image Brush used by our SImage
	FSlateBrush ThumbBrush;

	TSharedPtr<SImage> ThumbnailPreviewBox;

	// Capture Thumbnail from the Screen
	UTexture2D* CreateThumbnailFromScene();

	// Capture Thumbnail from a File
	UTexture2D* CreateThumbnailFromFile();

	// All finished, gather everything and package.
	FReply TryPackage();

	TArray<FString> GetAssetDependancies(const FName AssetPath) const;

	TSharedPtr<SMultiLineEditableTextBox> SecondaryAssetsBox;

	FText GetSecondaryAssetList() const;

	// Check if we are all ready to publish (controls publish button enabled)
	bool CanPackage() const;

	// Stores if our last screenshot loaded back successfully. Mainly for packaging checks
	bool bHasValidScreenshot = false;

	// Output Log Object
	TSharedPtr<FVaultOutputLog> VaultOutputLog;
	TSharedPtr<SListView<TSharedPtr< FVaultLogMessage>>> VaultOutputLogList;
	TSharedRef<SWidget> ConstructOutputLog();

	TSharedRef<ITableRow> HandleVaultLogGenerateRow(TSharedPtr<FVaultLogMessage> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	void RefreshOutputLogList();

	FAssetData CurrentlySelectedAsset;
	FString GetCurrentAssetPath() const;
	void OnAssetSelected(const FAssetData& InAssetData);

	bool IsPythonMapGenAvailable() const;

	FReply GenerateMapFromPython();

	FReply GenerateMapFromPreset();

	// New user entry stuff
	TSharedPtr<SEditableTextBox> PackageNameInput;
	TSharedPtr<SEditableTextBox> AuthorInput;
	TSharedPtr<SMultiLineEditableTextBox> DescriptionInput;

	TSharedPtr<SExpandableArea> OutputLogExpandableBox;

	TSharedPtr<class SPublisherTagsWidget> TagsWidget;

};



