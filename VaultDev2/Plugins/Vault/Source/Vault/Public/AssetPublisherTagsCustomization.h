// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SlateFwd.h"
#include "Misc/TextFilterExpressionEvaluator.h"
#include "VaultTypes.h"

class FUICommandInfo;
class SWidget;
class SEditableTextBox;
class SPublisherWindow;
class SMultiLineEditableTextBox;

class VAULT_API SPublisherTagsWidget : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SPublisherTagsWidget) {}
	SLATE_END_ARGS()

	// Construct Widget
	void Construct(const FArguments& InArgs);
	
	TSharedPtr<SMultiLineEditableTextBox> TagsCustomBox;
private:

	TSharedRef<SWidget> CreateTagEntryBox();
	TSharedRef<SWidget> CreateTagList();

	TSharedPtr<SEditableTextBox> TagUserEntryTextBox;

	TWeakPtr<SPublisherWindow> PublisherWindow;

	
	TSharedPtr<SSearchBox> TagSearchBox;
	
	/* The actual UI list */
	TSharedPtr< SListView< TSharedPtr<FString> > > KeysList;


	void OnTagSearchTextChanged(const FText& InFilterText);
	void OnTagSearchTextCommitted(const FText& InFilterText, ETextCommit::Type CommitInfo);

	// Tag Search Bits
	void RefreshTagPool();

	TSharedPtr<FTextFilterExpressionEvaluator> TagTextFilterPtr;

	void AddTagFromPool(TSharedPtr<FString> InTag);

	void UpdateUserTagsMetadata(const FText& InText, ETextCommit::Type CommitMethod);

	// tag list view - on row
	TSharedRef<ITableRow> MakeTagRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/* The list of strings */
	TArray<TSharedPtr<FString>> Items;

	TSharedPtr<IPropertyHandle> TagClassVar;




};
