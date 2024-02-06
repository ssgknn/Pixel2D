// Fill out your copyright notice in the Description page of Project Settings.


#include "FileHandler.h"

#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include <Serialization/BufferArchive.h>


void UFileHandler::SerializeChunkData(FArchive& Ar, FChunkData& ChunkData)
{
	Ar << ChunkData.ChunkCoordinate;
	Ar << ChunkData.BlockTextureID;
	Ar << ChunkData.bHasCollision;
}

void UFileHandler::SaveChunkDataToFile(const FString& FileName, const FChunkData& ChunkData)
{
	FBufferArchive Buffer;
	FMemoryWriter Writer(Buffer);

	FString SaveDirectory = FPaths::ProjectContentDir();
	FString FilePath = SaveDirectory / "WorldData" / FileName;

	// Serialize the chunk data
	SerializeChunkData(Writer, const_cast<FChunkData&>(ChunkData));

	if (FFileHelper::SaveArrayToFile(Buffer, *FilePath))
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.3f, FColor::Red, FString::Printf(TEXT("%f_regionSaved"), *FileName));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.3f, FColor::Red, TEXT("save error"));
	}
}

void UFileHandler::LoadChunkDataFromFile(const FString& FileName, FChunkData& ChunkData)
{
	TArray<uint8> FileData;

	FString SaveDirectory = FPaths::ProjectContentDir();
	FString FilePath = SaveDirectory / "WorldData" / FileName;


	if (FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		FMemoryReader Reader(FileData, true);

		// Deserialize the chunk data
		SerializeChunkData(Reader, ChunkData);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.3f, FColor::Red, TEXT("file read error"));
	}
}
