// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Types/SlateEnums.h"
#include "Slate.h"
#include "SlateExtras.h"
#include "IDetailsView.h"
#include "Misc/TextFilterExpressionEvaluator.h"
#include "VaultTypes.h"

class IDetailLayoutBuilder;
class FUICommandInfo;
class SWidget;
class SEditableTextBox;
class SPublisherWindow;
class IPropertyHandle;

class FAssetPublisherTagsCustomization : public IDetailCustomization
{
public:

	static TSharedRef<IDetailCustomization> MakeInstance(TWeakPtr<SPublisherWindow> InSequenceRecorder)
	{
		return MakeShareable(new FAssetPublisherTagsCustomization(InSequenceRecorder));
	}


	FAssetPublisherTagsCustomization(TWeakPtr<SPublisherWindow> InPublisherWindow)
	{
		PublisherWindow = InPublisherWindow;
	}


	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

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
