// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VaultTypes.h"
#include "Input/Reply.h"
#include "Slate.h"
#include "SlateExtras.h"


/**
 * 
 */
class VAULT_API OperationChoiceWindow
{
public:
	OperationChoiceWindow();
	~OperationChoiceWindow();
};


class SOperationsChoice : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SOperationsChoice)
	{}
	SLATE_END_ARGS()

	SOperationsChoice();

	void Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab, const TSharedPtr<SWindow>& ConstructUnderWindow);

private:

	TSharedPtr<FTabManager> TabManager;

	/** Callback for spawning tabs. */
	TSharedRef<SDockTab> HandleTabManagerSpawnTab(const FSpawnTabArgs& Args, FName TabIdentifier) const;



protected:

	/**
 * Fills the Window menu with menu items.
 *
 * @param MenuBuilder The multi-box builder that should be filled with content for this pull-down menu.
 * @param TabManager A Tab Manager from which to populate tab spawner menu items.
 */
	static void FillWindowMenu(FMenuBuilder& MenuBuilder, const TSharedPtr<FTabManager> TabManager);
};
