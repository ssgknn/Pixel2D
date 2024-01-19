// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkActor.h"
#include "Components/TextRenderComponent.h"
#include "PaperTileMapComponent.h"
#include "PaperTileSet.h"

// Sets default values
AChunkActor::AChunkActor(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TileMapComponent = CreateDefaultSubobject<UPaperTileMapComponent>(TEXT("TileMap"));


	// Initialize TileMapComponents (chunks)
	static ConstructorHelpers::FObjectFinder<UPaperTileSet> TileSetObject(TEXT("/Game/2DAssets/Assets/TS_Placeholder0"));
	TileSet0 = TileSetObject.Object;

}

// Called when the game starts or when spawned
void AChunkActor::BeginPlay()
{
	Super::BeginPlay();
	LoadChunk();
}

// Called every frame
void AChunkActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AChunkActor::LoadChunk()
{	
	TileMapComponent->CreateNewTileMap(ChunkElementCount, ChunkElementCount, BlockSize, BlockSize, 1.0, true);
	for (int32 y = 0; y < ChunkElementCount; y++)
	{
		for (int32 x = 0; x < ChunkElementCount; x++)
		{
			FPaperTileInfo LocalTileInfo;
			LocalTileInfo.TileSet = TileSet0;
			if (FMath::RandRange(0, 624) < 312)
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
}

bool AChunkActor::ModifyBlock(FVector2d BlockToModify, int DesiredBlock)
{
	FPaperTileInfo LocalTileInfo;
	LocalTileInfo.TileSet = TileSet0;
	LocalTileInfo.PackedTileIndex = DesiredBlock;
	TileMapComponent->SetTile(BlockToModify.X, BlockToModify.Y, 0, LocalTileInfo);
	return true;
}


