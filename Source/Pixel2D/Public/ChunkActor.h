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

private:
	UPROPERTY()
	class AWorldHandler* WorldHandlerRef;

	UPROPERTY()
	USceneComponent* AttachComponent;

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* CollisionBoxComponent;

	UPROPERTY(EditAnywhere, Category = "Componens")
	class UTextRenderComponent* TextComponent;

	UPROPERTY(EditAnywhere, Category = "Componens")
	class UPaperTileMapComponent* TileMapComponent;

	class UProceduralMeshComponent* ProceduralTerrainCollisionMesh;

	FChunkData ChunkData;

public:
	AChunkActor(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void LoadChunk(int32 ChunkGlobalX, int32 ChunkGlobalZ);

	UFUNCTION()
	void RefreshCollision(int32 blockSize, int32 chunkElementCount);

	void ModifyBlock(FVector HitLocation, int32 DesiredBlockID);

private:
	//TArray<int32> GetBlockTextureIDByChunkCoordinate(const FIntPoint& TargetChunkCoordinate);

private:
	// getters setters
	UFUNCTION()
	void SetFChunkData(FChunkData data);
};
