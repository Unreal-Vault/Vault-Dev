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


class VAULT_API SPublisherWindow : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SPublisherWindow) {}
	SLATE_END_ARGS()

	// Construct Widget
	void Construct(const FArguments& InArgs);

	// On Tick
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	// Main data holder
	FVaultMetadata AssetPublishMetadata;

	//void UpdateAuthorMetadata(const FText& InText, ETextCommit::Type CommitMethod);

	void UpdateUserInputMetadata();

	void UpdateDescriptionMetadata(const FText& InText, ETextCommit::Type CommitMethod);

	void UpdateLastModifiedMetadata(const FText& InText, ETextCommit::Type CommitMethod);

	UTextureRenderTarget2D* CaptureThumbnailFromScene(); // old

	// Capture Thumbnail from the Screen
	FReply StartScreenshotCapture();

	UTextureRenderTarget2D* ChosenThumbnail; // old

	FSlateBrush ConvertRenderTargetToSlateBrush(); //old

	TSharedPtr<SImage> ThumbnailImage; //old

	FString LastScreenshotPath;
	int ScreenshotFrameCounter;
	int ScreenshotFrameTryForDuration = 120;
	void OnThumbnailReady();
	bool bScreenshotRequested;
	FSlateBrush ThumbBrush;

	TSharedPtr<SImage> ThumbnailPreviewBox;

	FSlateBrush* ImageSlateBrush;

	UTexture2D* CreateThumbnailFromScene();

	UTexture2D* CreateThumbnailFromFile();

	// All finished, gather everything and package.
	FReply TryPackage();

	FReply AddSelectedToList();

	void OnPrimaryAssetListChanged();

	TSharedPtr<SMultiLineEditableTextBox> OutputLogBox;

	TArray<FString> GetAssetDependancies(const FName AssetPath) const;

	TSharedPtr<SViewport> ViewportWidget;
	TSharedPtr<FSceneViewport> ViewportScene;
	UGameViewportClient* ViewportClient;
	UWorld* ViewportWorld;

	TSharedPtr<SMultiLineEditableTextBox> PrimaryAssetsBox;
	TSharedPtr<SMultiLineEditableTextBox> SecondaryAssetsBox;

	// This is the details view for the asset publisher view
	TSharedPtr<IDetailsView> AssetPublisherDetailsView;

	TSharedPtr<IDetailsView> AssetPublisherTagsView;

	TSharedPtr<FUICommandList> CommandList;


	FText GetPrimaryAssetList() const;
	FText GetSecondaryAssetList() const;

	// Check if we are all ready to publish (controls publish button enabled)
	bool CanPackage();

	// Stores if our last screenshot loaded back successfully. Mainly for packaging checks
	bool bHasValidScreenshot = false;

};



