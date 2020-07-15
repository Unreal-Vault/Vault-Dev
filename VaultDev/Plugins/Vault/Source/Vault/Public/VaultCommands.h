// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "VaultStyle.h"

class FVaultCommands : public TCommands<FVaultCommands>
{
public:

	FVaultCommands()
		: TCommands<FVaultCommands>(TEXT("Vault"), NSLOCTEXT("Contexts", "Vault", "Vault Plugin"), NAME_None, FVaultStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
