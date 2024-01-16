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

#pragma region DataVariables

	UPROPERTY()
	int32 RenderRange = 3;

	UPROPERTY()
	int32 ChunkElementCount = 64;

	UPROPERTY()
	int32 ChunkSize;

	UPROPERTY()
	int32 ChunkSizeHalf;

	UPROPERTY()
	int32 BlockSize = 16;

	UPROPERTY()
	int32 ChunksCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChunkProperty")
	TArray<FIntPoint> ChunkCoordinates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChunkProperty")
	int32 ChunkX = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChunkProperty")
	int32 ChunkZ = 0;

	//ChunksCenter
	FVector ChunksCenter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChunkProperty")
	TSubclassOf<AChunkActor> ChunkActorTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelComponent")
	TArray<AChunkActor*> ChunksArray;
	
	UPROPERTY()
	class AZDPlayerCharacterBase* CharacterRef;

	//character position
	UPROPERTY()
	FVector PlayerPosition;

#pragma endregion DataVariables


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	// Sets default values for this actor's properties
	AWorldHandler();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void AddChunks();

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	AChunkActor* FindChunk(const int32 X, const int32 Y);

	UFUNCTION()
	void RefreshChunks(int direction);

	UFUNCTION()
	void RemoveBlockByIndex(int32 index);

	UFUNCTION()
	void AddBlockByVector(FVector& vector);

private:

	FVector GetHalfVoxelSize();

	FIntPoint GetChunkCords(const FVector LocalPosition);

	void UpdatePlayerPosition();

	// Removing all chunks which are not in render range anymore
	void RemoveChunks();

	// Checking if chunk is in specific render distance
	bool CheckRadius(const float CenterX, const float CenterY);

};
