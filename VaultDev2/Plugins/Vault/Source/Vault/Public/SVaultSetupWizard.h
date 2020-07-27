// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWindow.h"

/**
 * 
 */
class VAULT_API SVaultSetupWizard : public SWindow
{
public:

	SLATE_BEGIN_ARGS(SVaultSetupWizard)
	{
	}
	SLATE_END_ARGS()

	SVaultSetupWizard();

	~SVaultSetupWizard();

	/** Widget constructor */
	void Construct(const FArguments& Args);

private:
	FReply Close();

};
