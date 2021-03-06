// Copyright Daniel Orchard 2020

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWindow.h"

class VAULT_API SVaultSetupWizard : public SWindow
{
public:

	SLATE_BEGIN_ARGS(SVaultSetupWizard) {}
	SLATE_END_ARGS()

	/** Widget constructor */
	void Construct(const FArguments& Args);

private:
	FReply Close();

};
