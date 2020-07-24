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
			AssetPickerWidget
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
			.Padding(FMargin(0.0, 3.f))
		[
			SNew(SHorizontalBox)
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
				SNew(SPublisherTagsWidget)
			]
		
		;

	// Command List
	//CommandList = MakeShareable(new FUICommandList);

	// Get Property Module
	//FPropertyEditorModule& PropertyEditorModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	// Store a shared this
	TWeakPtr<SPublisherWindow> WeakPtr = SharedThis(this);

	// Setup the look
	//FDetailsViewArgs DetailsViewArgs;
	//DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	//DetailsViewArgs.bAllowSearch = false;

	// Create the Details view widget
	//AssetPublisherDetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	//AssetPublisherTagsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	//AssetPublisherDetailsView->RegisterInstancedCustomPropertyLayout(UAssetPublisher::StaticClass(), FOnGetDetailCustomizationInstance::CreateStatic(&FAssetPublisherTagsCustomization::MakeInstance, WeakPtr));
	//AssetPublisherDetailsView->SetObject(GetMutableDefault<UAssetPublisher>());

	// #todo dump this and make the function return the SWidget directly.
	//ConstructThumbnailWidget();

	VaultOutputLog = MakeShareable(new FVaultOutputLog);

	// Start UI
	TSharedRef<SVerticalBox> RootWidget = SNew(SVerticalBox);

	RootWidget->AddSlot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.AutoHeight()
		.Padding(5.f, 5.f, 5.f, 5.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			.Padding(FMargin(5.0, 5.0, 20.0, 5.f))
			[
				// Left Side Slot
				LeftPanel
			]
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.AutoWidth()
			[
				// Left Panel
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.FillHeight(1)
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
						//.OnClicked(this, &SPublisherWindow::StartScreenshotCapture)
					]
					+ SHorizontalBox::Slot()
					.Padding(FMargin(1.f, 0.f, 0.f, 0.f))
					[
						SNew(SButton)
						.Text(LOCTEXT("LoadPresetMapLabel", "Load Map from Preset Map"))
						.ButtonStyle(FEditorStyle::Get(), "FlatButton.Info")
						//.OnClicked(this, &SPublisherWindow::StartScreenshotCapture)
					]
				]



			]
		]; // Close VBox
		
	//RootWidget->AddSlot()
	//.HAlign(HAlign_Fill)
	//.VAlign(VAlign_Fill)
	//.AutoHeight()
	//.Padding(0.0)
	//[
	//	//AssetPublisherDetailsView.ToSharedRef()
	//	
	//];

	//RootWidget->AddSlot()
	//.HAlign(HAlign_Fill)
	//.VAlign(VAlign_Fill)
	//.AutoHeight()
	//.Padding(0.0)
	//[
	//	AssetPublisherTagsView.ToSharedRef()
	//];
	
	RootWidget->AddSlot()
	.AutoHeight()
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Center)
	.Padding(0.0)
	[
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
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
		] // close border
	]; //close slot

	RootWidget->AddSlot()
	.AutoHeight()
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Center)
	.Padding(5.f, 5.f, 5.f, 5.f)
	[
		SNew(SButton)
		.HAlign(HAlign_Center)
		.OnClicked(this, &SPublisherWindow::TryPackage)
		.Text(LOCTEXT("SubmitToVaultLabel", "Submit to Vault"))
		.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
		.TextStyle(FEditorStyle::Get(), "NormalText.Important")
		.ContentPadding(FMargin(10.0,10.0))
		.IsEnabled(this, &SPublisherWindow::CanPackage)
	];

	RootWidget->AddSlot()
	[
		ConstructOutputLog()
	];
		
	ChildSlot
	[
		SNew(SScrollBox)
		.Orientation(Orient_Vertical)
		+SScrollBox::Slot()
		[
			RootWidget
		]
	];

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
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
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
					if (ShotTexture)
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
	UE_LOG(LogVault, Display, TEXT("Starting Packaging Operation"));

	// Get Default Asset - May be expanded to array
	TSoftObjectPtr<UObject> Asset = GetMutableDefault<UAssetPublisher>()->PrimaryAsset;

	if (!Asset->IsValidLowLevel())
	{
		UE_LOG(LogVault, Error, TEXT("No Asset Selected"));
		return FReply::Handled();
	}

	const FString OutputDirectory = FVaultSettings::Get().GetAssetLibraryRoot();
	const FString ScreenshotPath = OutputDirectory / GetMutableDefault<UAssetPublisher>()->PackName.ToString() + TEXT(".png");

	//FImageUtils::ExportRenderTarget2DAsPNG()
	FImageWriteOptions Params;
	Params.bAsync = true;
	Params.bOverwriteFile = true;
	Params.CompressionQuality = 90;
	Params.Format = EDesiredImageFormat::PNG;

	UImageWriteBlueprintLibrary::ExportToDisk(ShotTexture, ScreenshotPath, Params);


	
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
	TSoftObjectPtr<UObject> Asset = GetMutableDefault<UAssetPublisher>()->PrimaryAsset;

	if (Asset)
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
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

bool SPublisherWindow::CanPackage() const
{
	// #todo Currently only check pack name, should check more like screenshot and other details. Enforce some rules!
	return true;
}

TSharedRef<SWidget> SPublisherWindow::ConstructOutputLog()
{
	VaultOutputLog->OnVaultMessageReceived.BindRaw(this, &SPublisherWindow::RefreshOutputLogList);

	return SNew(SExpandableArea)
	.InitiallyCollapsed(false)
	.MaxHeight(300.f)
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
	CurrentlySelectedAsset = InAssetData;
	// #todo something
}

//FText SPublisherWindow::GetPrimaryAssetList() const
//{
//	TSoftObjectPtr<UObject> Asset = GetMutableDefault<UAssetPublisher>()->PrimaryAsset;
//
//	if (Asset)
//	{
//		return FText::FromString(Asset->GetName());
//	}
//	return FText::GetEmpty();
//}

void SPublisherWindow::UpdateLastModifiedMetadata(const FText& InText, ETextCommit::Type CommitMethod)
{
	AssetPublishMetadata.LastModified = FDateTime::UtcNow();
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
#undef VAULT_PUBLISHER_THUMBNAIL_SIZE

