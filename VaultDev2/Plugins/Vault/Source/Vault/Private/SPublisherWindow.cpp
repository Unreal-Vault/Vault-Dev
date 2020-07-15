// Fill out your copyright notice in the Description page of Project Settings.

#include "SPublisherWindow.h"
#include "Misc/DateTime.h"
#include "Json.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Slate/SceneViewport.h"
#include <Kismet/GameplayStatics.h>
#include <Editor.h>
#include <EngineModule.h>
#include <Kismet/KismetRenderingLibrary.h>
#include "Engine/SceneCapture2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Styling/SlateBrush.h"
#include "VaultSettings.h"
#include "WorkflowOrientedApp/SContentReference.h" //required?
#include "PropertyEditorModule.h"
#include <AssetRegistryModule.h>
#include <Editor/EditorEngine.h>
#include <ContentBrowserModule.h> //required?
#include "AssetPublisher.h"
#include "AssetPublisherTagsCustomization.h"
#include "Vault.h"

// new
#include "IDetailsView.h"
#include "Framework\Commands\UICommandList.h"
#include <LevelEditor.h>
#include "Slate/SGameLayerManager.h"
#include <Engine/AssetManager.h>
#include "HAL/PlatformFilemanager.h"
#include "HAL/FileManager.h"
#include "HighResScreenshot.h" //required?
#include <ImageUtils.h>
#include <FunctionalUIScreenshotTest.h>
#include <EditorSupportDelegates.h>
#include "IDesktopPlatform.h"
#include "DesktopPlatformModule.h"


#define LOCTEXT_NAMESPACE "FVaultPublisher"

SPublisherWindow::SPublisherWindow()
{

}

SPublisherWindow::~SPublisherWindow()
{
#if WITH_EDITOR
	FEditorSupportDelegates::PrepareToCleanseEditorObject.RemoveAll(this);
#endif


}

void SPublisherWindow::Construct(const FArguments& InArgs)
{

	// Command List
	CommandList = MakeShareable(new FUICommandList);

	// Get Property Module
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	// Store a shared this
	TWeakPtr<SPublisherWindow> WeakPtr = SharedThis(this);

	// Setup the look
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bAllowSearch = false;
	   

	// Create the Details view widget
	AssetPublisherDetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	AssetPublisherTagsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	//AssetPublisherTagsView->RegisterInstancedCustomPropertyLayout(UAssetPublisherTags::StaticClass(), FOnGetDetailCustomizationInstance::CreateStatic(&FAssetPublisherTagsCustomization::MakeInstance, WeakPtr));
	//AssetPublisherTagsView->SetObject(GetMutableDefault<UAssetPublisherTags>());

	AssetPublisherDetailsView->RegisterInstancedCustomPropertyLayout(UAssetPublisher::StaticClass(), FOnGetDetailCustomizationInstance::CreateStatic(&FAssetPublisherTagsCustomization::MakeInstance, WeakPtr));
	AssetPublisherDetailsView->SetObject(GetMutableDefault<UAssetPublisher>());


	//SetupMicroViewport();
	
	// Start UI
	TSharedRef<SVerticalBox> MainVerticalBox = SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.AutoHeight()
		.Padding(5.f, 5.f, 5.f, 5.f)
		[

			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
				[
					SNew(SBox)
					.MaxAspectRatio(1.0f)
					.HeightOverride(256.0f)
					.WidthOverride(256.0f)
					[
						SAssignNew(ThumbnailImage, SImage)
						.Image(&ThumbBrush)
					]
				]
			+ SHorizontalBox::Slot()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
				[
					SNew(SButton)
					.Text(LOCTEXT("TakeScreenshotLabel", "Take Screenshot"))
					.OnClicked(this, &SPublisherWindow::StartScreenshotCapture)
				]

			+ SVerticalBox::Slot()
				[
					SNew(SButton)
					.Text(LOCTEXT("GeneratePythonMapLabel", "Generate Map from Python"))
					.OnClicked(this, &SPublisherWindow::StartScreenshotCapture)
				]

			+ SVerticalBox::Slot()
				[
					SNew(SButton)
					.Text(LOCTEXT("LoadPresetMapLabel", "Load Map from Preset Map"))
					.OnClicked(this, &SPublisherWindow::StartScreenshotCapture)
				]

				]
		]


	// Add the Details View, this contains most of the UI
	+ SVerticalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.AutoHeight()
		.Padding(0.0)
		[
			AssetPublisherDetailsView.ToSharedRef()
		]

	+SVerticalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.AutoHeight()
		.Padding(0.0)
		[
			AssetPublisherTagsView.ToSharedRef()
		]
	

	+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.Padding(0.0)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SHorizontalBox)
				// Left Slot - Primary Assets
				+SHorizontalBox::Slot()
				.Padding(0,0,10,0)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("primaryAssetBoxHeaderLbl", "Primary Assets to Package"))
					]

					+ SVerticalBox::Slot()
						.AutoHeight()
					[
						SAssignNew(PrimaryAssetsBox, SMultiLineEditableTextBox)
						.IsReadOnly(true)
						.AllowMultiLine(true)
						.AlwaysShowScrollbars(false)
						.Text(this, &SPublisherWindow::GetPrimaryAssetList)
					]
				]




				+ SHorizontalBox::Slot()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("seecondaryAssetBoxHeaderLbl", "Additional Referenced Assets to Package"))
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SAssignNew(SecondaryAssetsBox, SMultiLineEditableTextBox)
						.IsReadOnly(true)
						.AllowMultiLine(true)
						.AlwaysShowScrollbars(false)
						.Text(this, &SPublisherWindow::GetSecondaryAssetList)
					]
				]
			]
		]
		
		// Confirm buttons, these will go at the bottom, so new things go above this.
		+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Center)
			.Padding(5.f, 5.f, 5.f, 5.f)
			[
				SNew(SButton)
				.OnClicked(this, &SPublisherWindow::TryPackage)
				.Text(LOCTEXT("SubmitToVaultLabel", "Submit to Vault"))
			]

		+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Bottom)
			.Padding(5.f, 5.f, 5.f, 5.f)
			[
				SAssignNew(OutputLogBox, SMultiLineEditableTextBox)
				.AllowContextMenu(false)
				.AllowMultiLine(true)
				.IsReadOnly(true)
				.AutoWrapText(false)
				.Text(LOCTEXT("tmpLabel", "Output data display - tmp text"))
			];
	
		//+ SVerticalBox::Slot()
		//	[
		//		SNew(SOutputLog)
		//	
		//	]
		
		ChildSlot
		[
			MainVerticalBox
		];


#if WITH_EDITOR
		FEditorSupportDelegates::PrepareToCleanseEditorObject.AddRaw(this, &SPublisherWindow::HandleMapUnload);
#endif


}

void SPublisherWindow::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	//Super::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

		
	if (FApp::CanEverRender())
	{
		//ViewportScene->Draw(true);
	}

	if (bScreenshotRequested && ScreenshotFrameCounter < ScreenshotFrameTryForDuration)
	{
		ScreenshotFrameCounter++;
		if (FPaths::FileExists(LastScreenshotPath))
		{
			OnThumbnailReady();
		}
	}

	
}

// Safely remove anything that might cause a crash when changing map
void SPublisherWindow::HandleMapUnload(UObject* Object)
{
	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();

	if (EditorWorld == Object)
	{
		// When a map object is about to be GC'd we want to make sure the UI releases all references
		// to anything that is owned by the scene.
		//AssetPublisherDetailsView->SetObject(nullptr);
		AssetPublisherTagsView->SetObject(nullptr);

		// Force the list view to rebuild after clearing it's data source. This clears the list view's
		// widget children. The ListView should only contain TWeakObjectPtrs but it's holding a hard
		// reference anyways that's causes a gc leak on map change.
		/*FSequenceRecorder::Get().ClearQueuedRecordings();
		ActorListView->RebuildList();
		for (TSharedPtr<SListView<USequenceRecordingBase*>>& ListView : ExtenderListViews)
		{
			ListView->RebuildList();
		}*/

		// We also want to construct a new mutable default so it resets the recording paths to the
		// default paths for the new map.
		//RecordingGroupDetailsView->SetObject(GetMutableDefault<USequenceRecorderActorGroup>());
		//AssetPublisherDetailsView->SetObject(GetMutableDefault<UAssetPublisher>());
		AssetPublisherTagsView->SetObject(GetMutableDefault<UAssetPublisherTags>());
	}

}

void SPublisherWindow::UpdateRelativePathMetadata(const FText& InText, ETextCommit::Type CommitMethod)
{
	//AssetPublishMetadata.RelativePath
}

UTextureRenderTarget2D* SPublisherWindow::CaptureThumbnailViaScreencapture()
{
	// #todo

	//FLevelEditorModule& LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));

	//LevelEditor->GetFirstActiveViewport()-
	return nullptr;
}

FReply SPublisherWindow::StartScreenshotCapture()
{
//	
	//GEditor->GameViewport->OnScreenshotCaptured().AddRaw(this, &SPublisherWindow::OnThumbnailCaptured);
	const FString OutputDirectory = FVaultSettings::Get().GetAssetLibraryRoot();

	const FString TempName = TEXT("Temp.png");


	FString path = OutputDirectory / TempName;
	FPaths::NormalizeFilename(path);


	//FImageUtils::CreateTexture2D()
	//FImageUtils::ImportFileAsTexture2D()


	//.Image(FEditorStyle::GetBrush("HighresScreenshot.Capture"))
	//FUnrealClient::

	GetHighResScreenshotConfig().SetResolution(512, 512, 1.0f);
	GetHighResScreenshotConfig().FilenameOverride = path;
	//FScreenshotRequest::RequestScreenshot(path, false, false);
	GEditor->TakeHighResScreenShots();

	OnThumbnailCaptured(path);

	//const FVector2D TextureSize = FVector2D(256.0, 256.0);

	//ThumbBrush.SetResourceObject(Tex);
	//ThumbBrush.ImageSize = TextureSize;
	//ThumbBrush.DrawAs = ESlateBrushDrawType::Image;

	//ThumbnailImage->SetImage(&ThumbBrush);

	

	//FLevelEditorModule& LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>(LevelEditorModName);
	//LevelEditor.

	

	/*FTexture2DDynamicCreateInfo info;
	info.bSRGB = true;
	info.
	TArray<FColor> col;
	UTexture2DDynamic* img = UTexture2DDynamic(col);

	viewport.onscreenshotcaptured*/

	return FReply::Handled();

}

FSlateBrush SPublisherWindow::ConvertRenderTargetToSlateBrush()
{
	ImageSlateBrush = nullptr;
	//FSlateBrush NewBrush(ESlateBrushDrawType::Image, "ThumbnailBrush", FMargin::Bottom, ESlateBrushTileType::NoTile, ESlateBrushImageType::FullColor, FVector2D(ChosenThumbnail->SizeX, ChosenThumbnail->SizeY));
	FSlateBrush NewBrush;
	NewBrush.SetResourceObject(ChosenThumbnail);
	ImageSlateBrush = &NewBrush;

	return *ImageSlateBrush;
}


void SPublisherWindow::OnThumbnailCaptured(FString Path)
{
	ScreenshotFrameCounter = 0;
	LastScreenshotPath = Path;
	bScreenshotRequested = true;
	//FCreateTexture2DParameters OutParams;
	//UTexture2D* MyTex = FImageUtils::CreateTexture2D(Width, Height, Colors, GetTransientPackage(), "ThumbTex", EObjectFlags::RF_Transient, OutParams);




	
}

void SPublisherWindow::OnThumbnailReady()
{
	bScreenshotRequested = false;
	UTexture2D* ThumbTexture2D = FImageUtils::ImportFileAsTexture2D(LastScreenshotPath);

	if (ThumbTexture2D)
	{
		const FVector2D TextureSize = FVector2D(256.0, 256.0);

		ThumbBrush.SetResourceObject(ThumbTexture2D);
		ThumbBrush.ImageSize = TextureSize;
		ThumbBrush.DrawAs = ESlateBrushDrawType::Image;

		//ThumbnailImage->SetImage(&ThumbBrush);


	}
}

UTexture2D* SPublisherWindow::CreateThumbnailFromScene()
{

	FViewport* Viewport = GEditor->GetActiveViewport();

	if (!ensure(GCurrentLevelEditingViewportClient || !ensure(Viewport)))
	{
		return nullptr;
	}

	// Cache viewport ptr
	FLevelEditorViewportClient* OldViewportClient = GCurrentLevelEditingViewportClient;

	// Clear to remove viewports selection box
	GCurrentLevelEditingViewportClient = nullptr;

	// Redraw viewport
	Viewport->Draw();

	const uint32 SrcWidth = Viewport->GetSizeXY().X;
	const uint32 SrcHeight = Viewport->GetSizeXY().Y;

	TArray<FColor> OrigBitmap;
	if (!Viewport->ReadPixels(OrigBitmap) || OrigBitmap.Num() != SrcWidth * SrcHeight)
	{
		return nullptr;
	}

	TArray<FColor> ScaledBitmap;
	FImageUtils::CropAndScaleImage(SrcWidth, SrcHeight, 128, 128, OrigBitmap, ScaledBitmap);

	// Redraw viewport to restore highlight
	GCurrentLevelEditingViewportClient = OldViewportClient;
	Viewport->Draw();

	FScopedTransaction Transaction(LOCTEXT("CapturePackageThumbnail", "Capture Package Image from Viewport"));

	FCreateTexture2DParameters Params;
	Params.bDeferCompression = true;

	UTexture2D* ResizedTexture = FImageUtils::CreateTexture2D(128, 128, ScaledBitmap, GEditor->GetWorld(), FString(), RF_NoFlags, Params);
	if (ResizedTexture)
	{
		return ResizedTexture;
	}
	return nullptr;

}

UTexture2D* SPublisherWindow::CreateThumbnailFromFile()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform == nullptr)
	{
		return nullptr;
	}

	// Formats that IImageWrapper can handle
	const FString Filter = TEXT("Image files (*.jpg; *.png; *.bmp; *.ico; *.exr; *.icns)|*.jpg; *.png; *.bmp; *.ico; *.exr; *.icns|All files (*.*)|*.*");
	TArray<FString> OutFiles;
	TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());
	void* ParentWindowHandle = (ParentWindow.IsValid() && ParentWindow->GetNativeWindow().IsValid()) ? ParentWindow->GetNativeWindow()->GetOSWindowHandle() : nullptr;

	bool bPickedFile = DesktopPlatform->OpenFileDialog(
		ParentWindowHandle,
		LOCTEXT("LoadThumbnailFromFileTitle", "Choose a file for the thumbnail").ToString(),
		TEXT(""), TEXT(""),
		Filter,
		EFileDialogFlags::None,
		OutFiles);

	if (!bPickedFile || OutFiles.Num() != 1)
	{
		return nullptr;
	}

	FString SourceImagePath = FPaths::ConvertRelativePathToFull(OutFiles[0]);
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*SourceImagePath))
	{
		return nullptr;
	}

	UTexture2D* OriginalTexture = FImageUtils::ImportFileAsTexture2D(SourceImagePath);
	if (!OriginalTexture)
	{
		return nullptr;
	}

	FScopedTransaction Transaction(FText::Format(LOCTEXT("LoadThumbnailTransaction", "Load file '{0}' as a package thumbnail"), FText::FromString(SourceImagePath)));

	return OriginalTexture;
}

FReply SPublisherWindow::TryPackage()
{
	UE_LOG(LogVault, Display, TEXT("Starting Packaging Operation"));

	// Get Default Asset - May be expanded to array
	TSoftObjectPtr<UObject> Asset = GetMutableDefault<UAssetPublisher>()->PrimaryAsset;

	if (!Asset->IsValidLowLevel())
	{
		UE_LOG(LogVault, Error, TEXT("No Asset Selected"));
		return FReply::Handled();
	}

	// Rename screenshot
	const FString OutputDirectory = FVaultSettings::Get().GetAssetLibraryRoot();
	const FString TempName = TEXT("Temp.png");
	const FString ScreenshotPath = OutputDirectory / TempName;
	const FString RenamedScreenshot = OutputDirectory / GetMutableDefault<UAssetPublisher>()->PackName.ToString() + TEXT(".png");

	if (FPaths::FileExists(ScreenshotPath))
	{
		IFileManager::Get().Move(*RenamedScreenshot, *ScreenshotPath, true, false);
	}

	
	// Find AssetData
	FAssetData AssetPublisherData;
	UAssetManager::GetIfValid()->GetAssetDataForPath(Asset.ToSoftObjectPath(), AssetPublisherData);

	// Store PackageName
	const FString AssetPath = AssetPublisherData.PackageName.ToString();

	TSet<FString> PublishList;

	TSet<FString> AssetsToProcess = { AssetPublisherData.PackageName.ToString() };
	AssetsToProcess.Append(GetAssetDependancies(AssetPublisherData.PackageName));


	// Loop through all Assets, including the root object, and format into the correct absolute system filename for the UnrealPak operation
	for (const FString Path : AssetsToProcess)
	{
		UE_LOG(LogVault, Display, TEXT("Processing Object Path: %s"), *Path);

		FString Filename;
		bool bGotFilename = FPackageName::TryConvertLongPackageNameToFilename(Path, Filename);
		//FString SuperAbs = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*Filename);

		// Find the UPackage to determine the asset type
		UPackage* Package = FindPackage(nullptr, *AssetPublisherData.PackageName.ToString());

		if (!Package)
		{
			UE_LOG(LogVault, Error, TEXT("Unable to find UPackage for %s"), *AssetPublisherData.PackageName.ToString());
		}

		// Get and append the asset extension
		const FString ExtensionName = Package->ContainsMap() ? FPackageName::GetMapPackageExtension() : FPackageName::GetAssetPackageExtension();

		// Append extension into filename
		Filename += ExtensionName;
		
		PublishList.Add(Filename);
	}


	UpdateUserInputMetadata();
	UAssetPublisher::PackageSelected(PublishList, AssetPublishMetadata);
	return FReply::Handled();
}

void SPublisherWindow::UpdateDependanciesList()
{

}

void SPublisherWindow::OnPrimaryAssetListChanged()
{
	AddSelectedToList();
}

FReply SPublisherWindow::AddSelectedToList()
{
	// Check if we have something selected:
	if (!GetDefault<UAssetPublisher>()->PrimaryAsset->IsValidLowLevel())
	{
		return FReply::Handled();
	}


	FText PriorText = PrimaryAssetsBox->GetText();
	FString NewText = PriorText.ToString();


	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FAssetData> SelectedAssets;
	ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);
	TArray<FString> Result;
	TArray<FString> SecondaryResult;
	for (FAssetData& AssetData : SelectedAssets) {
		Result.Add(AssetData.PackageName.ToString());

		NewText += "," + AssetData.PackageName.ToString();

		SecondaryResult.Append(GetAssetDependancies((AssetData.PackageName)));
	}
	
	
	//const UAssetPublisher* APPtr = GetDefault<UAssetPublisher>();


	PrimaryAssetsBox->SetText(FText::FromName(GetDefault<UAssetPublisher>()->PrimaryAsset->GetFName()));

	
	//PrimaryAssetsBox->SetText(LOCTEXT("supermp", "Test New Text"));
	PrimaryAssetsBox->Refresh();

	FString SecondaryAssetsBoxText;
	
	for (FString Secondary: SecondaryResult)
	{
		SecondaryAssetsBoxText += Secondary;
	}

	SecondaryAssetsBox->SetText(FText::FromString(SecondaryAssetsBoxText));
	return FReply::Handled();
}

TArray<FString> SPublisherWindow::GetAssetDependancies(const FName AssetPath) const
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FName> Dependencies;
	AssetRegistryModule.Get().GetDependencies(AssetPath, Dependencies);

	TArray<FString> DependenciesList;

	for (FName& name : Dependencies)
	{
		DependenciesList.Add(name.ToString());
	}
	return DependenciesList;

}

void SPublisherWindow::SetupMicroViewport()
{
	ViewportClient = NewObject<UGameViewportClient>(GEngine, UGameViewportClient::StaticClass());

	ViewportWorld = UWorld::CreateWorld(EWorldType::Editor, true);
	GEngine->DestroyWorldContext(ViewportWorld);


	TSharedPtr<SOverlay> ViewportOverlayWidget = SNew(SOverlay);
	
	TSharedRef<SGameLayerManager> LayerManagerRef = SNew(SGameLayerManager)
		.SceneViewport(ViewportClient->GetGameViewport())
		.Visibility(EVisibility::Visible)
		[
			ViewportOverlayWidget.ToSharedRef()
		];


	ViewportWidget = SNew(SViewport)
		.RenderDirectlyToWindow(false)
		[
			LayerManagerRef
		];

	
	ViewportScene = MakeShareable(new FSceneViewport(ViewportClient, ViewportWidget));
	ViewportWidget->SetViewportInterface(ViewportScene.ToSharedRef());
	
	ViewportClient->SetGameLayerManager(LayerManagerRef);
	
	ViewportClient->Viewport = ViewportScene.Get();
	


	//ViewportScene = MakeShareable(new FSceneViewport(GEngine->GameViewport, ViewportWidget));
	//ViewportWidget->SetViewportInterface(ViewportScene.ToSharedRef());


}

FText SPublisherWindow::GetSecondaryAssetList() const
{
	TSoftObjectPtr<UObject> Asset = GetMutableDefault<UAssetPublisher>()->PrimaryAsset;

	if (Asset)
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
		//AssetRegistryModule.Get().
		//Asset.ToStringToSoftObjectPath();
		FAssetData AssetPublisherData;
		UAssetManager::GetIfValid()->GetAssetDataForPath(Asset.ToSoftObjectPath(), AssetPublisherData);

		TArray<FString> Dependancies = GetAssetDependancies(AssetPublisherData.PackageName);
		Dependancies.Remove(AssetPublisherData.PackageName.ToString());

		FString FormattedList;
		for (FString Dependant : Dependancies)
		{
			FormattedList += (Dependant);
			FormattedList += LINE_TERMINATOR;
		}
			   		 	  
		return FText::FromString(FormattedList);
	}
	return FText::GetEmpty();
}

FText SPublisherWindow::GetPrimaryAssetList() const
{
	TSoftObjectPtr<UObject> Asset = GetMutableDefault<UAssetPublisher>()->PrimaryAsset;

	if (Asset)
	{
		return FText::FromString(Asset->GetName());
	}
	return FText::GetEmpty();
}

UTextureRenderTarget2D* SPublisherWindow::CaptureThumbnailFromScene()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();

	FIntPoint TexSize = FIntPoint(256, 256);

	if (World)
	{
		UTextureRenderTarget2D* NewRenderTarget2D = NewObject<UTextureRenderTarget2D>(World);
		check(NewRenderTarget2D);
		NewRenderTarget2D->RenderTargetFormat = RTF_RGBA16f;
		NewRenderTarget2D->InitAutoFormat(TexSize.X, TexSize.Y);
		NewRenderTarget2D->UpdateResourceImmediate(true);

		ASceneCapture2D* SceneCapture = NewObject<ASceneCapture2D>(World);
		SceneCapture->GetCaptureComponent2D()->TextureTarget = NewRenderTarget2D;
		SceneCapture->GetCaptureComponent2D()->bCaptureOnMovement = false;
		SceneCapture->GetCaptureComponent2D()->bCaptureEveryFrame = false;
		SceneCapture->GetCaptureComponent2D()->CaptureScene();


		return NewRenderTarget2D;

	}
	return nullptr;
}

void SPublisherWindow::UpdateLastModifiedMetadata(const FText& InText, ETextCommit::Type CommitMethod)
{
	AssetPublishMetadata.LastModified = FDateTime::UtcNow();
}

void SPublisherWindow::UpdateCreationDateMetadata(const FText& InText, ETextCommit::Type CommitMethod)
{
	// all temp and not part of this
	



}



void SPublisherWindow::UpdateAuthorMetadata(const FText& InText, ETextCommit::Type CommitMethod)
{
	//AssetPublishMetadata.Author = InText;
}

void SPublisherWindow::UpdateUserInputMetadata()
{
	AssetPublishMetadata.Author = GetMutableDefault<UAssetPublisher>()->Author;
	AssetPublishMetadata.PackName = GetMutableDefault<UAssetPublisher>()->PackName;
	AssetPublishMetadata.Description = GetMutableDefault<UAssetPublisher>()->Description;
	AssetPublishMetadata.CreationDate = FDateTime::UtcNow();
	AssetPublishMetadata.LastModified = FDateTime::UtcNow();

	//AssetPublisherTagsView->Get
	//TArray<FString> TagArrayParsed;
	//GetMutableDefault<UAssetPublisherTags>()->TagsListInternal.ParseIntoArray(TagArrayParsed, TEXT(","));

	//TSet<FString> Array;
	//Array.Append(TagArrayParsed);


	//AssetPublishMetadata.Tags = Array;
}

void SPublisherWindow::UpdateDescriptionMetadata(const FText& InText, ETextCommit::Type CommitMethod)
{
	//AssetPublishMetadata.Description = InText;
}






#undef LOCTEXT_NAMESPACE

