// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2DDynamic.h"

// Any metadata required for your assets can be added here.
struct FVaultMetadata
{
	FName Author;

	FName PackName;

	FString Description;

	TSet<FString> Tags;

	TSoftObjectPtr<UTexture2DDynamic> Thumbnail;

	FDateTime CreationDate;

	FDateTime LastModified;

	FString RelativePath;

	FString MachineID;

	TSet<FString> ObjectsInPack;



	FVaultMetadata()
	{
		Author = NAME_None;
		PackName = NAME_None;
		Description = FString();
		CreationDate = FDateTime::UtcNow();
		LastModified = FDateTime::UtcNow();
		RelativePath = FString();
		MachineID = FString();
	}

	//FVaultMetadata(FVaultMetadata inMeta)
	//{
	//	Author = inMeta.Author;
	//	PackName = inMeta.PackName;
	//	Description = inMeta.Description;
	//	CreationDate = inMeta.CreationDate;
	//	LastModified = inMeta.LastModified;
	//	RelativePath = inMeta.RelativePath;
	//	MachineID = inMeta.MachineID;
	//}

};



struct FTagFilteringItem
{
	FTagFilteringItem()

	{
	}

	virtual ~FTagFilteringItem() {}

	FString Tag;

	bool bFilterflag;

	int UseCount;

};

struct FDeveloperFilteringItem
{
	FDeveloperFilteringItem()

	{
	}

	virtual ~FDeveloperFilteringItem() {}

	FName Developer;

	bool bFilterflag;

	int UseCount;

};



class VAULT_API VaultTypes
{
public:
	VaultTypes();
	~VaultTypes();
};
