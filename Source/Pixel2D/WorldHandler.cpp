// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldHandler.h"
#include "Kismet/GameplayStatics.h"
#include "ZDPlayerCharacterBase.h"

// Sets default values
AWorldHandler::AWorldHandler()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Chunks size
	ChunkSize = 64;

	//Starter chunk boundary values, *** later on switch to chunk number ***
	Boundary_L = 0;
	Boundary_R = 0;
	Boundary_U = 0;
	Boundary_D = 0;

	//TileSetName1 = TEXT("TS_Placeholder0");

	// Initialize TileMapComponents (chunks)
	TileMapComponent.SetNum(25);
	for (int32 i = 0; i < 25; ++i)
	{
		TileMapComponent[i] = CreateDefaultSubobject<UPaperTileMapComponent>(FName("TileMapComponent_" + FString::FromInt(i)));
	}

	this->SetActorLocation(FVector( - (ChunkSize / 2), 1, ChunkSize / 2));

	//TileMapComponent->SetRelativeLocation(FVector(0, 0, 0));
	//TileMapComponent->RegisterComponent();
}

// Called when the game starts or when spawned
void AWorldHandler::BeginPlay()
{
	Super::BeginPlay();

	//PlayerCharacter reference
	CharacterRef = Cast<AZDPlayerCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	LoadChunks();
}

void AWorldHandler::LoadChunks()
{
	for (int32 i = 0; i < 25; ++i)
	{
		TileMapComponent[i]->CreateNewTileMap(ChunkSize, ChunkSize, 16, 16, 1.0, true);
	}

	FVector chunkLocation;

	//Old manual spawn method
	/*
	chunkLocation.Set(-(ChunkSize * 16) * 1.5, 1, (ChunkSize * 16) * 1.5);
	TileMapComponent[0]->SetRelativeLocation(chunkLocation);

	chunkLocation.Set(-(ChunkSize * 16) / 2, 1, (ChunkSize * 16) * 1.5);
	TileMapComponent[1]->SetRelativeLocation(chunkLocation);

	chunkLocation.Set((ChunkSize * 16) / 2, 1, (ChunkSize * 16) * 1.5);
	TileMapComponent[2]->SetRelativeLocation(chunkLocation);

	chunkLocation.Set(-(ChunkSize * 16) * 1.5, 1, (ChunkSize * 16) / 2);
	TileMapComponent[3]->SetRelativeLocation(chunkLocation);

	chunkLocation.Set(-(ChunkSize * 16) / 2, 1, (ChunkSize * 16) / 2);
	TileMapComponent[4]->SetRelativeLocation(chunkLocation);

	chunkLocation.Set((ChunkSize * 16) / 2, 1, (ChunkSize * 16) / 2);
	TileMapComponent[5]->SetRelativeLocation(chunkLocation);

	chunkLocation.Set(-(ChunkSize * 16) * 1.5, 1, -(ChunkSize * 16) / 2);
	TileMapComponent[6]->SetRelativeLocation(chunkLocation);

	chunkLocation.Set(-(ChunkSize * 16) / 2, 1, -(ChunkSize * 16) / 2);
	TileMapComponent[7]->SetRelativeLocation(chunkLocation);

	chunkLocation.Set((ChunkSize * 16) / 2, 1, -(ChunkSize * 16) / 2);
	TileMapComponent[8]->SetRelativeLocation(chunkLocation);
	*/

	{
		int idx = 0;
		for (int32 i = -2; i < 2; i++)
		{
			for (int32 y = -2; y < 2; y++)
			{
				chunkLocation.Set(i * ChunkSize * 16 / 2, 1, y * ChunkSize * 16 / 2);
				TileMapComponent[idx]->SetRelativeLocation(chunkLocation);
				idx++;
			}
		}
	}
	

	for (int32 i = 0; i < 25; i++)
	{
		for (int32 y = 0; y < ChunkSize; y++)
		{
			for (int32 x = 0; x < ChunkSize; x++)
			{
				FPaperTileInfo LocalTileInfo;
				LocalTileInfo.TileSet = TileSet1;
				if (i > 5) {
					LocalTileInfo.PackedTileIndex = 53;
				}
				else
				{
					LocalTileInfo.PackedTileIndex = 53; //FMath::RandRange(0, 624);
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

	//chunkLocation.Set(-(ChunkSize * 16) * 2.5, 1, (ChunkSize * 16) * 2.5);
	//TileMapComponent[0]->SetRelativeLocation(chunkLocation);

	//chunkLocation.Set(-(ChunkSize * 16) / 2, 1, (ChunkSize * 16) * 2.5);
	//TileMapComponent[1]->SetRelativeLocation(chunkLocation);

	//chunkLocation.Set((ChunkSize * 16) * 2.5, 1, (ChunkSize * 16) * 2.5);
	//TileMapComponent[2]->SetRelativeLocation(chunkLocation);

	//chunkLocation.Set(-(ChunkSize * 16) * 2.5, 1, (ChunkSize * 16) / 2);
	//TileMapComponent[3]->SetRelativeLocation(chunkLocation);

	//chunkLocation.Set(-(ChunkSize * 16) / 2, 1, (ChunkSize * 16) / 2);
	//TileMapComponent[4]->SetRelativeLocation(chunkLocation);

	//chunkLocation.Set((ChunkSize * 16) * 2.5, 1, (ChunkSize * 16) / 2);
	//TileMapComponent[5]->SetRelativeLocation(chunkLocation);

	//chunkLocation.Set(-(ChunkSize * 16) * 2.5, 1, -(ChunkSize * 16) * 2.5);
	//TileMapComponent[6]->SetRelativeLocation(chunkLocation);

	//chunkLocation.Set(-(ChunkSize * 16) / 2, 1, -(ChunkSize * 16) * 2.5);
	//TileMapComponent[7]->SetRelativeLocation(chunkLocation);

	//chunkLocation.Set((ChunkSize * 16) * 2.5, 1, -(ChunkSize * 16) * 2.5);
	//TileMapComponent[8]->SetRelativeLocation(chunkLocation);

	//for (int32 i = 0; i < 9; i++)
	//{
	//	for (int32 y = 0; y < ChunkSize; y++)
	//	{
	//		for (int32 x = 0; x < ChunkSize; x++)
	//		{
	//			FPaperTileInfo LocalTileInfo;
	//			LocalTileInfo.TileSet = TileSet1;
	//			if (i > 5) {
	//				LocalTileInfo.PackedTileIndex = 53;
	//			}
	//			else
	//			{
	//				LocalTileInfo.PackedTileIndex = 454; // FMath::RandRange(0, 200);
	//			}

	//			TileMapComponent[i]->SetTile(x, y, 0, LocalTileInfo);
	//		}
	//	}
	//}
}

void AWorldHandler::RemoveBlockByIndex(int32 index)
{
}

void AWorldHandler::AddBlockByVector(FVector& vector)
{
}


 //Called every frame
void AWorldHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (CharacterRef)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 0.3f, FColor::Red, CharacterRef->GetActorLocation().ToString());
		CharacterPosition = CharacterRef->GetActorLocation();
		if (CharacterPosition.X < Boundary_L)
		{
			RefreshChunks(0);
		}
		if (CharacterPosition.X > Boundary_R)
		{
			RefreshChunks(1);
		}
		if (CharacterPosition.Z > Boundary_U)
		{
			RefreshChunks(2);
		}
		if (CharacterPosition.Z < Boundary_D)
		{
			RefreshChunks(3);
		}
		
	}
	else
	{
		//GEngine->AddOnScreenDebugMessage(-1, 0.3f, FColor::Red, ":(");
	}

}
