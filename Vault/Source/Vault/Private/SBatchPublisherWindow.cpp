// Fill out your copyright notice in the Description page of Project Settings.


#include "SBatchPublisherWindow.h"
#include "VaultStyle.h"

#define LOCTEXT_NAMESPACE "NSBatchPublisher"

void SBatchPublisherWindow::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("VaultSettingsComingSoonLbl", "Batch Publisher \nComing Soon"))
				.TextStyle(FVaultStyle::Get(), "MetaTitleText")
				.Justification(ETextJustify::Center)
			]
		]
	];
}

#undef LOCTEXT_NAMESPACE