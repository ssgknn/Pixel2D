// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Pickup.h"
#include "Item.h"

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "StructDataTypes.generated.h"


/**
 * 
 */
UCLASS()
class PIXEL2D_API UStructDataTypes : public UObject
{
	GENERATED_BODY()
	
};

/**
*	ChunkData
*/
USTRUCT(BlueprintType)
struct PIXEL2D_API FChunkData
{
	GENERATED_USTRUCT_BODY()

	FIntPoint ChunkCoordinate;
	TArray<int32> BlockTextureID;
	TArray<uint8> bHasCollision;
	TArray<FPickupData> Pickups;
};

/**
*	PickupInfo
*/
USTRUCT(BlueprintType)
struct PIXEL2D_API FPickupData
{
	GENERATED_USTRUCT_BODY()

	TSubclassOf<class APickup> PickupType;
	UItem* Item;
	FVector Location;

};
