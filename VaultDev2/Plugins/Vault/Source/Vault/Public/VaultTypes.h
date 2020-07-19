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

	// Constructor
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

	bool IsMetaValid()
	{
		return PackName != NAME_None;
	}
};

// Tag Filter Struct used for the Loader UI
struct FTagFilteringItem
{
	FTagFilteringItem() {}
	virtual ~FTagFilteringItem() {}
	FString Tag;
	int UseCount;
};

// Developer Name Struct used for Loader UI
struct FDeveloperFilteringItem
{
	FDeveloperFilteringItem() {}
	virtual ~FDeveloperFilteringItem() {}
	FName Developer;
	bool bFilterflag;
	int UseCount;
};

//// Used to gather all related files in a directory once so we dont need to keep querying the disk for information.
//// This will be populated as a Map, using the Filename Clean for the Key.
//struct FVaultFileDirectoryMap
//{
//	// All 3 files will share this same name, differing only by extension (upack, png, and meta)
//
//	// Absolute Paths, Stored as FFilePath for the sake of explanation, type is identical to FString.
//	FFilePath MetaFileAbsolutePath;
//	FFilePath AssetPackAbsolutePath;
//	FFilePath ThumbnailPathAbsolute;
//};