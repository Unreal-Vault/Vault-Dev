// Fill out your copyright notice in the Description page of Project Settings.

#include "AssetPublisherTagsCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "DetailWidgetRow.h"
#include "AssetPublisher.h"
#include "SPublisherWindow.h"

#include "Slate.h"
#include "SlateExtras.h"
#include "VaultSettings.h"

static const FName TagsListColumnName(TEXT("Tags"));
#define LOCTEXT_NAMESPACE "FVaultPublisherTagsCustomization"

void FAssetPublisherTagsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TagTextFilterPtr = MakeShareable(new FTextFilterExpressionEvaluator(ETextFilterExpressionEvaluatorMode::BasicString));
	
	// Refresh available tags into our array from the json file
	RefreshTagPool();

	// Force Vault Category to show first by calling its edit.
	DetailBuilder.EditCategory(TEXT("Vault"));

	// Replace existing variable display with a new whole row customization
	DetailBuilder.EditCategory(TEXT("Tags"))
		.InitiallyCollapsed(true)
		.AddCustomRow(FText::FromString(TEXT("TagsPool"))).WholeRowContent()
	[

		// Tags Slot Root
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Top)
		.Padding(2.0f)
		[
			SNew(SBox)
			.MinDesiredHeight(30.f)
		    .MaxDesiredHeight(200.f)
			[
				SNew(SHorizontalBox)

				// Tags Left - User Entry				
				+ SHorizontalBox::Slot()
				.Padding(2.f, 2.f)
				.FillWidth(1.0f)
				.HAlign(HAlign_Fill)
				[
					SAssignNew(TagsCustomBox, SMultiLineEditableTextBox)
					.HintText(LOCTEXT("TagsUserEntry", "Comma Separated Tags"))
					.AutoWrapText(true)
					.IsReadOnly(false)
					.OnTextCommitted(this, &FAssetPublisherTagsCustomization::UpdateUserTagsMetadata)
				]


				// Tags Right - Tag Pool
				+ SHorizontalBox::Slot()
				.Padding(2.f, 2.f)
				.FillWidth(1.0f)
				.HAlign(HAlign_Fill)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding(2.f, 2.f)
					.AutoHeight()
					[
						SAssignNew(TagSearchBox, SSearchBox)
						.OnTextChanged(this, &FAssetPublisherTagsCustomization::OnTagSearchTextChanged)
						.OnTextCommitted(this, &FAssetPublisherTagsCustomization::OnTagSearchTextCommitted)
					]

					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						SAssignNew(KeysList, SListView<TSharedPtr<FString>>)
						.SelectionMode(ESelectionMode::Single)
						.ListItemsSource(&Items)
						.OnGenerateRow(this, &FAssetPublisherTagsCustomization::MakeTagRow)
						.ItemHeight(12.f)
						.OnMouseButtonClick(this, &FAssetPublisherTagsCustomization::AddTagFromPool)
						.HeaderRow
						(
							SNew(SHeaderRow)
							.Visibility(EVisibility::Collapsed)
							+ SHeaderRow::Column(TagsListColumnName)
							.DefaultLabel(LOCTEXT("headerLabel", "Tag List"))
						)
					]
				]
		]

		] // End tag section





	];

	DetailBuilder.EditCategory(TEXT("Package List"));

	DetailBuilder.EditCategory(TEXT("Output Log"))
		.AddCustomRow(FText::FromString(TEXT("TagsPool"))).WholeRowContent()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			[
				SNew(SMultiLineEditableTextBox)
				.IsReadOnly(true)
			]
		];
}

void FAssetPublisherTagsCustomization::OnTagSearchTextChanged(const FText& InFilterText)
{
	TagTextFilterPtr->SetFilterText(InFilterText);
	TagSearchBox->SetError(TagTextFilterPtr->GetFilterErrorText());

	RefreshTagPool();
}

void FAssetPublisherTagsCustomization::OnTagSearchTextCommitted(const FText& InFilterText, ETextCommit::Type CommitInfo)
{
}

void FAssetPublisherTagsCustomization::AddTagFromPool(TSharedPtr<FString> InTag)
{
	// Store the entire string list that's already there so we can append to it.
	const FString ExistingString = TagsCustomBox->GetText().ToString();

	if (ExistingString.Contains(*InTag.Get()))
	{
		// error, tag already contained. 
		// #todo add some sort of error message, like the lil red bar with "already included"
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
	UpdateUserTagsMetadata(TagsCustomBox->GetText(), ETextCommit::Default);
}

void FAssetPublisherTagsCustomization::UpdateUserTagsMetadata(const FText& InText, ETextCommit::Type CommitMethod)
{
	// #todo update tag list internals
	UE_LOG(LogTemp, Warning, TEXT("tags changed!!"));

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
		return false;
	}

private:
	const FString* StrPtr;
};

// Construct Tag Row
TSharedRef<ITableRow> FAssetPublisherTagsCustomization::MakeTagRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable)
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

void FAssetPublisherTagsCustomization::RefreshTagPool()
{

	//#todo this might run every tag, so clearing and updating the array for every single tag. i think this is wrong and should happen at construct start?
	// Get existing tags and stored as shared ptrs
	Items.Empty();

	// Get tags
	TSet<FString> ExistingTagsString;
	FVaultSettings::Get().ReadVaultTags(ExistingTagsString);

	for (auto TagString : ExistingTagsString)
	{
		const FText FilterText = TagTextFilterPtr->GetFilterText();

		if (FilterText.IsEmpty())
		{
			Items.Add(MakeShareable(new FString(TagString)));
		}
		else
		{
			if (TagTextFilterPtr->TestTextFilter(FTagFilterContext(TagString)))
			{
				Items.Add(MakeShareable(new FString(TagString)));
			}
		}

	}

	//KeysList->RequestListRefresh;
}






#undef LOCTEXT_NAMESPACE