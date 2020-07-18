// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VaultTypes.h"
#include "Input/Reply.h"
#include "SlateFwd.h"

class SVaultRootPanel : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SVaultRootPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab, const TSharedPtr<SWindow>& ConstructUnderWindow);

private:

	TSharedPtr<FTabManager> TabManager;

	/** Callback for spawning tabs. */
	TSharedRef<SDockTab> HandleTabManagerSpawnTab(const FSpawnTabArgs& Args, FName TabIdentifier) const;

protected:

	static void FillWindowMenu(FMenuBuilder& MenuBuilder, const TSharedPtr<FTabManager> TabManager);
};
