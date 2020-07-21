// Fill out your copyright notice in the Description page of Project Settings.


#include "VaultOutputLog.h"
#include "CoreGlobals.h"

#define LOCTEXT_NAMESPACE "VaultOutputLog"

// On construct, add as an output device, and serialize.
FVaultOutputLog::FVaultOutputLog()
{
	GLog->AddOutputDevice(this);
	GLog->SerializeBacklog(this);

	AcceptedLogCategories = 
	{
		FName(TEXT("LogVault")), 
		FName(TEXT("LogPakFiles"))
	};
}

// On destruct, remove from the GLog output list.
FVaultOutputLog::~FVaultOutputLog()
{
	// At shutdown, GLog may already be null
	if (GLog != NULL)
	{
		GLog->RemoveOutputDevice(this);
	}
};

bool FVaultOutputLog::CreateLogMessages(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category)
{
	if (true)
	{
		TSharedPtr<FVaultLogMessage> NewMessage = MakeShareable(new FVaultLogMessage);

		bool bIncludeTimestamps = false;
		bool bIncludeCategory = true;

		static ELogTimes::Type LogTimestampMode = bIncludeTimestamps ? ELogTimes::Local : ELogTimes::None;
		FName ShownCategory = bIncludeCategory ? Category : NAME_None;
		FString FormattedLine = FOutputDeviceHelper::FormatLogLine(Verbosity, ShownCategory, V, LogTimestampMode);

		NewMessage->Message = FText::FromString(FormattedLine);
		NewMessage->Verbosity = Verbosity;

		if (AcceptedLogCategories.Contains(Category))
		{
			UE_LOG(LogTemp, Error, TEXT("Found the right category"));
			MessageList.Add(NewMessage);
		}

		
		return true;
	}
	return false;
};


// Serialize - if a new message was received, broadcast the new message
void FVaultOutputLog::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category)
{
	CreateLogMessages(V, Verbosity, Category);
	OnVaultMessageReceived.ExecuteIfBound();
}


#undef LOCTEXT_NAMESPACE