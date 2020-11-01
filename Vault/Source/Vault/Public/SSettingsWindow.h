// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Slate.h"
#include "SlateExtras.h"
#include "VaultTypes.h"
//#include "GameFramework/Actor.h"
//#include "SSettingsWindow.generated.h"

class VAULT_API SSettingsWindow : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SSettingsWindow) {}
	SLATE_END_ARGS()

	// Construct Widget
	void Construct(const FArguments& InArgs);

	//~SSettingsWindow();
	


};
