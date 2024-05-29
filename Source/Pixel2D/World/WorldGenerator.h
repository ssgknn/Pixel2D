// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../Framework/StructDataTypes.h"
#include "WorldGenerator.generated.h"

/**
 * 
 */
UCLASS()
class PIXEL2D_API UWorldGenerator : public UObject
{
	GENERATED_BODY()
	
public:
	friend class AWorldHandler;

private:

	UPROPERTY()
	TArray<FChunkData> WorldData;

public:

	void GenerateWorld(int32 ChunkElementCount);
};
