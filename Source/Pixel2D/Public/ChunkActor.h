// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

	UPROPERTY(EditAnywhere, Category = "TerrainMesh", Meta = (MakeEditWidget = true))
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

public:
	AChunkActor(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void LoadChunk();

	void ModifyBlock(FVector HitLocation, int32 DesiredBlockID);

};
