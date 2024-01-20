// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkActor.h"
#include "Components/TextRenderComponent.h"
#include "PaperTileMapComponent.h"
#include "PaperTileMap.h"
#include "PaperTileSet.h"
#include "Components/BoxComponent.h"


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

	//AttachComponent->SetRelativeLocation(FVector(BlockSize / 2.0f, 0.0f, -BlockSize / 2.0f));
	
	TileMapComponent->CreateNewTileMap(ChunkElementCount, ChunkElementCount, BlockSize, BlockSize, 1.0, true);
	TileMapComponent->SetRelativeLocation(FVector(BlockSize / 2.0f, 0.0f, -BlockSize / 2.0f));
	for (int32 y = 0; y < ChunkElementCount; y++)
	{
		for (int32 x = 0; x < ChunkElementCount; x++)
		{
			FPaperTileInfo LocalTileInfo;
			LocalTileInfo.TileSet = TileSet0;
			if (FMath::RandRange(0, 100) < 25)
			{
				LocalTileInfo.PackedTileIndex = 16;
			}
			else
			{
				LocalTileInfo.PackedTileIndex = 32;
			}

			TileMapComponent->SetTile(x, y, 0, LocalTileInfo);
		}
	}

	TileMapComponent->SetDefaultCollisionThickness(100, true);
	

	//CollisionBoxComponent->SetRelativeLocation(FVector(ChunkSize/2, 0, -ChunkSize/2));
	//CollisionBoxComponent->SetRelativeScale3D(FVector(ChunkSize/100.0f, 1, ChunkSize/100.0f));
	//CollisionBoxComponent->SetWorldScale3D(TileMapComponent->GetComponentScale());
	CollisionBoxComponent->SetBoxExtent(FVector(ChunkSize * 0.5f, 20.0f, ChunkSize * 0.5f));
	CollisionBoxComponent->SetRelativeLocation(FVector(ChunkSize * 0.5f, 10.0f, -ChunkSize * 0.5f));
}

void AChunkActor::ModifyBlock(FVector HitLocation, int32 DesiredBlockID)
{
	GEngine->AddOnScreenDebugMessage(-1, 0.3f, FColor::Red, TEXT("AChunkActor::ModifyBlock()"));
	FPaperTileInfo LocalTileInfo;
	LocalTileInfo.TileSet = TileSet0;
	LocalTileInfo.PackedTileIndex = DesiredBlockID;

	int32 BlockToChangeX = FMath::Floor(HitLocation.X / BlockSize);
	int32 BlockToChangeZ = FMath::Floor(HitLocation.Z / BlockSize);

	// neighbor block check if can place block or not, cant do without tile matrix. Should check for neighbor chunk data...
	/*TArray<FInt32Vector2> NeighborBlocks;
	NeighborBlocks.SetNum(4);
	NeighborBlocks[0] = FInt32Vector2();*/

	/*GEngine->AddOnScreenDebugMessage(-1, 0.3f, FColor::Red, FString::Printf(TEXT("B_X: %d"), BlockToChangeX));
	GEngine->AddOnScreenDebugMessage(-1, 0.3f, FColor::Red, FString::Printf(TEXT("B_Z: %d"), BlockToChangeZ));*/

	TileMapComponent->SetTile(BlockToChangeX, BlockToChangeZ, 0, LocalTileInfo);
}


