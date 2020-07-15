// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "VaultCommands.h"

#define LOCTEXT_NAMESPACE "FVaultModule"

void FVaultCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "Vault", "Execute Vault action", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
