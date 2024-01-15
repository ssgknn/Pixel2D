// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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
*	Sector element - X coordinate
*/
USTRUCT(BlueprintType)
struct PIXEL2D_API FSectorX
{
	GENERATED_USTRUCT_BODY()

	/*UPROPERTY(BlueprintReadWrite, Category = "SectorX")
	TArray<FVoxel> Voxels;*/

};
