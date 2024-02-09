// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkActor.h"
#include "Components/TextRenderComponent.h"
#include "PaperTileMapComponent.h"
#include "PaperTileMap.h"
#include "PaperTileSet.h"
#include "Components/BoxComponent.h"
#include "ProceduralMeshComponent.h"
#include "Net/UnrealNetwork.h"

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

	TextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextComp"));
	TextComponent->AttachToComponent(AttachComponent, FAttachmentTransformRules::KeepRelativeTransform);

	CollisionBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBoxComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	CollisionBoxComponent->AttachToComponent(AttachComponent, FAttachmentTransformRules::KeepRelativeTransform);
	
	ProceduralTerrainCollisionMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TerrainCollisionMesh"));
	ProceduralTerrainCollisionMesh->AttachToComponent(AttachComponent, FAttachmentTransformRules::KeepRelativeTransform);
	ProceduralTerrainCollisionMesh->SetVisibility(false);


	// Initialize TileSets
	static ConstructorHelpers::FObjectFinder<UPaperTileSet> TileSetObject(TEXT("/Game/2DAssets/Assets/TS_Placeholder0"));
	TileSet0 = TileSetObject.Object;

}

void AChunkActor::BeginPlay()
{
	Super::BeginPlay();
	//LoadChunk();
}

void AChunkActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AChunkActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AChunkActor, ChunkData);
}

void AChunkActor::LoadChunk()
{

	TextComponent->AddRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	TextComponent->AddRelativeLocation(FVector(0.0f, 1.0f, 0.0f));

	int32 blockSize = WorldHandlerRef->BlockSize;
	int32 chunkElementCount = WorldHandlerRef->ChunkElementCount;
	
	TileMapComponent->CreateNewTileMap(chunkElementCount, chunkElementCount, blockSize, blockSize, 1.0, true);
	TileMapComponent->SetRelativeLocation(FVector(blockSize / 2.0f, 0.0f, -blockSize / 2.0f));

	FPaperTileInfo LocalTileInfo;
	LocalTileInfo.TileSet = TileSet0;

	for (int32 x = 0; x < chunkElementCount; x++)
	{
		for (int32 z = 0; z < chunkElementCount; z++)
		{
			LocalTileInfo.PackedTileIndex = ChunkData.BlockTextureID[x + (chunkElementCount * z)]; //tilesInChunk[x + (chunkElementCount * z)];

			TileMapComponent->SetTile(x, z, 0, LocalTileInfo);

			// if (FMath::RandRange(0, 100) < 99)
			
		}
	}

	CollisionBoxComponent->SetBoxExtent(FVector(WorldHandlerRef->ChunkSize * 0.5f, 20.0f, WorldHandlerRef->ChunkSize * 0.5f));
	CollisionBoxComponent->SetRelativeLocation(FVector(WorldHandlerRef->ChunkSize * 0.5f, 10.0f, -(WorldHandlerRef->ChunkSize) * 0.5f));
	
	RefreshCollisionV2(blockSize, chunkElementCount);
}

void AChunkActor::RefreshCollision(int32 blockSize, int32 chunkElementCount)
{
	VerticesTerrain.Empty();
	TrianglesTerrain.Empty();

	//ProceduralTerrainCollisionMesh->ClearAllMeshSections();

	int32 i = 0;

	for (int32 x = 0; x < chunkElementCount; x++)
	{
		for (int32 z = 0; z < chunkElementCount; z++)
		{
			if (ChunkData.bHasCollision[x + (chunkElementCount * z)])
			{
				// Add vertices to the desired block corners
				VerticesTerrain.Add(FVector(blockSize * x, -50, -blockSize * z));
				VerticesTerrain.Add(FVector(blockSize * (x + 1), -50, -blockSize * z));
				VerticesTerrain.Add(FVector(blockSize * (x + 1), -50, -blockSize * (z + 1)));
				VerticesTerrain.Add(FVector(blockSize * x, -50, -blockSize * (z + 1)));
				VerticesTerrain.Add(FVector(blockSize * x, +100, -blockSize * z));
				VerticesTerrain.Add(FVector(blockSize * (x + 1), +100, -blockSize * z));
				VerticesTerrain.Add(FVector(blockSize * (x + 1), +100, -blockSize * (z + 1)));
				VerticesTerrain.Add(FVector(blockSize * x, +100, -blockSize * (z + 1)));

				// Make collision triangles from points
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
		}
	}

	GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Orange, FString::Printf(TEXT("Verts: %i"), VerticesTerrain.Num()));
	GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Green, FString::Printf(TEXT("Tris: %i"), TrianglesTerrain.Num()));


	ProceduralTerrainCollisionMesh->CreateMeshSection(0, VerticesTerrain, TrianglesTerrain, TArray<FVector>(), TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), true);
}

void AChunkActor::RefreshCollisionV2(int32 blockSize, int32 chunkElementCount)
{
	VerticesTerrain.Empty();
	TrianglesTerrain.Empty();

	//ProceduralTerrainCollisionMesh->ClearAllMeshSections();

	int32 i = 0;
	int32 zFirst;
	int32 zLast;

	for (int32 x = 0; x < chunkElementCount; x++)
	{
		zFirst = -1;

		for (int32 z = 0; z < chunkElementCount; z++)
		{
			if ((zFirst < 0) && (ChunkData.bHasCollision[x + (chunkElementCount * z)] == 1))
			{
				zFirst = z;
			}
			//if ((zFirst >= 0) && ((ChunkData.bHasCollision[x + (chunkElementCount * z)] == 0)) || (z == (chunkElementCount - 1)))
			if (zFirst >= 0)
			{
				if (ChunkData.bHasCollision[x + (chunkElementCount * z)] == 0)
				{
					zLast = z - 1;

					// Add vertices to the desired block corners
					VerticesTerrain.Add(FVector(blockSize * x, -50, -blockSize * zFirst));
					VerticesTerrain.Add(FVector(blockSize * (x + 1), -50, -blockSize * zFirst));
					VerticesTerrain.Add(FVector(blockSize * (x + 1), -50, -blockSize * (zLast + 1)));
					VerticesTerrain.Add(FVector(blockSize * x, -50, -blockSize * (zLast + 1)));
					VerticesTerrain.Add(FVector(blockSize * x, +100, -blockSize * zFirst));
					VerticesTerrain.Add(FVector(blockSize * (x + 1), +100, -blockSize * zFirst));
					VerticesTerrain.Add(FVector(blockSize * (x + 1), +100, -blockSize * (zLast + 1)));
					VerticesTerrain.Add(FVector(blockSize * x, +100, -blockSize * (zLast + 1)));

					// Make collision triangles from points
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

					zFirst = -1;
				}
				else if (z == (chunkElementCount - 1))
				{
					zLast = z;

					// Add vertices to the desired block corners
					VerticesTerrain.Add(FVector(blockSize * x, -50, -blockSize * zFirst));
					VerticesTerrain.Add(FVector(blockSize * (x + 1), -50, -blockSize * zFirst));
					VerticesTerrain.Add(FVector(blockSize * (x + 1), -50, -blockSize * (zLast + 1)));
					VerticesTerrain.Add(FVector(blockSize * x, -50, -blockSize * (zLast + 1)));
					VerticesTerrain.Add(FVector(blockSize * x, +100, -blockSize * zFirst));
					VerticesTerrain.Add(FVector(blockSize * (x + 1), +100, -blockSize * zFirst));
					VerticesTerrain.Add(FVector(blockSize * (x + 1), +100, -blockSize * (zLast + 1)));
					VerticesTerrain.Add(FVector(blockSize * x, +100, -blockSize * (zLast + 1)));

					// Make collision triangles from points
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

					zFirst = -1;
				}
			}
		}
	}

	/*GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Orange, FString::Printf(TEXT("Verts: %i"), VerticesTerrain.Num() ));
	GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Green, FString::Printf(TEXT("Tris: %i"), TrianglesTerrain.Num() ));
	*/
	ProceduralTerrainCollisionMesh->CreateMeshSection(0, VerticesTerrain, TrianglesTerrain, TArray<FVector>(), TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), true);
}

void AChunkActor::ModifyBlock(FVector HitLocation, int32 DesiredBlockID)
{
	int32 blockSize = WorldHandlerRef->BlockSize;
	int32 chunkElementCount = WorldHandlerRef->ChunkElementCount;

	FPaperTileInfo LocalTileInfo;
	LocalTileInfo.TileSet = TileSet0;
	LocalTileInfo.PackedTileIndex = 336; // DesiredBlockID;

	int32 BlockToChangeX = FMath::Floor(HitLocation.X / blockSize);
	int32 BlockToChangeZ = FMath::Floor(HitLocation.Z / blockSize);

	ChunkData.BlockTextureID[BlockToChangeX + (BlockToChangeZ * chunkElementCount)] = 336;  //DesiredBlockID;
	ChunkData.bHasCollision[BlockToChangeX + (BlockToChangeZ * chunkElementCount)] = 1;
	RefreshCollisionV2(blockSize, WorldHandlerRef->ChunkElementCount);

	TileMapComponent->SetTile(BlockToChangeX, BlockToChangeZ, 0, LocalTileInfo);

	Server_SetChunkData(HitLocation, DesiredBlockID);
}

//TArray<int32> AChunkActor::GetBlockTextureIDByChunkCoordinate(const FIntPoint& TargetChunkCoordinate)
//{
//	const FChunkData* FoundChunkData = WorldHandlerRef->WorldDATA.FindByPredicate([&](const FChunkData& ChunkData) {
//		return ChunkData.ChunkCoordinate == TargetChunkCoordinate;
//		});
//
//	if (FoundChunkData)
//	{
//		// Found the matching ChunkCoordinate, return the BlockTextureID
//		return FoundChunkData->BlockTextureID;
//	}
//
//	// If the function reaches here, the ChunkCoordinate was not found
//	// You may want to handle this case accordingly (e.g., return an empty array)
//	return TArray<int32>();
//}

// -------- OnRep --------
#pragma region OnRep

void AChunkActor::OnRep_ChunkDataChanged()
{
	RefreshCollisionV2(WorldHandlerRef->BlockSize, WorldHandlerRef->ChunkElementCount);

}

void AChunkActor::Server_SetChunkData_Implementation(FVector HitLocation, int32 DesiredBlockID)
{
	if (HasAuthority())
	{
		int32 blockSize = WorldHandlerRef->BlockSize;
		int32 chunkElementCount = WorldHandlerRef->ChunkElementCount;

		FPaperTileInfo LocalTileInfo;
		LocalTileInfo.TileSet = TileSet0;
		LocalTileInfo.PackedTileIndex = 336; // DesiredBlockID;

		int32 BlockToChangeX = FMath::Floor(HitLocation.X / blockSize);
		int32 BlockToChangeZ = FMath::Floor(HitLocation.Z / blockSize);

		ChunkData.BlockTextureID[BlockToChangeX + (BlockToChangeZ * chunkElementCount)] = 336;  //DesiredBlockID;
		ChunkData.bHasCollision[BlockToChangeX + (BlockToChangeZ * chunkElementCount)] = 1;
	}
}

#pragma endregion OnRep



// -------- SettersGetters --------
#pragma region SettersGetters

void AChunkActor::SetFChunkData(FChunkData data)
{
	ChunkData = data;
}

#pragma endregion SettersGetters

