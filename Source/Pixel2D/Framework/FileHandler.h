// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "StructDataTypes.h"
#include "FileHandler.generated.h"

/**
 * 
 */
UCLASS()
class PIXEL2D_API UFileHandler : public UObject
{
	GENERATED_BODY()
	

public:
	bool SaveRegion(const FString& WorldName, const FIntPoint& RegionCoordinate, const TArray<FChunkData>& ChunkDataArray, int32 N);
	bool LoadRegion(const FString& WorldName, const FIntPoint& RegionCoordinate, TArray<FChunkData>& ChunkDataArray, int32 N);
	bool LoadChunksToWorldData(const FString& WorldName, TMap<FIntPoint, FChunkData>& ChunkDataMap);

private:
	FString GetRegionFilePath(const FString& WorldName, const FIntPoint& RegionCoordinate) const;


//public:
//	void SerializeChunkData(FArchive& Ar, FChunkData& ChunkData);
//
//	void SaveChunkDataToFile(const FString& FileName, const FChunkData& ChunkData);
//
//	void LoadChunkDataFromFile(const FString& FileName, FChunkData& ChunkData);
};
