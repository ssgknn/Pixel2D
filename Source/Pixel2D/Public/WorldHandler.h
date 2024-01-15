// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldHandler.generated.h"

UCLASS()
class PIXEL2D_API AWorldHandler : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWorldHandler();

private:
	
#pragma region DataVariables
	UPROPERTY()
	int ChunkElementCount;

	UPROPERTY()
	int ChunkSize;

	UPROPERTY()
	double ChunkSizeHalf;

	UPROPERTY()
	int BlockSize;

	UPROPERTY()
	TArray<int> ChunkList_Top;
	UPROPERTY()
	TArray<int> ChunkList_Bottom;
	UPROPERTY()
	TArray<int> ChunkList_Left;
	UPROPERTY()
	TArray<int> ChunkList_Right;
	UPROPERTY()
	int ChunkList_Center;


	UPROPERTY()
	int ChunksCount;

	//ChunksCenter
	FVector ChunksCenter;

	//character position
	UPROPERTY()
	FVector PlayerPosition;

#pragma endregion DataVariables
		

protected:

	UPROPERTY()
	TArray<class UTextRenderComponent*> TextComponent;

	UPROPERTY()
	TArray<class UPaperTileMapComponent*> TileMapComponent;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tilesets")
	class UPaperTileSet* TileSet1;

	UPROPERTY()
	class AZDPlayerCharacterBase* CharacterRef;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void LoadChunks();

	UFUNCTION()
	void RefreshChunks(int direction);

	UFUNCTION()
	void CheckChunkBoundary();

	UFUNCTION()
	void RemoveBlockByIndex(int32 index);

	UFUNCTION()
	void AddBlockByVector(FVector& vector);

};
