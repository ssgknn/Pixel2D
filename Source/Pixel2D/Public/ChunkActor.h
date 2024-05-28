// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StructDataTypes.h"
#include "ChunkActor.generated.h"

UCLASS()
class PIXEL2D_API AChunkActor : public AActor
{
	GENERATED_BODY()
	
public:
	friend class AWorldHandler;

	UPROPERTY(EditAnywhere, Category = "Componens")
	class UTextRenderComponent* TextComponent;

#pragma region Tilesets

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tilesets")
	class UPaperTileSet* TileSet0;

	

#pragma endregion Tilesets

#pragma region ProceduralMeshData

	//EditAnywhere, Category = "TerrainMesh", Meta = (MakeEditWidget = true)
	UPROPERTY()
	TArray<FVector> VerticesTerrain;

	UPROPERTY()
	TArray<int> TrianglesTerrain;

#pragma endregion ProceduralMeshData

	UPROPERTY(ReplicatedUsing = OnRep_ChunkDataChanged)
	FChunkData ChunkData;

private:
	/*UPROPERTY()
	class AWorldHandler* WorldHandlerRef;*/

	UPROPERTY()
	USceneComponent* AttachComponent;

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* CollisionBoxComponent;

	UPROPERTY(EditAnywhere, Category = "Componens")
	class UPaperTileMapComponent* TileMapComponent;

	UPROPERTY(VisibleAnywhere)
	class UProceduralMeshComponent* ProceduralTerrainCollisionMesh;

	

public:
	AChunkActor(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

public:

	UFUNCTION()
	void LoadChunk();

	UFUNCTION()
	void RefreshChunk();

	UFUNCTION(Server, Reliable)
	void Server_RefreshChunk();

	UFUNCTION()
	void RefreshCollision(int32 blockSize, int32 chunkElementCount);

	UFUNCTION()
	void RefreshCollisionV2(int32 blockSize, int32 chunkElementCount);

	UFUNCTION()
	void RefreshCollisionV3(int32 blockSize, int32 chunkElementCount);

	UFUNCTION()
	void RefreshCollisionV4(int32 blockSize, int32 chunkElementCount);
	
	//UFUNCTION()
	//void ModifyBlock(FVector HitLocation, int32 DesiredBlockID);
	//
	//UFUNCTION(Server, reliable)
	//void Server_ModifyBlock(FVector HitLocation, int32 DesiredBlockID);

private:

	//// -------- OnRep --------
	UFUNCTION()
	void OnRep_ChunkDataChanged();

	//// -------- RPC --------
	//UFUNCTION(Server, reliable)
	//void Server_SetChunkData(FVector HitLocation, int32 DesiredBlockID);

	// getters setters
	UFUNCTION()
	void SetFChunkData(FChunkData data);
};
