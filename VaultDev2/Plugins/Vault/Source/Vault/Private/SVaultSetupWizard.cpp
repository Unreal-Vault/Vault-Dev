// Fill out your copyright notice in the Description page of Project Settings.


#include "SVaultSetupWizard.h"
#include "VaultStyle.h"

#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "EditorStyleSet.h"

#include "Editor/EditorEngine.h"

#define LOCTEXT_NAMESPACE "VaultSetupWizard"

SVaultSetupWizard::SVaultSetupWizard()
{
}

SVaultSetupWizard::~SVaultSetupWizard()
{
}




void SVaultSetupWizard::Construct(const FArguments& Args)
{
	SWindow::Construct(SWindow::FArguments()
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.IsPopupWindow(false)
		.SizingRule(ESizingRule::Autosized)
		.SupportsTransparency(EWindowTransparency::None)
		.InitialOpacity(1.0f)
		.FocusWhenFirstShown(true)
		.bDragAnywhere(false)
		.ActivationPolicy(EWindowActivationPolicy::Always)
		.ScreenPosition(FVector2D((float)(GEditor->GetActiveViewport()->GetSizeXY().X) / 2.0, (float)(GEditor->GetActiveViewport()->GetSizeXY().Y) / 2.0))
		[
			SNew(SBorder)
			.BorderImage(FVaultStyle::Get().GetBrush("RichText.Background"))
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.Padding(20)
				.FillHeight(1)
				[
					SNew(SRichTextBlock)
					.WrapTextAt(900)
					.DecoratorStyleSet(&FVaultStyle::Get())
					.TextStyle(FVaultStyle::Get(), "RichText.Text")
					.Text(LOCTEXT("VaultSetupWizardMsg", 
						"<RichText.Header> Vault Setup Wizard </>\n\n" \
						"Welcome to the Vault Plugin. Vault has detected that some or all user settings were missing, so I assume this is your first run.\n\n" \
						
						"Vault has automatically created your Global Settings, Local Settings, and an empty Tag Library for you to setup as you wish.\n\n" \
						
						"Your Local Settings can be found in the Vault plugin root directory. This file informs the Vault where to find your global settings and tags, and other temporary files. " \
						"The Local Settings is your per-user settings. Once you have set it up once, you can copy this into any other Vault Projects you have.\n\n" \
						
						"Your Global Settings file teaches Vault where to find the asset library. This file is designed to be team-wide, and shared across all users of Vault. " \
						"This file is sparse right now, but new features will be added as Vault develops.\n\n" \

						"The VaultTags file is a shared list of Tags to be used, so you can avoid those close-duplicates like Chars and Characters. Like global settings, this should be stored in a central place. During publishing, new tags can be added automatically.\n\n" \
						
						"If you have setup Vault before, or working as part of a team who already setup Vault, you just need to update your VaultLocalSettings file to point to the correct directories\n\n" \
						
						"Your global settings and tags are being created in your Documents/Vault folder. This is also the default location for your Vault Library to be stored." \
						"Creating these as a forced-step is a temporary solution, and more control will be offered in this wizard in future updates.\n\n" \
						
						"I hope you enjoy using Vault. I will soon be setting up a proper issue and feature tracking solution, so for now, please accept this is a early beta build."
				
					))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.OnClicked(this, &SVaultSetupWizard::Close)
					.Text(LOCTEXT("CloseWizardLbl", "Close"))
				]
			]
		]);
	
	FlashWindow();

}


FReply SVaultSetupWizard::Close()
{
	RequestDestroyWindow();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE