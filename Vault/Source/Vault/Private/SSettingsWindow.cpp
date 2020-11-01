// Fill out your copyright notice in the Description page of Project Settings.


#include "SSettingsWindow.h"
#include "VaultStyle.h"
#include "VaultSettings.h"

#define LOCTEXT_NAMESPACE "NSVaultSettingsWindow"

void SSettingsWindow::Construct(const FArguments& InArgs)
{

	FString Readout1 = "Settings Page - Coming Soon \n\nAdjust your setting in the Local & Global JSON files:";
	
	FString Readout2 = "Local (Per-User) Settings File - " + FVaultSettings::Get().LocalSettingsFilePathFull;
	FString Readout3 = "Default Global (Per-Team) Settings File - " + FVaultSettings::Get().DefaultGlobalsPath;

	FString ReadoutFull = Readout1 + LINE_TERMINATOR+ LINE_TERMINATOR + Readout2 + LINE_TERMINATOR+ LINE_TERMINATOR + Readout3;

	ChildSlot
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.FillHeight(1)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.FillWidth(1)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(ReadoutFull))
					.TextStyle(FVaultStyle::Get(), "MetaTitleText")
					.Justification(ETextJustify::Center)
				]
			]
		];
}


#undef LOCTEXT_NAMESPACE

