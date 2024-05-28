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
* 
*	To holds chunks data informations.
*/
USTRUCT(BlueprintType)
struct PIXEL2D_API FChunkData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FIntPoint ChunkCoordinate;

	UPROPERTY()
	TArray<int32> BlockTextureID;

	UPROPERTY()
	TArray<uint8> bHasCollision;

	UPROPERTY()
	class AWorldHandler* WorldHandlerRef;

	TArray<FPickupData> Pickups;
};

/**
*	ChunkChangeData
* 
*	To hold chunk data change in small size. Only the changes are stored.
*/
USTRUCT(BlueprintType)
struct PIXEL2D_API FChunkChangeData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FIntPoint ChunkCoordinate;

	UPROPERTY()
	TArray<int32> BlockIdx;

	UPROPERTY()
	TArray<int32> BlockTextureID;

	UPROPERTY()
	TArray<uint8> bHasCollision;
};

/**
*	PlacementData
* 
*	Template for each placeable object.
*/
USTRUCT(BlueprintType)
struct PIXEL2D_API FPlacementData
{
	GENERATED_USTRUCT_BODY()

	FIntPoint Dimensions;
	TArray<int32> BlockTextureID;
	TArray<uint8> bHasCollision;
};

/**
*	PickupInfo
* 
*  For store pickups in the world.
*/
USTRUCT(BlueprintType)
struct PIXEL2D_API FPickupData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TSubclassOf<class APickup> PickupType;

	UPROPERTY()
	UItem* Item;

	UPROPERTY()
	FVector Location;

};
