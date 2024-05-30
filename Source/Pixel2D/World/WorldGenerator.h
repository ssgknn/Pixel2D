// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../Framework/StructDataTypes.h"
#include "WorldGenerator.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class PIXEL2D_API UWorldGenerator : public UObject
{
	GENERATED_BODY()
	
public:
	friend class AWorldHandler;

	UWorldGenerator();

private:

	UPROPERTY()
	TArray<FChunkData> WorldData;

	UPROPERTY()
	class UFileHandler* FileHandler;

public:

	void GenerateWorld(int32 ChunkElementCount);

	UFUNCTION(BlueprintCallable)
	bool GenerateWorld2(const FString& WorldName, int32 NumRegionsX, int32 NumRegionsZ);
};
