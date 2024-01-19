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

	//Chunk
	UPROPERTY()
	int32 RenderRange;

	UPROPERTY()
	int32 ChunkElementCount;

	UPROPERTY()
	int32 ChunkSize;

	UPROPERTY()
	int32 ChunkSizeHalf;

	UPROPERTY()
	int32 BlockSize;

	UPROPERTY()
	int32 ChunksCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChunkProperty")
	TArray<FIntPoint> ChunkCoordinates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChunkProperty")
	int32 ChunkX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChunkProperty")
	int32 ChunkZ;

	//chunk Actor class ref
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChunkProperty")
	TSubclassOf<AChunkActor> ChunkActorTemplate;

	//array of loaded chunks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelComponent")
	TArray<AChunkActor*> ChunksArray;

	//Fvector to compare player position to check if chunk loading needed
	UPROPERTY()
	FVector ChunkCenterPosition;
	

	//PlayerActor
	UPROPERTY()
	class AZDPlayerCharacterBase* PlayerActorRef;

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

	void InitializeData();

	FVector GetHalfVoxelSize();

	FIntPoint GetChunkCords(const FVector LocalPosition);

	void UpdatePlayerPosition();

	// Removing all chunks which are not in render range anymore
	void RemoveChunks();

	// Checking if chunk is in specific render distance
	bool CheckRadius(const float CenterX, const float CenterY);

	void CheckChunkLoads();

};
