// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Slate.h"
#include "SlateExtras.h"
#include "VaultTypes.h"

class VAULT_API SBatchPublisherWindow : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SBatchPublisherWindow) {}
	SLATE_END_ARGS()

	// Construct Widget
	void Construct(const FArguments& InArgs);

};
