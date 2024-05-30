// Fill out your copyright notice in the Description page of Project Settings.


#include "FileHandler.h"

#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include <Serialization/BufferArchive.h>



FString UFileHandler::GetRegionFilePath(const FString& WorldName, const FIntPoint& RegionCoordinate) const
{
	FString DirectoryPath = FPaths::ProjectSavedDir() / TEXT("WORLDS") / WorldName;
	FString FileName = FString::Printf(TEXT("region_%d_%d.dat"), RegionCoordinate.X, RegionCoordinate.Y);
	return DirectoryPath / FileName;
}

bool UFileHandler::SaveRegion(const FString& WorldName, const FIntPoint& RegionCoordinate, const TArray<FChunkData>& ChunkDataArray, int32 N)
{
	FString DirectoryPath = FPaths::ProjectSavedDir() / TEXT("WORLDS") / WorldName;
	if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*DirectoryPath))
	{
		FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*DirectoryPath);
	}

	FString FilePath = DirectoryPath / FString::Printf(TEXT("region_%d_%d.dat"), RegionCoordinate.X, RegionCoordinate.Y);

	TArray<uint8> BinaryData;
	FMemoryWriter Writer(BinaryData, true);
	//Writer << N;
	Writer << const_cast<TArray<FChunkData>&>(ChunkDataArray); // Note the cast for const correctness

	return FFileHelper::SaveArrayToFile(BinaryData, *FilePath);
}

bool UFileHandler::LoadRegion(const FString& WorldName, const FIntPoint& RegionCoordinate, TArray<FChunkData>& ChunkDataArray, int32 N)
{
	FString FilePath = GetRegionFilePath(WorldName, RegionCoordinate);

	TArray<uint8> BinaryData;
	if (!FFileHelper::LoadFileToArray(BinaryData, *FilePath))
	{
		return false;
	}

	FMemoryReader Reader(BinaryData, true);
	//Reader << N;
	Reader << ChunkDataArray;

	return true;
}

bool UFileHandler::LoadChunksToWorldData(const FString& WorldName, TMap<FIntPoint, FChunkData>& ChunkDataMap)
{
	// Get all files with the .chunk extension in the specified folder
	TArray<FString> ChunkFiles;
	FString DirectoryPath = FPaths::ProjectSavedDir() / TEXT("WORLDS") / WorldName;
	IFileManager::Get().FindFiles(ChunkFiles, *DirectoryPath, TEXT("*.dat"));

	for (const FString& ChunkFileName : ChunkFiles)
	{
		FString FilePath = DirectoryPath / ChunkFileName;

		TArray<uint8> BinaryData;
		if (!FFileHelper::LoadFileToArray(BinaryData, *FilePath))
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to load file: %s"), *FilePath);
			continue; // Skip this file and continue with the next
		}

		FMemoryReader Reader(BinaryData, true);
		//int32 ChunkId;
		TArray<FChunkData> ChunkData;

		// Assuming the file starts with an int32 ChunkId followed by the FChunkData
		//Reader << ChunkId;
		ChunkData.Empty();
		Reader << ChunkData;

		for (FChunkData& Chunk : ChunkData)
		{
			ChunkDataMap.Add(Chunk.ChunkCoordinate, Chunk);
		}
	
	}

	return true;
}

// oldCode
// 
// void UFileHandler::SerializeChunkData(FArchive& Ar, FChunkData& ChunkData)
//{
//	Ar << ChunkData.ChunkCoordinate;
//	Ar << ChunkData.BlockTextureID;
//	Ar << ChunkData.bHasCollision;
//}
//
//void UFileHandler::SaveChunkDataToFile(const FString& FileName, const FChunkData& ChunkData)
//{
//	FBufferArchive Buffer;
//	FMemoryWriter Writer(Buffer);
//
//	FString SaveDirectory = FPaths::ProjectContentDir();
//	FString FilePath = SaveDirectory / "WorldData" / FileName;
//
//	// Serialize the chunk data
//	SerializeChunkData(Writer, const_cast<FChunkData&>(ChunkData));
//
//	if (FFileHelper::SaveArrayToFile(Buffer, *FilePath))
//	{
//		GEngine->AddOnScreenDebugMessage(-1, 0.3f, FColor::Red, FString::Printf(TEXT("%f_regionSaved"), *FileName));
//	}
//	else
//	{
//		GEngine->AddOnScreenDebugMessage(-1, 0.3f, FColor::Red, TEXT("save error"));
//	}
//}
//
//void UFileHandler::LoadChunkDataFromFile(const FString& FileName, FChunkData& ChunkData)
//{
//	TArray<uint8> FileData;
//
//	FString SaveDirectory = FPaths::ProjectContentDir();
//	FString FilePath = SaveDirectory / "WorldData" / FileName;
//
//
//	if (FFileHelper::LoadFileToArray(FileData, *FilePath))
//	{
//		FMemoryReader Reader(FileData, true);
//
//		// Deserialize the chunk data
//		SerializeChunkData(Reader, ChunkData);
//	}
//	else
//	{
//		GEngine->AddOnScreenDebugMessage(-1, 0.3f, FColor::Red, TEXT("file read error"));
//	}
//}
