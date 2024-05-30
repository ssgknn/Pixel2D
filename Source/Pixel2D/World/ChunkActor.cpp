// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkActor.h"
#include "Components/TextRenderComponent.h"
#include "PaperTileMapComponent.h"
#include "PaperTileMap.h"
#include "PaperTileSet.h"
#include "Components/BoxComponent.h"
#include "ProceduralMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "PaperTileLayer.h"

#include "WorldHandler.h"
#include "../../../../../Program Files/Epic Games/UE_5.1/Engine/Plugins/Runtime/ProceduralMeshComponent/Source/ProceduralMeshComponent/Public/ProceduralMeshComponent.h"


AChunkActor::AChunkActor(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	//NetCullDistanceSquared = 1000;

	AttachComponent = CreateDefaultSubobject<USceneComponent>(TEXT("AttachComponent"));

	SetRootComponent(AttachComponent);

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
	ProceduralTerrainCollisionMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ProceduralTerrainCollisionMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	ProceduralTerrainCollisionMesh->SetVisibility(false);
	ProceduralTerrainCollisionMesh->bUseComplexAsSimpleCollision = true;
	ProceduralTerrainCollisionMesh->bEnableAutoLODGeneration = false;


	// Initialize TileSets
	static ConstructorHelpers::FObjectFinder<UPaperTileSet> TileSetObject(TEXT("/Game/2DAssets/PrototypeItch/Assets/TS_Placeholder0"));
	TileSet0 = TileSetObject.Object;

}

void AChunkActor::BeginPlay()
{
	Super::BeginPlay();
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
	TextComponent->SetText(FText::FromString(FString::Printf(TEXT("%i, %i"), ChunkData.ChunkCoordinate.X, ChunkData.ChunkCoordinate.Y)));

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

	RefreshCollisionV3(blockSize, chunkElementCount);
	
}

void AChunkActor::RefreshChunk()
{
	int32 blockSize = WorldHandlerRef->BlockSize;
	int32 chunkElementCount = WorldHandlerRef->ChunkElementCount;

	TileMapComponent->CreateNewTileMap(chunkElementCount, chunkElementCount, blockSize, blockSize, 1.0, true);

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

	RefreshCollisionV3(blockSize, chunkElementCount);
}

void AChunkActor::Server_RefreshChunk_Implementation()
{
	RefreshChunk();
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

	ProceduralTerrainCollisionMesh->ClearMeshSection(0);
	/*GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Orange, FString::Printf(TEXT("Verts: %i"), VerticesTerrain.Num() ));
	GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Green, FString::Printf(TEXT("Tris: %i"), TrianglesTerrain.Num() ));
	*/
	ProceduralTerrainCollisionMesh->CreateMeshSection(0, VerticesTerrain, TrianglesTerrain, TArray<FVector>(), TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), true);
}

void AChunkActor::RefreshCollisionV3(int32 blockSize, int32 chunkElementCount)
{
	VerticesTerrain.Empty();
	TrianglesTerrain.Empty();

	//ProceduralTerrainCollisionMesh->ClearAllMeshSections();

	int32 i = 0;

	TArray<bool> visited;
	visited.SetNumZeroed(chunkElementCount * chunkElementCount);

	for (int32 x = 0; x < chunkElementCount; x++)
	{
		for (int32 z = 0; z < chunkElementCount; z++)
		{
			if (ChunkData.bHasCollision[x + chunkElementCount * z] == 1 && !visited[x + chunkElementCount * z])
			{
				int32 width = 1;
				int32 height = 1;

				// Find the width of the rectangle
				while (x + width < chunkElementCount && ChunkData.bHasCollision[(x + width) + chunkElementCount * z] == 1 && !visited[(x + width) + chunkElementCount * z])
				{
					width++;
				}

				// Find the height of the rectangle
				bool expandHeight = true;
				while (expandHeight && z + height < chunkElementCount)
				{
					for (int32 k = 0; k < width; k++)
					{
						if (ChunkData.bHasCollision[(x + k) + chunkElementCount * (z + height)] == 0 || visited[(x + k) + chunkElementCount * (z + height)])
						{
							expandHeight = false;
							break;
						}
					}
					if (expandHeight)
					{
						height++;
					}
				}

				// Mark visited cells
				for (int32 dx = 0; dx < width; dx++)
				{
					for (int32 dz = 0; dz < height; dz++)
					{
						visited[(x + dx) + chunkElementCount * (z + dz)] = true;
					}
				}

				// Add vertices to the desired block corners
				VerticesTerrain.Add(FVector(blockSize * x, -50, -blockSize * z));
				VerticesTerrain.Add(FVector(blockSize * (x + width), -50, -blockSize * z));
				VerticesTerrain.Add(FVector(blockSize * (x + width), -50, -blockSize * (z + height)));
				VerticesTerrain.Add(FVector(blockSize * x, -50, -blockSize * (z + height)));
				VerticesTerrain.Add(FVector(blockSize * x, +100, -blockSize * z));
				VerticesTerrain.Add(FVector(blockSize * (x + width), +100, -blockSize * z));
				VerticesTerrain.Add(FVector(blockSize * (x + width), +100, -blockSize * (z + height)));
				VerticesTerrain.Add(FVector(blockSize * x, +100, -blockSize * (z + height)));

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

	ProceduralTerrainCollisionMesh->ClearMeshSection(0);

	ProceduralTerrainCollisionMesh->CreateMeshSection(0, VerticesTerrain, TrianglesTerrain, TArray<FVector>(), TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), true);
}

void AChunkActor::RefreshCollisionV4(int32 blockSize, int32 chunkElementCount)
{
	VerticesTerrain.Reset();
	TrianglesTerrain.Reset();

	TArray<bool> visited;
	visited.Init(false, chunkElementCount * chunkElementCount);

	int32 i = 0; // Declare i here

	for (int32 z = 0; z < chunkElementCount; ++z)
	{
		for (int32 x = 0; x < chunkElementCount; ++x)
		{
			int32 index = x + chunkElementCount * z;
			if (ChunkData.bHasCollision[index] == 1 && !visited[index])
			{
				int32 width = 0;
				int32 height = 0;

				// Determine the width of the rectangle
				while ((x + width) < chunkElementCount && ChunkData.bHasCollision[index + width] == 1 && !visited[index + width])
				{
					++width;
				}

				// Determine the height of the rectangle
				bool expandHeight = true;
				while (expandHeight && (z + height) < chunkElementCount)
				{
					for (int32 w = 0; w < width; ++w)
					{
						if (ChunkData.bHasCollision[index + w + chunkElementCount * height] == 0 || visited[index + w + chunkElementCount * height])
						{
							expandHeight = false;
							break;
						}
					}
					if (expandHeight)
					{
						++height;
					}
				}

				// Mark the cells within the rectangle as visited
				for (int32 dz = 0; dz < height; ++dz)
				{
					for (int32 dx = 0; dx < width; ++dx)
					{
						visited[index + dx + chunkElementCount * dz] = true;
					}
				}

				// Add vertices for the block corners
				FVector v0(blockSize * x, -50, -blockSize * z);
				FVector v1(blockSize * (x + width), -50, -blockSize * z);
				FVector v2(blockSize * (x + width), -50, -blockSize * (z + height));
				FVector v3(blockSize * x, -50, -blockSize * (z + height));
				FVector v4(blockSize * x, +100, -blockSize * z);
				FVector v5(blockSize * (x + width), +100, -blockSize * z);
				FVector v6(blockSize * (x + width), +100, -blockSize * (z + height));
				FVector v7(blockSize * x, +100, -blockSize * (z + height));

				VerticesTerrain.Append({ v0, v1, v2, v3, v4, v5, v6, v7 });

				// Add collision triangles
				TrianglesTerrain.Append({
					i + 0, i + 4, i + 5, i + 5, i + 1, i + 0,
					i + 1, i + 5, i + 2, i + 2, i + 5, i + 6,
					i + 3, i + 2, i + 7, i + 7, i + 2, i + 6,
					i + 0, i + 3, i + 4, i + 4, i + 3, i + 7
					});

				i += 8;
			}
		}
	}

	ProceduralTerrainCollisionMesh->ClearMeshSection(0);
	ProceduralTerrainCollisionMesh->CreateMeshSection(0, VerticesTerrain, TrianglesTerrain, TArray<FVector>(), TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), true);
}

//void AChunkActor::ModifyBlock(FVector HitLocation, FPlacementData changeData)
//{
//	if (HasAuthority())
//	{
//		int32 blockSize = WorldHandlerRef->BlockSize;
//		int32 chunkElementCount = WorldHandlerRef->ChunkElementCount;
//
//		FPaperTileInfo LocalTileInfo;
//		LocalTileInfo.TileSet = TileSet0;
//		LocalTileInfo.PackedTileIndex = 5; //336 - brick, 5 - air // DesiredBlockID;
//
//		for (int32 x = 0; x < changeData.Dimensions.X; x++)
//		{
//			for (int32 z = 0; z < changeData.Dimensions.Y ; z++)
//			{
//
//			}
//		}
//
//		int32 BlockToChangeX = FMath::Floor(HitLocation.X / blockSize);
//		int32 BlockToChangeZ = FMath::Floor(HitLocation.Z / blockSize);
//
//		ChunkData.BlockTextureID[BlockToChangeX + (BlockToChangeZ * chunkElementCount)] = 5; //DesiredBlockID;
//		ChunkData.bHasCollision[BlockToChangeX + (BlockToChangeZ * chunkElementCount)] = 0;
//		RefreshCollisionV2(blockSize, chunkElementCount);
//
//		TileMapComponent->SetTile(BlockToChangeX, BlockToChangeZ, 0, LocalTileInfo);
//	}
//	else
//	{
//		Server_ModifyBlock(HitLocation, DesiredBlockID);
//	}
//}

//void AChunkActor::Server_ModifyBlock_Implementation(FVector HitLocation, int32 DesiredBlockID)
//{
//	ModifyBlock(HitLocation, DesiredBlockID);
//}

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

// //-------- OnRep --------
//#pragma region OnRep
//
//void AChunkActor::OnRep_ChunkDataChanged()
//{
//	//RefreshCollisionV2(WorldHandlerRef->BlockSize, WorldHandlerRef->ChunkElementCount);
//	LoadChunk();
//}
//
//void AChunkActor::Server_SetChunkData_Implementation(FVector HitLocation, int32 DesiredBlockID)
//{
//	if (HasAuthority())
//	{
//		int32 blockSize = WorldHandlerRef->BlockSize;
//		int32 chunkElementCount = WorldHandlerRef->ChunkElementCount;
//
//		FPaperTileInfo LocalTileInfo;
//		LocalTileInfo.TileSet = TileSet0;
//		LocalTileInfo.PackedTileIndex = 336; // DesiredBlockID;
//
//		int32 BlockToChangeX = FMath::Floor(HitLocation.X / blockSize);
//		int32 BlockToChangeZ = FMath::Floor(HitLocation.Z / blockSize);
//
//		ChunkData.BlockTextureID[BlockToChangeX + (BlockToChangeZ * chunkElementCount)] = 336;  //DesiredBlockID;
//		ChunkData.bHasCollision[BlockToChangeX + (BlockToChangeZ * chunkElementCount)] = 1;
//	}
//}
//
//#pragma endregion OnRep



// -------- SettersGetters --------
#pragma region SettersGetters

void AChunkActor::OnRep_ChunkDataChanged()
{
	RefreshChunk();
}

void AChunkActor::SetFChunkData(FChunkData data)
{
	ChunkData = data;
}

#pragma endregion SettersGetters

