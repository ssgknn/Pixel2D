// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PixelGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PIXEL2D_API UPixelGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(BlueprintReadWrite)
	FString WorldName;
};
