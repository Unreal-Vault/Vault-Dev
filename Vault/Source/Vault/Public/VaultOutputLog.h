// Copyright Daniel Orchard 2020

#pragma once

#include "CoreMinimal.h"
#include "SlateFwd.h"
#include "Misc/OutputDeviceHelper.h"

DECLARE_DELEGATE(FOnVaultMessageReceived);

struct FVaultLogMessage
{
public:
	FText Message;
	ELogVerbosity::Type Verbosity;

	FVaultLogMessage()
	{
		Message = FText();
		Verbosity = ELogVerbosity::NoLogging;
	}

	FVaultLogMessage(const FText& NewMessage, ELogVerbosity::Type InVerbosity)
		: Message(NewMessage)
		, Verbosity(InVerbosity)
	{ }
};


class VAULT_API FVaultOutputLog : public FOutputDevice
{
public:

	FVaultOutputLog();
	~FVaultOutputLog();

	TArray<FName> AcceptedLogCategories;

	// Delegate for the Publisher List to Bind too. 
	FOnVaultMessageReceived OnVaultMessageReceived;

	// Ptr to our List
	TSharedPtr<SListView<TSharedPtr< FVaultLogMessage>>> VaultLogList;

	// Generate Row for Log Message
	
	// All Messages
	TArray<TSharedPtr<FVaultLogMessage>> MessageList;

	bool CreateLogMessages(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category);

	// Serialize, which runs on tick, and notifies if a new message has been created.
	virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category) override;

};
