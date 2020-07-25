// Fill out your copyright notice in the Description page of Project Settings.

#include "AssetPublisherTagsCustomization.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "DetailWidgetRow.h"
#include "AssetPublisher.h"
#include "SPublisherWindow.h"
#include "EditorFontGlyphs.h"
#include "Slate.h"
#include "SlateExtras.h"
#include "VaultSettings.h"

#define LOCTEXT_NAMESPACE "FVaultPublisherTagsCustomization"

static const FName TagsListColumnName(TEXT("Tags"));

void SPublisherTagsWidget::Construct(const FArguments& InArgs)
{
	TagTextFilterPtr = MakeShareable(new FTextFilterExpressionEvaluator(ETextFilterExpressionEvaluatorMode::BasicString));
	
	// Refresh available tags into our array from the json file
	CacheBaseTagsPool();

	TSharedRef<SBox> TagsWidget = SNew(SBox)
	[
		// Tags Slot Root
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Top)
		.Padding(0.0f)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(0, 0, 3, 0))
			[
				SNew(STextBlock)
				.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.12"))
				.Text_Lambda([this]
				{
					return TagsCustomBox->GetText().IsEmpty() ?
						FEditorFontGlyphs::Times_Circle : FEditorFontGlyphs::Check_Circle;
				})
				.ColorAndOpacity_Lambda([this]
				{
					return TagsCustomBox->GetText().IsEmpty() ? FLinearColor::Red : FLinearColor::Green;
				})
			]

			+ SHorizontalBox::Slot()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("TagsHeaderTitleLbl", "Tags"))
			]
		]
		
		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Top)
		.Padding(0, 7, 0, 0)
		[
			SNew(SBox)
			.MinDesiredHeight(160.f)
		    .MaxDesiredHeight(160.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot() // Tags Right - Tag Pool
				.Padding(0.f, 0.f)
				.FillWidth(1.0f)
				.HAlign(HAlign_Fill)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding(2.f, 2.f)
					.AutoHeight()
					[
						SAssignNew(TagSearchBox, SSearchBox)
						.OnTextChanged(this, &SPublisherTagsWidget::OnTagSearchTextChanged)
						.OnTextCommitted(this, &SPublisherTagsWidget::OnTagSearchTextCommitted)
						.HintText(LOCTEXT("TagSearchHintTextLbl", "Search Tags"))
					]

					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						SAssignNew(KeysList, SListView<TSharedPtr<FString>>)
						.SelectionMode(ESelectionMode::Single)
						.ListItemsSource(&TagsListItems)
						.OnGenerateRow(this, &SPublisherTagsWidget::MakeTagRow)
						.ItemHeight(12.f)
						.OnMouseButtonClick(this, &SPublisherTagsWidget::AddTagFromPool)
						.HeaderRow
						(
							SNew(SHeaderRow)
							.Visibility(EVisibility::Collapsed)
							+ SHeaderRow::Column(TagsListColumnName)
							.DefaultLabel(LOCTEXT("headerLabel", "Tag List"))
						)
					]
				] // End H-Box slot

				+ SHorizontalBox::Slot() // User Entry Box Left
					.Padding(FMargin(9, 0,0,0))
					.FillWidth(1.0f)
					.HAlign(HAlign_Fill)
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SBox)
							.HeightOverride(140.0)
							[
								SAssignNew(TagsCustomBox, SMultiLineEditableTextBox)
								.HintText(LOCTEXT("TagsUserEntry", "Comma Separated Tags"))
								.AutoWrapText(true)
								.IsReadOnly(false)
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0,5,0,0)
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
							.AutoWidth()
							[
								SAssignNew(ShouldAddNewTagsToGlobalTagsCheckBox, SCheckBox)
							]
							+SHorizontalBox::Slot()
							[
								SNew(STextBlock)
								.Text(LOCTEXT("AddTagsToPoolLbl", "Add new tags to Global Tags List?"))
							]
						]
					]
			] // end sbox
		] // End tag section
	];

	ChildSlot
		[
			TagsWidget
		];
}


TSet<FString> SPublisherTagsWidget::GetUserSelectedTags()
{
	const FString TagString = TagsCustomBox->GetText().ToString();
	TArray<FString> TagArrayParsed;
	TagString.ParseIntoArray(TagArrayParsed, TEXT(","));
	TSet<FString> TagSet = TSet<FString>(TagArrayParsed);
	return TagSet;
}

bool SPublisherTagsWidget::GetShouldAddNewTagsToLibrary()
{
	return ShouldAddNewTagsToGlobalTagsCheckBox->IsChecked();
}

void SPublisherTagsWidget::OnTagSearchTextChanged(const FText& InFilterText)
{
	TagTextFilterPtr->SetFilterText(InFilterText);
	TagSearchBox->SetError(TagTextFilterPtr->GetFilterErrorText());
	RefreshTagPool();
}

void SPublisherTagsWidget::OnTagSearchTextCommitted(const FText& InFilterText, ETextCommit::Type CommitInfo)
{

}

void SPublisherTagsWidget::CacheBaseTagsPool()
{
	// Cache our JSON Tags
	FVaultSettings::Get().ReadVaultTags(TagsListCache);

	// Create a clean list of Live Tags for the untouched view
	// We don't want to use the RefreshTagPool as it has ptr's that are still null
	for (auto CacheTag : TagsListCache)
	{
		TagsListItems.Add(MakeShareable(new FString(CacheTag)));
	}
}

void SPublisherTagsWidget::AddTagFromPool(TSharedPtr<FString> InTag)
{
	// Store the entire string list that's already there so we can append to it.
	const FString ExistingString = TagsCustomBox->GetText().ToString();

	// Avoid Duplicate Tags
	if (ExistingString.Contains(*InTag.Get()))
	{
		return;
	}

	// UpdatedString will contain the old + the new
	FString UpdatedString;

	// If user added comma correctly, the system did, or its just empty, go ahead and add.
	if (ExistingString.EndsWith(",") || ExistingString.IsEmpty())
	{
		UpdatedString = ExistingString + *InTag.Get();
	}

	// If the comma is missing, add in the comma, then add as normal
	else
	{
		UpdatedString = ExistingString + "," + *InTag.Get();
	}

	// Update Tag box with new string info
	TagsCustomBox->SetText(FText::FromString(UpdatedString));


}

// Tag Filtering Context Search
class FTagFilterContext : public ITextFilterExpressionContext
{
public:
	explicit FTagFilterContext(const FString& InString)
		: StrPtr(&InString)
	{
	}

	virtual bool TestBasicStringExpression(const FTextFilterString& InValue, const ETextFilterTextComparisonMode InTextComparisonMode) const override
	{
		return TextFilterUtils::TestBasicStringExpression(*StrPtr, InValue, InTextComparisonMode);
	}

	virtual bool TestComplexExpression(const FName& InKey, const FTextFilterString& InValue, const ETextFilterComparisonOperation InComparisonOperation, const ETextFilterTextComparisonMode InTextComparisonMode) const override
	{
		// Not Required or implemented.
		return false;
	}

private:
	const FString* StrPtr;
};

// Construct Tag Row
TSharedRef<ITableRow> SPublisherTagsWidget::MakeTagRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FString> >, OwnerTable)
		[
			SNew(SBorder)
			[
				SNew(STextBlock)
				.Text(FText::FromString(*Item.Get()))
			]
		];
}

void SPublisherTagsWidget::RefreshTagPool()
{
	// Clear Existing List of Live Tags.
	TagsListItems.Empty();

	for (auto TagToTest : TagsListCache)
	{
		if (TagSearchBox.IsValid() == false || TagSearchBox->GetText().IsEmpty())
		{
			TagsListItems.Add(MakeShareable(new FString(TagToTest)));
		}
		else
		{
			if (TagTextFilterPtr->TestTextFilter(FTagFilterContext(TagToTest)))
			{
				TagsListItems.Add(MakeShareable(new FString(TagToTest)));
			}
		}
	}
	KeysList->RebuildList();
}

#undef LOCTEXT_NAMESPACE