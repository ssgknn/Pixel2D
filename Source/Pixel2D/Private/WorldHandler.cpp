// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldHandler.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextRenderComponent.h"
#include "PaperTileMapComponent.h"
#include "ZDPlayerCharacterBase.h"

// Sets default values
AWorldHandler::AWorldHandler()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Chunks size
	ChunkElementCount = 64;
	ChunkSize = ChunkElementCount * BlockSize;
	ChunkSizeHalf = ChunkSize / 2;
	ChunksCount = 25;

	// Initialize TileMapComponents (chunks)
	TileMapComponent.SetNum(ChunksCount);
	TextComponent.SetNum(ChunksCount);
	for (int32 i = 0; i < ChunksCount; ++i)
	{
		TileMapComponent[i] = CreateDefaultSubobject<UPaperTileMapComponent>(FName("TileMapComponent_" + FString::FromInt(i)));
		TextComponent[i] = CreateDefaultSubobject<UTextRenderComponent>(FName("TextComponent_" + FString::FromInt(i)));
	}

	this->SetActorLocation(FVector( - (ChunkSizeHalf), 1, ChunkSizeHalf));

	PlayerPosition = FVector(0.0, 100.0, 0.0);
}

// Called when the game starts or when spawned
void AWorldHandler::BeginPlay()
{
	Super::BeginPlay();

	//PlayerCharacter reference
	CharacterRef = Cast<AZDPlayerCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("WorldHandler"));

	LoadChunks();
}

//Called every frame
void AWorldHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CharacterRef)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 0.3f, FColor::Red, CharacterRef->GetActorLocation().ToString());
		PlayerPosition = CharacterRef->GetActorLocation();
	}

}


void AWorldHandler::LoadChunks()
{
	for (int32 i = 0; i < ChunksCount; ++i)
	{
		TileMapComponent[i]->CreateNewTileMap(ChunkElementCount, ChunkElementCount, 16, 16, 1.0, true);
	}

	FVector chunkLocation;
	int idx = 0;
	for (int32 i = -2; i < 3; i++)
	{
		for (int32 y = -2; y < 3; y++)
		{
			chunkLocation.Set(i * ChunkElementCount * 16, 1, y * ChunkElementCount * 16);
			TileMapComponent[idx]->SetRelativeLocation(chunkLocation);

			TextComponent[idx]->SetText(FText::FromString(FString::Printf(TEXT("%d"), idx)));
			TextComponent[idx]->SetRelativeLocation(chunkLocation);

			idx++;
		}
	}
	
	

	for (int32 i = 0; i < ChunksCount; i++)
	{
		for (int32 y = 0; y < ChunkElementCount; y++)
		{
			for (int32 x = 0; x < ChunkElementCount; x++)
			{
				FPaperTileInfo LocalTileInfo;
				LocalTileInfo.TileSet = TileSet1;
				if (i > 5) {
					LocalTileInfo.PackedTileIndex = 53;
				}
				else
				{
					LocalTileInfo.PackedTileIndex = FMath::RandRange(0, 624);
				}

				TileMapComponent[i]->SetTile(x, y, 0, LocalTileInfo);
			}
		}
		TileMapComponent[i]->SetDefaultCollisionThickness(100, true);
		//TileMapComponent[i]->RebuildCollision();
	}
}

void AWorldHandler::RefreshChunks(int direction)
{
	FVector chunkLocation;

	if (direction == 0)
	{

	}
}

void AWorldHandler::CheckChunkBoundary()
{
	if (PlayerPosition.X < ChunksCenter.X - ChunkSizeHalf )
	{
		// left the Center in L
		RefreshChunks(0);

	}
	if (PlayerPosition.X > ChunksCenter.X + ChunkSizeHalf)
	{
		// left the Center in R
		RefreshChunks(1);
	}
	if (PlayerPosition.Z < ChunksCenter.Z - ChunkSizeHalf)
	{
		// left the Center in D
		RefreshChunks(2);
	}
	if (PlayerPosition.Z > ChunksCenter.Z + ChunkSizeHalf)
	{
		// left the Center in U
		RefreshChunks(3);
	}
}

void AWorldHandler::RemoveBlockByIndex(int32 index)
{
}

void AWorldHandler::AddBlockByVector(FVector& vector)
{
}