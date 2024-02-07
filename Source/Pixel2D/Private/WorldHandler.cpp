// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldHandler.h"
#include "ZDPlayerCharacterBase.h"
#include "ChunkActor.h"
#include "PlayerStatsData.h"
#include "WorldGenerator.h"
#include "FileHandler.h"

#include "Kismet/GameplayStatics.h"
#include "Components/TextRenderComponent.h"
#include "PaperTileMapComponent.h"
#include <Kismet/KismetMathLibrary.h>


// Sets default values
AWorldHandler::AWorldHandler()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AWorldHandler::BeginPlay()
{
	Super::BeginPlay();

	//initialize calculated parameters
	InitializeData();

	WorldGen->GenerateWorld(ChunkElementCount);
	WorldDATA = WorldGen->WorldData;
}

//Called every frame
void AWorldHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdatePlayerPosition();
	CheckChunkLoads();
	AddChunks();
}

void AWorldHandler::InitializeData()
{
	//Manually set data... later on from file
	RenderRange = 2;
	ChunkElementCount = 32;
	BlockSize = 16;
	ChunkX = 0;
	ChunkZ = 0;
	ChunkCenterPosition = FVector(0.0f, 0.0f, 0.0f);

	//Calculated data to initialize
	ChunkSize = ChunkElementCount * BlockSize;
	ChunkSizeHalf = ChunkSize / 2;
	ChunksCount = (RenderRange * 2 + 1) ^ 2;

	//Called data  to initialize
	ChunkActorTemplate = AChunkActor::StaticClass();
	WorldGen = NewObject<UWorldGenerator>(this, UWorldGenerator::StaticClass());
	FileHandler = NewObject<UFileHandler>(this, UFileHandler::StaticClass());
	PlayerActorRef = Cast<AZDPlayerCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
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
				return;
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

				FChunkData* FoundChunkData = WorldDATA.FindByPredicate([&](const FChunkData& ChunkData) 
												{ return ChunkData.ChunkCoordinate == FIntPoint(GlobalX, GlobalZ); });

				SpawnedActor->WorldHandlerRef = this;
				SpawnedActor->SetFChunkData(*FoundChunkData);
				SpawnedActor->LoadChunk(GlobalX, GlobalZ);

				//SpawnedActor->RefreshCollision(BlockSize, ChunkElementCount);

				/* Call ChunkActor variables...
				*  SpawnedActor->
				*  SpawnedActor-> ...
				*  SpawnedActor->BlockSize = BlockSize;
				*  SpawnedActor->ChunkElementCount = ChunkElementCount;
				*  SpawnedActor->ChunkSize = ChunkSize;
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
	if (PlayerActorRef)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 0.3f, FColor::Red, CharacterRef->GetActorLocation().ToString());
		PlayerPosition = PlayerActorRef->GetActorLocation();
		//PlayerPosition = UGameplayStatics::GetPlayerPawn(GetOwner(), 0)->GetActorLocation();
		ChunkX = PlayerPosition.X / ChunkSize;
		ChunkZ = PlayerPosition.Z / ChunkSize;
	}
}

void AWorldHandler::RemoveChunks()
{
	//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Red, TEXT("WorldHandler::RemoveChunks()"));
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

void AWorldHandler::CheckChunkLoads()
{
	const FVector PlayerDistanceFromChunkCenter = ChunkCenterPosition - PlayerPosition;

	if (FMath::Abs(PlayerDistanceFromChunkCenter.X) > ChunkSize || FMath::Abs(PlayerDistanceFromChunkCenter.Z) > ChunkSize)
	{
		ChunkCenterPosition = PlayerPosition;
		RemoveChunks();
	}
}

FChunkData AWorldHandler::GetFChunkDataByChunkCoordinate(const FIntPoint& TargetChunkCoordinate)
{
	const FChunkData* FoundChunkData = WorldDATA.FindByPredicate([&](const FChunkData& ChunkData) {
		return ChunkData.ChunkCoordinate == TargetChunkCoordinate;
		});

	if (FoundChunkData)
	{
		// Found the matching ChunkCoordinate, return the BlockTextureID
		return *FoundChunkData;
	}

	// If the function reaches here, the ChunkCoordinate was not found
	// You may want to handle this case accordingly (e.g., return an empty array)
	FChunkData empty;
	return empty;
}