// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkActor.h"
#include "Components/TextRenderComponent.h"
#include "PaperTileMapComponent.h"
#include "PaperTileMap.h"
#include "PaperTileSet.h"
#include "Components/BoxComponent.h"
#include "ProceduralMeshComponent.h"

#include "WorldHandler.h"
#include "../../../../../Program Files/Epic Games/UE_5.1/Engine/Plugins/Runtime/ProceduralMeshComponent/Source/ProceduralMeshComponent/Public/ProceduralMeshComponent.h"


AChunkActor::AChunkActor(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	SetRootComponent(RootComponent);

	AttachComponent = CreateDefaultSubobject<USceneComponent>(TEXT("AttachComponent"));

	TileMapComponent = CreateDefaultSubobject<UPaperTileMapComponent>(TEXT("TileMap"));
	TileMapComponent->AttachToComponent(AttachComponent, FAttachmentTransformRules::KeepRelativeTransform);

	CollisionBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBoxComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	CollisionBoxComponent->AttachToComponent(AttachComponent, FAttachmentTransformRules::KeepRelativeTransform);
	
	ProceduralTerrainCollisionMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TerrainCollisionMesh"));
	ProceduralTerrainCollisionMesh->AttachToComponent(AttachComponent, FAttachmentTransformRules::KeepRelativeTransform);


	// Initialize TileSets
	static ConstructorHelpers::FObjectFinder<UPaperTileSet> TileSetObject(TEXT("/Game/2DAssets/Assets/TS_Placeholder0"));
	TileSet0 = TileSetObject.Object;

}

void AChunkActor::BeginPlay()
{
	Super::BeginPlay();
	LoadChunk();
}

void AChunkActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AChunkActor::LoadChunk()
{	
	VerticesTerrain.Empty();
	TrianglesTerrain.Empty();

	int32 blockSize = WorldHandlerRef->BlockSize;
	int32 chunkElementCount = WorldHandlerRef->ChunkElementCount;

	//AttachComponent->SetRelativeLocation(FVector(BlockSize / 2.0f, 0.0f, -BlockSize / 2.0f));
	
	TileMapComponent->CreateNewTileMap(chunkElementCount, chunkElementCount, blockSize, blockSize, 1.0, true);
	TileMapComponent->SetRelativeLocation(FVector(blockSize / 2.0f, 0.0f, -blockSize / 2.0f));
	
	
	int32 i = 0;

	for (int32 z = 0; z < chunkElementCount; z++)
	{
		for (int32 x = 0; x < chunkElementCount; x++)
		{
			FPaperTileInfo LocalTileInfo;
			LocalTileInfo.TileSet = TileSet0;
			if (FMath::RandRange(0, 100) < 99)
			{
				LocalTileInfo.PackedTileIndex = 16;
			}
			else
			{
				LocalTileInfo.PackedTileIndex = 32;

				VerticesTerrain.Add(FVector(blockSize * x, -50, -blockSize * z));
				VerticesTerrain.Add(FVector(blockSize * (x + 1), -50, -blockSize * z));
				VerticesTerrain.Add(FVector(blockSize * (x + 1),  -50,  -blockSize * (z + 1)));
				VerticesTerrain.Add(FVector(blockSize * x, -50,  -blockSize * (z + 1)));
				VerticesTerrain.Add(FVector(blockSize * x, +100,  - blockSize * z));
				VerticesTerrain.Add(FVector(blockSize * (x + 1), +100,  -blockSize * z));
				VerticesTerrain.Add(FVector(blockSize * (x + 1), +100,  -blockSize * (z + 1)));
				VerticesTerrain.Add(FVector(blockSize * x, +100, -blockSize * (z + 1)));


				TrianglesTerrain.Add(i + 0);
				TrianglesTerrain.Add(i + 4);
				TrianglesTerrain.Add(i + 5);
				TrianglesTerrain.Add(i + 5);
				TrianglesTerrain.Add(i + 1);
				TrianglesTerrain.Add(i + 0);
				TrianglesTerrain.Add(i + 1);
				TrianglesTerrain.Add(i + 5);
				TrianglesTerrain.Add(i + 2);
				TrianglesTerrain.Add(i + 2);
				TrianglesTerrain.Add(i + 5);
				TrianglesTerrain.Add(i + 6);
				TrianglesTerrain.Add(i + 3);
				TrianglesTerrain.Add(i + 2);
				TrianglesTerrain.Add(i + 7);
				TrianglesTerrain.Add(i + 7);
				TrianglesTerrain.Add(i + 2);
				TrianglesTerrain.Add(i + 6);
				TrianglesTerrain.Add(i + 0);
				TrianglesTerrain.Add(i + 3);
				TrianglesTerrain.Add(i + 4);
				TrianglesTerrain.Add(i + 4);
				TrianglesTerrain.Add(i + 3);
				TrianglesTerrain.Add(i + 7);

				i += 8;
			}
			TileMapComponent->SetTile(x, z, 0, LocalTileInfo);
		}
	}

	/*for (int32 i = 0; i < VerticesTerrain.Num() / 8; i += 8)
	{
		TrianglesTerrain.Add(i + 0);
		TrianglesTerrain.Add(i + 4);
		TrianglesTerrain.Add(i + 5);
		TrianglesTerrain.Add(i + 5);
		TrianglesTerrain.Add(i + 1);
		TrianglesTerrain.Add(i + 0);
		TrianglesTerrain.Add(i + 1);
		TrianglesTerrain.Add(i + 5);
		TrianglesTerrain.Add(i + 2);
		TrianglesTerrain.Add(i + 2);
		TrianglesTerrain.Add(i + 5);
		TrianglesTerrain.Add(i + 6);
		TrianglesTerrain.Add(i + 3);
		TrianglesTerrain.Add(i + 2);
		TrianglesTerrain.Add(i + 7);
		TrianglesTerrain.Add(i + 7);
		TrianglesTerrain.Add(i + 2);
		TrianglesTerrain.Add(i + 6);
		TrianglesTerrain.Add(i + 0);
		TrianglesTerrain.Add(i + 3);
		TrianglesTerrain.Add(i + 4);
		TrianglesTerrain.Add(i + 4);
		TrianglesTerrain.Add(i + 3);
		TrianglesTerrain.Add(i + 7);
	}*/


	ProceduralTerrainCollisionMesh->CreateMeshSection(0, VerticesTerrain, TrianglesTerrain, TArray<FVector>(), TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), true);
	TileMapComponent->SetDefaultCollisionThickness(100, true);
	

	//CollisionBoxComponent->SetRelativeLocation(FVector(ChunkSize/2, 0, -ChunkSize/2));
	//CollisionBoxComponent->SetRelativeScale3D(FVector(ChunkSize/100.0f, 1, ChunkSize/100.0f));
	//CollisionBoxComponent->SetWorldScale3D(TileMapComponent->GetComponentScale());
	CollisionBoxComponent->SetBoxExtent(FVector(WorldHandlerRef->ChunkSize * 0.5f, 20.0f, WorldHandlerRef->ChunkSize * 0.5f));
	CollisionBoxComponent->SetRelativeLocation(FVector(WorldHandlerRef->ChunkSize * 0.5f, 10.0f, -(WorldHandlerRef->ChunkSize) * 0.5f));
}

void AChunkActor::ModifyBlock(FVector HitLocation, int32 DesiredBlockID)
{
	GEngine->AddOnScreenDebugMessage(-1, 0.3f, FColor::Red, TEXT("AChunkActor::ModifyBlock()"));
	FPaperTileInfo LocalTileInfo;
	LocalTileInfo.TileSet = TileSet0;
	LocalTileInfo.PackedTileIndex = DesiredBlockID;

	int32 BlockToChangeX = FMath::Floor(HitLocation.X / WorldHandlerRef->BlockSize);
	int32 BlockToChangeZ = FMath::Floor(HitLocation.Z / WorldHandlerRef->BlockSize);

	// neighbor block check if can place block or not, cant do without tile matrix. Should check for neighbor chunk data...
	/*TArray<FInt32Vector2> NeighborBlocks;
	NeighborBlocks.SetNum(4);
	NeighborBlocks[0] = FInt32Vector2();*/

	/*GEngine->AddOnScreenDebugMessage(-1, 0.3f, FColor::Red, FString::Printf(TEXT("B_X: %d"), BlockToChangeX));
	GEngine->AddOnScreenDebugMessage(-1, 0.3f, FColor::Red, FString::Printf(TEXT("B_Z: %d"), BlockToChangeZ));*/

	TileMapComponent->SetTile(BlockToChangeX, BlockToChangeZ, 0, LocalTileInfo);
}


