// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldHandler.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextRenderComponent.h"
#include "PaperTileMapComponent.h"
#include "ZDPlayerCharacterBase.h"
#include "ChunkActor.h"
#include <Kismet/KismetMathLibrary.h>



// Sets default values
AWorldHandler::AWorldHandler()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	

	// Initialize TileMapComponents (chunks)
	/*TileMapComponent.SetNum(ChunksCount);
	TextComponent.SetNum(ChunksCount);
	for (int32 i = 0; i < ChunksCount; ++i)
	{
		TileMapComponent[i] = CreateDefaultSubobject<UPaperTileMapComponent>(FName("TileMapComponent_" + FString::FromInt(i)));
		TextComponent[i] = CreateDefaultSubobject<UTextRenderComponent>(FName("TextComponent_" + FString::FromInt(i)));
	}

	this->SetActorLocation(FVector( - (ChunkSizeHalf), 1, ChunkSizeHalf));*/

	PlayerPosition = FVector(0.0, 100.0, 0.0);
}

// Called when the game starts or when spawned
void AWorldHandler::BeginPlay()
{
	Super::BeginPlay();

	ChunkActorTemplate = AChunkActor::StaticClass();

	//initialize calculated parameters
	ChunkSize = ChunkElementCount * BlockSize;
	ChunkSizeHalf = ChunkSize / 2;
	ChunksCount = (RenderRange * 2 + 1) ^ 2;

	//PlayerCharacter reference
	CharacterRef = Cast<AZDPlayerCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

//Called every frame
void AWorldHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdatePlayerPosition();
	RemoveChunks();
	AddChunks();

}

void AWorldHandler::AddChunks()
{
	if (ChunkActorTemplate == nullptr)
	{
		return;
	}

	for (int32 IndexX = -RenderRange; IndexX <= RenderRange; IndexX++)
	{
		for (int32 IndexZ = -RenderRange; IndexZ <= RenderRange; IndexZ++)
		{
			const int32 GlobalX = IndexX + ChunkX;
			const int32 GlobalZ = IndexZ + ChunkZ;

			// chunk already exists
			if (AChunkActor* Chunk = FindChunk(GlobalX, GlobalZ))
			{
				//
			}
			else
			{
				float ChunkPositionX = GlobalX * ChunkSize;
				float ChunkPositionZ = GlobalZ * ChunkSize;
				float XCenter = ChunkPositionX + ChunkSizeHalf;
				float YCenter = ChunkPositionZ + ChunkSizeHalf;

				ChunkCoordinates.Add(FIntPoint(GlobalX, GlobalZ));

				FTransform SpawnTransform = FTransform(FVector(ChunkPositionX, 0.0f, ChunkPositionZ));

				AChunkActor* SpawnedActor = GetWorld()->SpawnActorDeferred<AChunkActor>(ChunkActorTemplate, SpawnTransform);

				SpawnedActor->BlockSize = BlockSize;
				SpawnedActor->ChunkElementCount = ChunkElementCount;
				SpawnedActor->LoadChunk();
				/* Call ChunkActor variables...
				* SpawnedActor->
				* SpawnedActor-> ...
				* 
				*/
				
				ChunksArray.Add(SpawnedActor);
			}
		}
	}
}

AChunkActor* AWorldHandler::FindChunk(const int32 X, const int32 Y)
{
	int32 ChunkIndex = ChunkCoordinates.Find(FIntPoint(X, Y));
	if (ChunkIndex != -1)
	{
		return ChunksArray[ChunkIndex];
	}

	return nullptr;
}

void AWorldHandler::RefreshChunks(int direction)
{
	FVector chunkLocation;

	if (direction == 0)
	{

	}
}

void AWorldHandler::RemoveBlockByIndex(int32 index)
{
}

void AWorldHandler::AddBlockByVector(FVector& vector)
{
}

FVector AWorldHandler::GetHalfVoxelSize()
{
	float VoxelSizeHalf = ChunkSize / 2;
	return FVector(VoxelSizeHalf, VoxelSizeHalf, VoxelSizeHalf);
}

FIntPoint AWorldHandler::GetChunkCords(const FVector LocalPosition)
{
	FVector HalvVoxelSize = GetHalfVoxelSize();
	FVector Pos = (HalvVoxelSize + LocalPosition) / ChunkSize;
	int32 X = UKismetMathLibrary::FFloor(Pos.X);
	int32 Z = UKismetMathLibrary::FFloor(Pos.Z);

	return FIntPoint(X, Z);
}

void AWorldHandler::UpdatePlayerPosition()
{
	//update PlayerPosition
	if (CharacterRef)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 0.3f, FColor::Red, CharacterRef->GetActorLocation().ToString());
		PlayerPosition = CharacterRef->GetActorLocation();
		//PlayerPosition = UGameplayStatics::GetPlayerPawn(GetOwner(), 0)->GetActorLocation();
		ChunkX = PlayerPosition.X / ChunkSize;
		ChunkZ = PlayerPosition.Z / ChunkSize;
	}
}

void AWorldHandler::RemoveChunks()
{
	for (int32 Index = 0; Index < ChunkCoordinates.Num(); Index++)
	{
		FIntPoint Vect = ChunkCoordinates[Index];
		float ChunkPositionX = Vect.X * ChunkSize;
		float ChunkPositionZ = Vect.Y * ChunkSize;
		bool InRanderRange = CheckRadius(ChunkPositionX + ChunkSizeHalf, ChunkPositionZ + ChunkSizeHalf);

		if (!InRanderRange)
		{
			ChunkCoordinates.RemoveAt(Index, 1, false);
			ChunksArray[Index]->Destroy();
			ChunksArray.RemoveAt(Index, 1, false);
		}
	}

	ChunkCoordinates.Shrink();
	ChunksArray.Shrink();
}

bool AWorldHandler::CheckRadius(const float CenterX, const float CenterZ)
{
	float RenderDistnace = (FVector(CenterX - PlayerPosition.X, CenterZ - PlayerPosition.Z, 0.0f)).Size();

	return RenderDistnace < (ChunkSize * RenderRange);
}
