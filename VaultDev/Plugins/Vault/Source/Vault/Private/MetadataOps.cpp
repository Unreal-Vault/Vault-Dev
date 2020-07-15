// Fill out your copyright notice in the Description page of Project Settings.

#include "MetadataOps.h"
#include "Vault.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "JsonUtilities/Public/JsonUtilities.h"
#include "VaultSettings.h"
#include "HAL/FileManager.h"
//#include <JsonReader.h>

FMetadataOps::FMetadataOps()
{
}

FVaultMetadata FMetadataOps::ReadMetadata(FString File)
{
	// Raw data holder for json
	FString MetadataRaw;
	FFileHelper::LoadFileToString(MetadataRaw, *File);

	TSharedPtr<FJsonObject> JsonMetadata = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(MetadataRaw);
	FJsonSerializer::Deserialize(JsonReader, JsonMetadata);

	return ParseMetaJsonToVaultMetadata(JsonMetadata);

}

bool FMetadataOps::WriteMetadata(FVaultMetadata& Metadata)
{
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);

	FJsonSerializer::Serialize(ParseMetadataToJson(Metadata).ToSharedRef(), Writer);

	const FString Directory = FVaultSettings::Get().GetAssetLibraryRoot();

	const FString Filepath = Directory / Metadata.PackName.ToString() + ".meta";
	
	return FFileHelper::SaveStringToFile(OutputString, *Filepath);

}

TArray<FVaultMetadata> FMetadataOps::FindAllMetadataInLibrary()
{
	const FString LibraryPath = FVaultSettings::Get().GetAssetLibraryRoot();

	TArray<FVaultMetadata> MetaList;

	// Our custom file visitor that seeks out .meta files
	class FFindMetaFilesVisitor : public IPlatformFile::FDirectoryVisitor
	{
	public:

		FFindMetaFilesVisitor() {}
		TArray<FString> MetaFilenames;
		TArray<FString> MetaFilepaths;
		
		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory)
		{
			if (!bIsDirectory)
			{
				FString VisitedFile(FilenameOrDirectory);

				if (FPaths::GetExtension(VisitedFile) == TEXT("meta"))
				{
					MetaFilenames.Add(FPaths::GetBaseFilename(VisitedFile, true));
					MetaFilepaths.Add(VisitedFile);
				}
			}
			return true;
		}
	};



	// Create an instance of our custom visitor	   	 
	FFindMetaFilesVisitor Visitor;

	// Iterate Dir. Visitor will populate with the info we need.
	IFileManager::Get().IterateDirectory(*LibraryPath, Visitor);

	// Loop through all meta files we found. Use simple for to have a index
	for (int i = 0; i < Visitor.MetaFilenames.Num(); i++)
	{
		FString MetaRaw;
		FFileHelper::LoadFileToString(MetaRaw, *Visitor.MetaFilepaths[i]);

		TSharedPtr<FJsonObject> MetaJson = MakeShareable(new FJsonObject());
		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(MetaRaw);

		FJsonSerializer::Deserialize(JsonReader, MetaJson);
	
		MetaList.Add(ParseMetaJsonToVaultMetadata(MetaJson));
	
	}

	return MetaList;
}

FVaultMetadata FMetadataOps::ParseMetaJsonToVaultMetadata(TSharedPtr<FJsonObject> MetaFile)
{
	// New blank metadata struct
	FVaultMetadata Metadata;
	
	// User Info
	Metadata.Author = FName(*MetaFile->GetStringField("Author"));
	Metadata.PackName = FName(*MetaFile->GetStringField("PackName"));
	Metadata.Description = MetaFile->GetStringField("Description");

	// Tags
	TArray<TSharedPtr<FJsonValue>> TagValues = MetaFile->GetArrayField("Tags");
	TSet<FString> Tags;
	for (TSharedPtr<FJsonValue> TagRaw : TagValues)
	{
		Tags.Add(TagRaw->AsString());
	}
	Metadata.Tags = Tags;

	// Dates
	FDateTime CreationDate;
	FDateTime::Parse(MetaFile->GetStringField("CreationDate"), CreationDate);
	Metadata.CreationDate = CreationDate;

	FDateTime ModifiedDate;
	FDateTime::Parse(MetaFile->GetStringField("LastModified"), ModifiedDate );
	Metadata.LastModified = ModifiedDate;

	// System Info 
	Metadata.MachineID = MetaFile->GetStringField("MachineID");
	
	// Objects List
	TArray<TSharedPtr<FJsonValue>> ListOfObjects = MetaFile->GetArrayField("ObjectsInPack");
	TSet<FString> Objects;
	for (TSharedPtr<FJsonValue> TagRaw : ListOfObjects)
	{
		Objects.Add(TagRaw->AsString());
	}
	Metadata.ObjectsInPack = Objects;
	
	return Metadata;

}

TSharedPtr<FJsonObject> FMetadataOps::ParseMetadataToJson(FVaultMetadata Metadata)
{
	TSharedPtr<FJsonObject> MetaJson = MakeShareable(new FJsonObject());

	MetaJson->SetStringField("Author", Metadata.Author.ToString());
	MetaJson->SetStringField("PackName", Metadata.PackName.ToString());
	MetaJson->SetStringField("Description", Metadata.Description);

	// Tags

	TArray<TSharedPtr<FJsonValue>> TagsToWrite;
	for (FString TagText : Metadata.Tags)
	{
		TagsToWrite.Add(MakeShareable(new FJsonValueString(TagText)));
	
	}
	MetaJson->SetArrayField("Tags", TagsToWrite);

	// Dates

	MetaJson->SetStringField("CreationDate", Metadata.CreationDate.ToString());
	MetaJson->SetStringField("LastModified", Metadata.LastModified.ToString());

	// Sys info
	MetaJson->SetStringField("MachineID", Metadata.MachineID);

	// Objects
	TArray<TSharedPtr<FJsonValue>> ObjectsToWrite;
	for (FString ObjectText : Metadata.ObjectsInPack)
	{
		ObjectsToWrite.Add(MakeShareable(new FJsonValueString(ObjectText)));
	}
	MetaJson->SetArrayField("ObjectsInPack", ObjectsToWrite);

	return MetaJson;

}

FMetadataOps::~FMetadataOps()
{
}
