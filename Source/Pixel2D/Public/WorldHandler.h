// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StructDataTypes.h"
#include "WorldHandler.generated.h"

UCLASS()
class PIXEL2D_API AWorldHandler : public AActor
{
	GENERATED_BODY()
	
public:	

	friend class AChunkActor;

#pragma region DataVariables

	// ChunkDATA
	UPROPERTY()
	TArray<FChunkData> RegionData;

	UPROPERTY(ReplicatedUsing = OnRep_RegionDataChanged)
	TArray<FChunkChangeData> ChunkDataChange;
	
	// Chunk properties
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

	UPROPERTY()
	FIntPoint CenterChunkCoords;

	UPROPERTY()
	FIntPoint ChunkCoordsPlayerAt;

	UPROPERTY()
	TArray<FIntPoint> ActiveChunkCoordinates;

	UPROPERTY()
	TArray<FIntPoint> ChunkCoordinatesShouldBeActive;

	// chunk Actor class ref
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChunkProperty")
	TSubclassOf<AChunkActor> ChunkActorTemplate;

	// array of loaded chunks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelComponent")
	TArray<AChunkActor*> ChunksArray;

	// Fvector to compare player position to check if chunk loading needed
	/*UPROPERTY()
	FVector ChunkCenterPosition;*/
	
	UPROPERTY()
	class UWorldGenerator* WorldGen;

	UPROPERTY()
	class UFileHandler* FileHandler;

	// PlayerActor
	UPROPERTY()
	class AZDPlayerCharacterBase* PlayerActorRef;

	UPROPERTY()
	FVector PlayerPosition;

	uint8 bFirstLaunch;

#pragma endregion DataVariables


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Sets default values for this actor's properties
	AWorldHandler();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void AddChunks();

	//UFUNCTION(Server, Reliable)
	//void Server_AddChunks();

	UFUNCTION()
	uint8 IsChunkExists(const int32 X, const int32 Y);

	UFUNCTION(BlueprintCallable, Category = "Items")
	void SpawnPickup(FPickupData PickupData);

	/**[Server] Spawn a Pickup*/
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SpawnPickup(FPickupData PickupData);

	UFUNCTION()
	void UpdateRegionData(TArray<FChunkChangeData> chunksToUpdate);

	/*UFUNCTION(Server, Reliable)
	void Server_UpdateRegionData(const TArray<FChunkChangeData>& chunksToUpdate);*/

	UFUNCTION()
	void OnRep_RegionDataChanged();

	//UFUNCTION(Client, Reliable)
	//void Client_UpdateWorldData(TArray<FChunkData> chunksToUpdate);

	void GetFChunkDataByChunkCoordinate(FChunkData& desiredChunkData, const FIntPoint& TargetChunkCoordinate);

private:
	int32 FindChunkIndexByCoordinate(const FIntPoint& TargetCoordinate);
	int32 FindActiveChunkIndexByCoordinate(const FIntPoint& TargetCoordinate);

	void InitializeData();

	void UpdatePlayerPosition();

	void RemoveChunks();

	/*UFUNCTION(Server, Reliable)
	void Server_RemoveChunks();*/

	void RefreshChunks();

	FIntPoint GetChunkCoordPlayerAt();
};
