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

	TArray<TPair<uint8, TArray<FIntPoint>>> ChunkCoordinatesShouldBeActiveByPlayers;

	// array of loaded chunks
	UPROPERTY()
	TMap<FIntPoint, AChunkActor*> ChunkActors;

	// chunk Actor class ref
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChunkProperty")
	TSubclassOf<AChunkActor> ChunkActorTemplate;
	
	UPROPERTY()
	class UWorldGenerator* WorldGen;

	UPROPERTY()
	class UFileHandler* FileHandler;

	// PlayerActors IDs
	UPROPERTY()
	TMap<class AZDPlayerCharacterBase*, uint8> PlayerIDs;

	uint8 currentPlayerID = 0;

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

	UFUNCTION(Server, Reliable)
	void Server_RegisterPlayerID(class AZDPlayerCharacterBase* playerToRegister);

	/*UFUNCTION(Server, Reliable)
	void Server_AddPlayerChunkCoordinates(FIntPoint newChenterChunkCoord);*/

	UFUNCTION()
	void LoadChunks(uint8 playerID, FIntPoint newChenterChunk);

	UFUNCTION(Server, Reliable)
	void Server_LoadChunks(uint8 playerID, FIntPoint newChenterChunkCoord);

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

	/*UFUNCTION(Server, Reliable)
	void Server_RemoveChunks();*/
};
