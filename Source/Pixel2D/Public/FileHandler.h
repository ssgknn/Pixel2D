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
	void SerializeChunkData(FArchive& Ar, FChunkData& ChunkData);

	void SaveChunkDataToFile(const FString& FileName, const FChunkData& ChunkData);

	void LoadChunkDataFromFile(const FString& FileName, FChunkData& ChunkData);
};
