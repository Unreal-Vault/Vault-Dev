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
#include "PropertyCustomizationHelpers.h"

#include "Widgets/Input/SEditableTextBox.h"

#include "EditorFontGlyphs.h"
#include "ImageWriteBlueprintLibrary.h"
#include <Interfaces/IPluginManager.h>
#include <FileHelpers.h>


#define LOCTEXT_NAMESPACE "FVaultPublisher"
#define VAULT_PUBLISHER_CAPTURE_SIZE 512
#define VAULT_PUBLISHER_THUMBNAIL_SIZE 256

class SVaultLogMessageListRow : public SMultiColumnTableRow<TSharedPtr<FVaultLogMessage>>
{
public:
	SLATE_BEGIN_ARGS(SVaultLogMessageListRow) { }
	SLATE_ARGUMENT(TSharedPtr<FVaultLogMessage>, Message)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
	{
		Message = InArgs._Message;
		SMultiColumnTableRow<TSharedPtr<FVaultLogMessage>>::Construct(FSuperRowType::FArguments(), InOwnerTableView);
	}

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		return SNew(SBox)
			.Padding(FMargin(4.0, 0.0))
			.VAlign((VAlign_Center))
			[
				SNew(STextBlock)
				.ColorAndOpacity(HandleGetTextColor())
			.Text(Message->Message)
			];
	}

private:

	// Callback for getting the task's status text.
	FSlateColor HandleGetTextColor() const
	{
		if ((Message->Verbosity == ELogVerbosity::Error) ||
			(Message->Verbosity == ELogVerbosity::Fatal))
		{
			return FLinearColor::Red;
		}
		else if (Message->Verbosity == ELogVerbosity::Warning)
		{
			return FLinearColor::Yellow;
		}
		else
		{
			return FSlateColor::UseForeground();
		}
	}

	// Holds a pointer to the task that is displayed in this row.
	TSharedPtr<FVaultLogMessage> Message;
};

void SPublisherWindow::Construct(const FArguments& InArgs)
{

	// Store a shared this
	TWeakPtr<SPublisherWindow> WeakPtr = SharedThis(this);

	VaultOutputLog = MakeShareable(new FVaultOutputLog);
	
	// Our Asset Picking Widget
	TSharedRef<SWidget> AssetPickerWidget = SNew(SObjectPropertyEntryBox)
		.ObjectPath(this, &SPublisherWindow::GetCurrentAssetPath)
		.AllowedClass(UObject::StaticClass())
		.OnObjectChanged(this, &SPublisherWindow::OnAssetSelected)
		.AllowClear(false)
		.DisplayUseSelected(true)
		.DisplayBrowse(true)
		.NewAssetFactories(TArray<UFactory*>())
		.IsEnabled(true)
		;

	// Left SideBar

	TSharedRef<SVerticalBox> LeftPanel = SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0.0, 3.f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(0, 0, 3, 0))
			[
				SNew(STextBlock)
				.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.12"))
				.Text_Lambda([this]
				{
					return !CurrentlySelectedAsset.IsValid() ?
						FEditorFontGlyphs::Times_Circle : FEditorFontGlyphs::Check_Circle;
				})
				.ColorAndOpacity_Lambda([this]
				{
					return !CurrentlySelectedAsset.IsValid() ? FLinearColor::Red : FLinearColor::Green;
				})
			]

			+ SHorizontalBox::Slot()
			[
				AssetPickerWidget
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0.0, 3.f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(0, 0, 3, 0))
			[
				SNew(STextBlock)
				.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.12"))
				.Text_Lambda([this]
				{
					return PackageNameInput->GetText().IsEmpty() ?
						FEditorFontGlyphs::Times_Circle : FEditorFontGlyphs::Check_Circle;
				})
				.ColorAndOpacity_Lambda([this]
				{
					return PackageNameInput->GetText().IsEmpty() ? FLinearColor::Red : FLinearColor::Green;
				})
			]

			+SHorizontalBox::Slot()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PackageNameLbl", "Package Name"))
			]
			+SHorizontalBox::Slot()
			[
				SAssignNew(PackageNameInput, SEditableTextBox)
				.HintText(LOCTEXT("PackageNameHintTxt", "Package Name"))
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0.0, 3.f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(0,0,3,0))
			[
				SNew(STextBlock)
				.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.12"))
				.Text_Lambda([this] 
				{ 
					return AuthorInput->GetText().IsEmpty() ? 
						FEditorFontGlyphs::Times_Circle : FEditorFontGlyphs::Check_Circle;
				})
				.ColorAndOpacity_Lambda([this]
				{
					return AuthorInput->GetText().IsEmpty() ? FLinearColor::Red : FLinearColor::Green;
				})
			]
			+ SHorizontalBox::Slot()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("AuthorNameLbl", "Author"))
			]
			+ SHorizontalBox::Slot()
			[
				SAssignNew(AuthorInput, SEditableTextBox)
				.HintText(LOCTEXT("AuthorNameHintTxt", "Author Name"))
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0.0, 3.f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(0, 0, 3, 0))
			[
				SNew(STextBlock)
				.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.12"))
				.Text_Lambda([this]
				{
					return DescriptionInput->GetText().IsEmpty() ?
						FEditorFontGlyphs::Times_Circle : FEditorFontGlyphs::Check_Circle;
				})
				.ColorAndOpacity_Lambda([this]
				{
					return DescriptionInput->GetText().IsEmpty() ? FLinearColor::Red : FLinearColor::Green;
				})
			]
			+ SHorizontalBox::Slot()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("DescriptionNameLbl", "Description"))
			]
			+ SHorizontalBox::Slot()
			[
				SAssignNew(DescriptionInput, SMultiLineEditableTextBox)
				.HintText(LOCTEXT("DescriptionNameHintTxt", "Enter Description"))
			]
		]

		// tags
		+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0, 5, 0, 0))
			[
				SAssignNew(TagsWidget, SPublisherTagsWidget)
			]

		+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Center)
			.Padding(0.f, 16.f, 0.f, 0.f)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.OnClicked(this, &SPublisherWindow::TryPackage)
				.Text(LOCTEXT("SubmitToVaultLabel", "Submit to Vault"))
				.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
				.TextStyle(FEditorStyle::Get(), "NormalText.Important")
				.ContentPadding(FMargin(10.0, 10.0))
				//.IsEnabled(this, &SPublisherWindow::CanPackage) // dont handle it like this until we sort the disabled style
			]

		+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0,20,0,0)
			[
				ConstructOutputLog()
			]
		
		
		;



	// Start UI
	TSharedRef<SVerticalBox> RootWidget = SNew(SVerticalBox);

	RootWidget->AddSlot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.FillHeight(1)
		.Padding(2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			.Padding(FMargin(5.0, 5.0, 10.0, 5.f))
			[
				// Left Side Slot
				LeftPanel
			]
			+SHorizontalBox::Slot()
			.Padding(FMargin(10.0, 5.0, 5.0, 5.f))
			.HAlign(HAlign_Right)
			.AutoWidth()
			[
				// Left Panel
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					ConstructThumbnailWidget().ToSharedRef()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0,5,0,0))
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.Padding(FMargin(0.f, 0.f, 1.f, 0.f))
					[
						// Take Screenshot From World
						SNew(SButton)
						.ContentPadding(FMargin(6.f))
						.ButtonStyle(FEditorStyle::Get(), "FlatButton.Primary")
						.OnClicked(this, &SPublisherWindow::OnCaptureImageFromViewport)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(STextBlock)
								.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.12"))
								.TextStyle(FEditorStyle::Get(), "NormalText.Important")
								.Text(FEditorFontGlyphs::Camera)
							]

							+ SHorizontalBox::Slot()
							.FillWidth(1)
							.Padding(FMargin(5, 0, 0, 0))
							[
								SNew(STextBlock)
								.TextStyle(FEditorStyle::Get(), "NormalText.Important")
								.Text(LOCTEXT("TakeScreenshotLbl", "Capture Viewport"))
							]
						]
					]
					+ SHorizontalBox::Slot()
					.Padding(FMargin(1.f, 0.f, 0.f, 0.f))
					[
						SNew(SButton)
						.ContentPadding(FMargin(6.f))
						.ButtonStyle(FEditorStyle::Get(), "FlatButton.Primary")
						.OnClicked(this, &SPublisherWindow::OnCaptureImageFromFile)
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(STextBlock)
								.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.12"))
								.TextStyle(FEditorStyle::Get(), "NormalText.Important")
								.Text(FEditorFontGlyphs::File_Image_O)

							]
							+ SHorizontalBox::Slot()
							.FillWidth(1)
							.Padding(FMargin(5,0,0,0))
							[
								SNew(STextBlock)
								.TextStyle(FEditorStyle::Get(), "NormalText.Important")
								.Text(LOCTEXT("LoadScreenshotFromFileLbl", "Load From File"))
							]
						]
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0, 5, 0, 0))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(FMargin(0.f, 0.f, 1.f, 0.f))
					[
						SNew(SButton)
						.Text(LOCTEXT("GeneratePythonMapLabel", "Generate Map from Python"))
						.ButtonStyle(FEditorStyle::Get(), "FlatButton.Info")
						.IsEnabled(this, &SPublisherWindow::IsPythonMapGenAvailable)
						.ToolTipText(LOCTEXT("LoadMapFromPythonTooltip", "Not available in this release"))
						.OnClicked(this, &SPublisherWindow::GenerateMapFromPython)
					]
					+ SHorizontalBox::Slot()
					.Padding(FMargin(1.f, 0.f, 0.f, 0.f))
					[
						SNew(SButton)
						.Text(LOCTEXT("LoadPresetMapLabel", "Load Preset Map"))
						.ButtonStyle(FEditorStyle::Get(), "FlatButton.Info")
						.IsEnabled(true)
						.ToolTipText(LOCTEXT("LoadMapFromPresetTooltip", "Load Map from Preset. \n This will load either the Preset map, or if you have a map file to publish, will load this map. \nThis is hard-coded in this release, but future releases will support multiple choices"))
						.OnClicked(this, &SPublisherWindow::GenerateMapFromPreset)
					]
				]
				+ SVerticalBox::Slot()
				.Padding(0,15,0,3)
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("seecondaryAssetBoxHeaderLbl", "Additional Referenced Assets to Package"))
					]
				+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBox)
							.MinDesiredHeight(30.f)
							.MaxDesiredHeight(180.f)
						[
							SAssignNew(SecondaryAssetsBox, SMultiLineEditableTextBox)
							.IsReadOnly(true)
							.AllowMultiLine(true)
							.AlwaysShowScrollbars(false)
							//.BackgroundColor(FLinearColor::Black)
							.Style(FEditorStyle::Get(), "Log.TextBox")
							.Text(this, &SPublisherWindow::GetSecondaryAssetList)
						]
					]
			]
		]; // Close VBox
		

		
	ChildSlot
	[

		RootWidget
	];

}

SPublisherWindow::~SPublisherWindow()
{
	if (ShotTexture)
	{
		DestroyThumbnail();
		//FlushRenderingCommands();
	}
}

void SPublisherWindow::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{

}

TSharedPtr<SWidget> SPublisherWindow::ConstructThumbnailWidget()
{
	// Init the thumb brush. these settings are only for pre-image choice
	ThumbBrush.SetImageSize(FVector2D(256.0));

	return ThumbnailWidget = 
		SNew(SBox)
		//.HAlign(HAlign_Center)
		[
			// Thumbnail Area
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SAssignNew(ThumbnailImage, SImage)
				.Image(&ThumbBrush)
			]

			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SCircularThrobber)
				.Radius(48)
				.NumPieces(10)
				.Period(0.5f)
				.Visibility_Lambda([this]()
				{
					if (ShotTexture->IsValidLowLevel())
					{
						return ShotTexture->IsFullyStreamedIn() ? EVisibility::Hidden : EVisibility::SelfHitTestInvisible;
					}
					return EVisibility::Hidden;
				})
			]
		];
}

FReply SPublisherWindow::OnCaptureImageFromViewport()
{
	ShotTexture = CreateThumbnailFromScene();
	FSlateBrush Brush;

	if (ShotTexture)
	{
		Brush.SetResourceObject(ShotTexture);
		Brush.DrawAs = ESlateBrushDrawType::Image;
		Brush.SetImageSize(FVector2D(VAULT_PUBLISHER_THUMBNAIL_SIZE, VAULT_PUBLISHER_THUMBNAIL_SIZE));
	}
	ThumbBrush = Brush;
	return FReply::Handled();
}

FReply SPublisherWindow::OnCaptureImageFromFile()
{
	ShotTexture = CreateThumbnailFromFile();
	FSlateBrush Brush;

	if (ShotTexture)
	{
		Brush.SetResourceObject(ShotTexture);
		Brush.DrawAs = ESlateBrushDrawType::Image;
		Brush.SetImageSize(FVector2D(VAULT_PUBLISHER_THUMBNAIL_SIZE, VAULT_PUBLISHER_THUMBNAIL_SIZE));
	}
	ThumbBrush = Brush;
	return FReply::Handled();
}

void SPublisherWindow::DestroyThumbnail()
{
	if (ShotTexture)
	{
		if (ShotTexture->Resource)
		{
			ShotTexture->ReleaseResource();
		}
		//ShotTexture->MarkPendingKill();
		//ShotTexture.Reset();
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
	FImageUtils::CropAndScaleImage(SrcWidth, SrcHeight, VAULT_PUBLISHER_CAPTURE_SIZE, VAULT_PUBLISHER_CAPTURE_SIZE, OrigBitmap, ScaledBitmap);

	// Redraw viewport to restore highlight
	GCurrentLevelEditingViewportClient = OldViewportClient;
	Viewport->Draw();

	FScopedTransaction Transaction(LOCTEXT("CapturePackageThumbnail", "Capture Package Image from Viewport"));

	FCreateTexture2DParameters Params;
	Params.bDeferCompression = true;

	UTexture2D* ResizedTexture = FImageUtils::CreateTexture2D(VAULT_PUBLISHER_CAPTURE_SIZE, VAULT_PUBLISHER_CAPTURE_SIZE, ScaledBitmap, GetTransientPackage(), FString(), RF_NoFlags, Params);
	

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
	OutputLogExpandableBox->SetExpanded(true);
	
	if (!CanPackage())
	{
		UE_LOG(LogVault, Error, TEXT("One or more required fields are empty"));
		return FReply::Handled();
	}
	

	// #todo to abstract UI from functionality, much of this code should be reviewed for splitting
	// this will be important when it comes to python hooks
	// image writing stands out as one that certainly needs to be reviewed, so python can pass its own img

	UE_LOG(LogVault, Display, TEXT("Starting Packaging Operation"));


	const FString OutputDirectory = FVaultSettings::Get().GetAssetLibraryRoot();
	const FString ScreenshotPath = OutputDirectory / PackageNameInput->GetText().ToString() + TEXT(".png");
	
	// Pack file path, only used here for duplicate detection
	const FString PackageFileOutput = OutputDirectory / PackageNameInput->GetText().ToString() + TEXT(".upack");

	FImageWriteOptions Params;
	Params.bAsync = true;
	Params.bOverwriteFile = true;
	Params.CompressionQuality = 90;
	Params.Format = EDesiredImageFormat::PNG;

	if (FPaths::FileExists(PackageFileOutput))
	{
		const EAppReturnType::Type Confirmation = FMessageDialog::Open(
			EAppMsgType::OkCancel,
			LOCTEXT("TryPackageOverwriteMsg", "A Vault item already exists with this pack name, are you sure you want to overwrite it?\nThis action cannot be undone."));

		if (Confirmation == EAppReturnType::Cancel)
		{
			UE_LOG(LogVault, Error, TEXT("User cancelled packaging operation due to duplicate pack found"));
			return FReply::Handled();;
		}
	}

	UImageWriteBlueprintLibrary::ExportToDisk(ShotTexture, ScreenshotPath, Params);

	// Store PackageName
	const FString AssetPath = CurrentlySelectedAsset.PackageName.ToString();

	// Our core publish list, which gets written as a text file to be passed to the Pak Tool.
	TSet<FString> PublishList;
	TSet<FString> ObjectsInPackage;

	TSet<FString> AssetsToProcess = { AssetPath };
	AssetsToProcess.Append(GetAssetDependancies(CurrentlySelectedAsset.PackageName));


	// Loop through all Assets, including the root object, and format into the correct absolute system filename for the UnrealPak operation
	for (const FString Path : AssetsToProcess)
	{
		UE_LOG(LogVault, Display, TEXT("Processing Object Path: %s"), *Path);

		FString Filename;
		bool bGotFilename = FPackageName::TryConvertLongPackageNameToFilename(Path, Filename);

		

		// Find the UPackage to determine the asset type
		UPackage* PrimaryAssetPackage = CurrentlySelectedAsset.GetPackage();
		//UPackage* Package = FindPackage(nullptr, *CurrentlySelectedAsset.PackageName.ToString());
		UPackage* ItemPackage = FindPackage(nullptr, *Path);

		if (!PrimaryAssetPackage || !ItemPackage)
		{
			UE_LOG(LogVault, Error, TEXT("Unable to find UPackage for %s"), *CurrentlySelectedAsset.PackageName.ToString());
			continue;
		}

		ObjectsInPackage.Add(ItemPackage->FileName.ToString());

		// Get and append the asset extension
		const FString ExtensionName = ItemPackage->ContainsMap() ? FPackageName::GetMapPackageExtension() : FPackageName::GetAssetPackageExtension();

		// Append extension into filename
		Filename += ExtensionName;
		
		PublishList.Add(Filename);
	}


	// Build a Struct from the metadata to pass to the packager
	FVaultMetadata AssetPublishMetadata;

	AssetPublishMetadata.Author = FName(*AuthorInput->GetText().ToString());
	AssetPublishMetadata.PackName = FName(*PackageNameInput->GetText().ToString());
	AssetPublishMetadata.Description = DescriptionInput->GetText().ToString();
	AssetPublishMetadata.CreationDate = FDateTime::UtcNow();
	AssetPublishMetadata.LastModified = FDateTime::UtcNow();
	AssetPublishMetadata.Tags = TagsWidget->GetUserSelectedTags();
	AssetPublishMetadata.ObjectsInPack = ObjectsInPackage;


	// Lets see if we want to append any new tags to our global tags library
	if (TagsWidget->GetShouldAddNewTagsToLibrary())
	{
		FVaultSettings::Get().SaveVaultTags(TagsWidget->GetUserSelectedTags());
	}


	UAssetPublisher::PackageSelected(PublishList, AssetPublishMetadata);
	return FReply::Handled();
}

//void SPublisherWindow::OnPrimaryAssetListChanged()
//{
//	AddSelectedToList();
//}
//
//FReply SPublisherWindow::AddSelectedToList()
//{
//	// Check if we have something selected:
//	if (!GetDefault<UAssetPublisher>()->PrimaryAsset->IsValidLowLevel())
//	{
//		return FReply::Handled();
//	}
//
//	FText PriorText = PrimaryAssetsBox->GetText();
//	FString NewText = PriorText.ToString();
//
//	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
//	TArray<FAssetData> SelectedAssets;
//	ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);
//	TArray<FString> Result;
//	TArray<FString> SecondaryResult;
//	for (FAssetData& AssetData : SelectedAssets) {
//		Result.Add(AssetData.PackageName.ToString());
//
//		NewText += "," + AssetData.PackageName.ToString();
//
//		SecondaryResult.Append(GetAssetDependancies((AssetData.PackageName)));
//	}
//	
//	PrimaryAssetsBox->SetText(FText::FromName(GetDefault<UAssetPublisher>()->PrimaryAsset->GetFName()));
//	
//	PrimaryAssetsBox->Refresh();
//
//	FString SecondaryAssetsBoxText;
//	
//	for (FString Secondary: SecondaryResult)
//	{
//		SecondaryAssetsBoxText += Secondary;
//	}
//
//	SecondaryAssetsBox->SetText(FText::FromString(SecondaryAssetsBoxText));
//	return FReply::Handled();
//}

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

FText SPublisherWindow::GetSecondaryAssetList() const
{
	//TSoftObjectPtr<UObject> Asset = GetMutableDefault<UAssetPublisher>()->PrimaryAsset;

	if (CurrentlySelectedAsset.IsValid())
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
		//FAssetData AssetPublisherData;
		//UAssetManager::GetIfValid()->GetAssetDataForPath(Asset.ToSoftObjectPath(), AssetPublisherData);

		TArray<FString> Dependancies = GetAssetDependancies(CurrentlySelectedAsset.PackageName);
		Dependancies.Remove(CurrentlySelectedAsset.PackageName.ToString());

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

bool SPublisherWindow::CanPackage() const
{
	// #todo Currently only check pack name, should check more like screenshot and other details. Enforce some rules!
	
	if (PackageNameInput->GetText().IsEmpty() ||
		AuthorInput->GetText().IsEmpty() ||
		DescriptionInput->GetText().IsEmpty() ||
		TagsWidget->GetUserSelectedTags().Num() == 0 ||
		CurrentlySelectedAsset.IsValid() == false
		)
	{
		return false;
	}
	return true;
}

TSharedRef<SWidget> SPublisherWindow::ConstructOutputLog()
{
	VaultOutputLog->OnVaultMessageReceived.BindRaw(this, &SPublisherWindow::RefreshOutputLogList);

	return SAssignNew(OutputLogExpandableBox, SExpandableArea)
	.InitiallyCollapsed(true)
	.MaxHeight(200.f)
	.HeaderContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("OutputLogHeaderLbl", "Output Log"))
	]
	.BodyContent()
	[
		SNew(SScrollBox)
		.Orientation(Orient_Vertical)
		+ SScrollBox::Slot()
		[
			SNew(SBox)
			.MinDesiredHeight(FOptionalSize(200.f))
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("Menu.Background"))

				[
					SAssignNew(VaultOutputLogList, SListView<TSharedPtr< FVaultLogMessage>>)
					.ListItemsSource(&VaultOutputLog->MessageList)
					.OnGenerateRow(this, &SPublisherWindow::HandleVaultLogGenerateRow)
					.ItemHeight(24)
					.SelectionMode(ESelectionMode::Multi)
					.HeaderRow(SNew(SHeaderRow)
					.Visibility(EVisibility::Collapsed)
					+ SHeaderRow::Column("Log")
					.DefaultLabel(LOCTEXT("VaultLogHeaderLbl", "Output Log")))
				]
			]
		]
	];

}

TSharedRef<ITableRow> SPublisherWindow::HandleVaultLogGenerateRow(TSharedPtr<FVaultLogMessage> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SVaultLogMessageListRow, OwnerTable)
		.Message(InItem);
}

void SPublisherWindow::RefreshOutputLogList()
{
	if (VaultOutputLogList)
	{
		VaultOutputLogList->RebuildList();
	}
}

FString SPublisherWindow::GetCurrentAssetPath() const
{
	return CurrentlySelectedAsset.IsValid() ? CurrentlySelectedAsset.ObjectPath.ToString() : FString("");
}

void SPublisherWindow::OnAssetSelected(const FAssetData& InAssetData)
{
	// #todo lambda this for simpler code?
	CurrentlySelectedAsset = InAssetData;
}

bool SPublisherWindow::IsPythonMapGenAvailable() const
{
	return false;
}

FReply SPublisherWindow::GenerateMapFromPython()
{
	return FReply::Handled();
}

FReply SPublisherWindow::GenerateMapFromPreset()
{
	/* 
		#todo. Hard coded Path to content, In future want to make this flexible, copying the map from a central location.
		We also want to unify the python and pre-made maps options, 
		as the end user doesnt really need to care the source, just the result

	*/

	// Bug fix to stop crashes when a image was taken prior to choosing a map change.
	//ShotTexture->ReleaseResource();

	const FString ContentPath = IPluginManager::Get().FindPlugin(TEXT("Vault"))->GetContentDir();
	const FString MapPath = ContentPath / "PresetMap.umap";

	if (CurrentlySelectedAsset.IsValid())
	{
		if (CurrentlySelectedAsset.GetPackage()->ContainsMap())
		{
			FEditorFileUtils::LoadMap(CurrentlySelectedAsset.GetPackage()->FileName.ToString(), true, true);
			return FReply::Handled();
			
		}

		FEditorFileUtils::LoadMap(MapPath, true, true);
		// #future We could spawn the selected asset if possible. 
		// but its too early in the LoadMap to do it here, we need to do some bindings for level loads
		// and handle that when appropiate.
		return FReply::Handled();

	}

	FEditorFileUtils::LoadMap(MapPath, true, true);
	return FReply::Handled();
}


#undef LOCTEXT_NAMESPACE
#undef VAULT_PUBLISHER_THUMBNAIL_SIZE

