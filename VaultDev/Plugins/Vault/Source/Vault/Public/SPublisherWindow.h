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
	SLATE_BEGIN_ARGS(SPublisherWindow)
	{
	}

	SLATE_END_ARGS()

	// Constructor
	SPublisherWindow();

	~SPublisherWindow();



	// Main
	void Construct(const FArguments& InArgs);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:

	void HandleMapUnload(UObject* Object);

	// Main data holder
	FVaultMetadata AssetPublishMetadata;


	void UpdateAuthorMetadata(const FText& InText, ETextCommit::Type CommitMethod);

	void UpdateUserInputMetadata();

	void UpdateDescriptionMetadata(const FText& InText, ETextCommit::Type CommitMethod);


	void UpdateCreationDateMetadata(const FText& InText, ETextCommit::Type CommitMethod);

	void UpdateLastModifiedMetadata(const FText& InText, ETextCommit::Type CommitMethod);

	void UpdateRelativePathMetadata(const FText& InText, ETextCommit::Type CommitMethod);

	UTextureRenderTarget2D* CaptureThumbnailFromScene(); // old

	UTextureRenderTarget2D* CaptureThumbnailViaScreencapture(); //old

	FReply StartScreenshotCapture(); //old

	UTextureRenderTarget2D* ChosenThumbnail; // old


	FSlateBrush ConvertRenderTargetToSlateBrush(); //old


	TSharedPtr<SImage> ThumbnailImage; //old

	// all old
	void OnThumbnailCaptured(FString Path);
	FString LastScreenshotPath;
	int ScreenshotFrameCounter;
	int ScreenshotFrameTryForDuration = 120;
	void OnThumbnailReady();
	bool bScreenshotRequested;
	FSlateBrush ThumbBrush;


	// text entry variables



	TSharedPtr<SImage> ThumbnailPreviewBox;

	FSlateBrush* ImageSlateBrush;


	UTexture2D* CreateThumbnailFromScene();

	UTexture2D* CreateThumbnailFromFile();


	// All finished, gather everything and package.
	FReply TryPackage();

	FReply AddSelectedToList();

	void UpdateDependanciesList();


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

	void SetupMicroViewport();

	FText GetPrimaryAssetList() const;
	FText GetSecondaryAssetList() const;






};



