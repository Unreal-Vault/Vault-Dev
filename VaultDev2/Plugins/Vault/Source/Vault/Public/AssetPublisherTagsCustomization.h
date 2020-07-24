// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SlateFwd.h"
#include "Misc/TextFilterExpressionEvaluator.h"
#include "VaultTypes.h"

//class SEditableTextBox;
class SPublisherWindow;
class SMultiLineEditableTextBox;

class VAULT_API SPublisherTagsWidget : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SPublisherTagsWidget) {}
	SLATE_END_ARGS()

	// Construct Widget
	void Construct(const FArguments& InArgs);

	// Our final getter, passing the data into the publishing script.
	TSet<FString> GetUserSelectedTags();
	
	// Our Entry box that acts as the final list of tags, either from user entry or list selection
	TSharedPtr<SMultiLineEditableTextBox> TagsCustomBox;
private:

	TWeakPtr<SPublisherWindow> PublisherWindow;
	
	TSharedPtr<SSearchBox> TagSearchBox;
	
	/* The actual UI list */
	TSharedPtr< SListView< TSharedPtr<FString> > > KeysList;

	void OnTagSearchTextChanged(const FText& InFilterText);
	void OnTagSearchTextCommitted(const FText& InFilterText, ETextCommit::Type CommitInfo);


	// Initial clean gather of tags List. Caches tags for future use. 
	void CacheBaseTagsPool();

	// Contains the Tags List gathered from the JSON file. This only runs once. 
	TSet<FString> TagsListCache;

	/* The live list of strings to use for lists */
	TArray<TSharedPtr<FString>> TagsListItems;

	// Refresh List based on Search updates.
	void RefreshTagPool();


	TSharedPtr<FTextFilterExpressionEvaluator> TagTextFilterPtr;

	void AddTagFromPool(TSharedPtr<FString> InTag);

	//void UpdateUserTagsMetadata(const FText& InText, ETextCommit::Type CommitMethod);

	// tag list view - on row
	TSharedRef<ITableRow> MakeTagRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable);



	TSharedPtr<IPropertyHandle> TagClassVar;

};
