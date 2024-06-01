// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldHandler.h"
#include "../Player/PlayerCharacter.h"
#include "ChunkActor.h"
#include "../Player/PlayerStatsData.h"
#include "WorldGenerator.h"
#include "../Framework/FileHandler.h"
#include "../Item/Item.h"
#include "../Framework/PixelGameInstance.h"


#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextRenderComponent.h"
#include "PaperTileMapComponent.h"
#include <Kismet/KismetMathLibrary.h>


// Sets default values
AWorldHandler::AWorldHandler()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//SetReplicates(true);
	bReplicates = true;
}

// Called when the game starts or when spawned
void AWorldHandler::BeginPlay()
{
	Super::BeginPlay();

	//initialize calculated parameters
	InitializeData();

	if (HasAuthority())
	{
		UPixelGameInstance* PixelGameInstance = Cast<UPixelGameInstance>(GetGameInstance());
		LoadWorldIn(PixelGameInstance->WorldName);
	}

	/*WorldGen->GenerateWorld(ChunkElementCount);
	RegionData = WorldGen->WorldData;*/

	//PlayerActorRef->WorldHandlerRef = this;
}

//Called every frame
void AWorldHandler::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWorldHandler::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(AWorldHandler, ChunkDataChange);
}

void AWorldHandler::Server_RegisterPlayerID_Implementation(APlayerCharacter* playerToRegister)
{
	if (HasAuthority())
	{
		if (PlayerIDs.Contains(playerToRegister))
		{
			return;
		}
		else
		{
			int32 playerIndex = ChunkCoordinatesShouldBeActiveByPlayers.Num();
			TArray<FIntPoint> NewPlayerValues;
			TPair<uint8, TArray<FIntPoint>> newEmpty(playerIndex, NewPlayerValues);
			ChunkCoordinatesShouldBeActiveByPlayers.Add(newEmpty);

			PlayerIDs.Add(playerToRegister, playerIndex);
			playerToRegister->SetPlayerID(playerIndex);
		}
	}
}

void AWorldHandler::InitializeData()
{
	//Manually set data... later on from file
	RenderRange = 3;
	ChunkElementCount = 16;
	BlockSize = 16;

	//Calculated data to initialize
	ChunkSize = ChunkElementCount * BlockSize;
	ChunkSizeHalf = ChunkSize / 2;
	ChunksCount = (RenderRange * 2 + 1) ^ 2;

	//Called data  to initialize
	ChunkActorTemplate = AChunkActor::StaticClass();
	fileHandler = NewObject<UFileHandler>(this, UFileHandler::StaticClass());
}

void AWorldHandler::LoadWorldIn(const FString& WorldName)
{
	fileHandler->LoadChunksToWorldData(WorldName, WorldData);
}

void AWorldHandler::LoadChunks(uint8 playerID, FIntPoint newChenterChunk)
{
	if (ChunkActorTemplate == nullptr)
	{
		return;
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		int32 playerIndex = -1;
		// find the player ID indey in the array
		for (int32 i = 0; i < ChunkCoordinatesShouldBeActiveByPlayers.Num(); ++i)
		{
			if (ChunkCoordinatesShouldBeActiveByPlayers[i].Key == playerID)
			{
				playerIndex = i;

				// Clear the existing array but keep the pair
				ChunkCoordinatesShouldBeActiveByPlayers[playerIndex].Value.Empty();

				break;
			}
		}

		for (int32 IndexX = -RenderRange; IndexX <= RenderRange; IndexX++)
		{
			for (int32 IndexZ = -RenderRange; IndexZ <= RenderRange; IndexZ++)
			{
				int32 ChunkCoordX = IndexX + newChenterChunk.X;
				int32 ChunkCoordZ = IndexZ + newChenterChunk.Y;

				ChunkCoordinatesShouldBeActiveByPlayers[playerIndex].Value.Add(FIntPoint(ChunkCoordX, ChunkCoordZ));

				// chunk already exists
				if (!IsChunkExists(ChunkCoordX, ChunkCoordZ))
				{

					float ChunkSpawnPositionX = (ChunkCoordX * ChunkSize) - ChunkSizeHalf;
					float ChunkSpawnPositionZ = (ChunkCoordZ * ChunkSize) + ChunkSizeHalf;


					FTransform SpawnTransform = FTransform(FVector(ChunkSpawnPositionX, 0.0f, ChunkSpawnPositionZ));

					AChunkActor* SpawnedActor = GetWorld()->SpawnActorDeferred<AChunkActor>(ChunkActorTemplate, SpawnTransform);
					if (SpawnedActor)
					{
						/*FChunkData* FoundChunkData = RegionData.FindByPredicate([&](const FChunkData& ChunkData)
							{ return ChunkData.ChunkCoordinate == FIntPoint(ChunkCoordX, ChunkCoordZ); });*/

						FChunkData* FoundChunkData = WorldData.Find(FIntPoint(ChunkCoordX, ChunkCoordZ));

						//FoundChunkData->WorldHandlerRef = this;

						//SpawnedActor->WorldHandlerRef = this;
						SpawnedActor->WorldHandlerRef = this;
						SpawnedActor->SetFChunkData(*FoundChunkData);
						//SpawnedActor->LoadChunk();

						if (FoundChunkData->Pickups.Num())
						{
							for (int32 i = 0; i < FoundChunkData->Pickups.Num(); i++)
							{
								SpawnPickup(FoundChunkData->Pickups[i]);
							}
						}

						// Finish spawning
						UGameplayStatics::FinishSpawningActor(SpawnedActor, SpawnTransform);
						
						

						ChunkActors.Add(FIntPoint(ChunkCoordX, ChunkCoordZ), SpawnedActor);
					}
				}
			}
		}

		TArray<FIntPoint> AllActiveChunkValues;

		// Iterate through the array
		for (const TPair<uint8, TArray<FIntPoint>>& Pair : ChunkCoordinatesShouldBeActiveByPlayers)
		{
			UE_LOG(LogTemp, Warning, TEXT("Appending %d elements"), Pair.Value.Num());
			AllActiveChunkValues.Append(Pair.Value);
		}

		for (auto It = ChunkActors.CreateIterator(); It; ++It)
		{
			if (!AllActiveChunkValues.Contains(It.Key()))
			{
				AChunkActor* ChunkActor = It.Value();
				if (ChunkActor)
				{
					ChunkActor->Destroy();
				}

				It.RemoveCurrent();
			}
		}
	}
	else
	{
		Server_LoadChunks(playerID, newChenterChunk);
	}
}

void AWorldHandler::Server_LoadChunks_Implementation(uint8 playerID, FIntPoint newChenterChunk)
{
	LoadChunks(playerID, newChenterChunk);
}


void AWorldHandler::SpawnPickup(FPickupData PickupData)
{
	if (!HasAuthority())
	{
		Server_SpawnPickup(PickupData);
		return;
	}

	if (HasAuthority())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.bNoFail = true;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		//FTransform SpawnTransform(GetActorRotation(), SpawnLocation);
		FTransform SpawnTransform(FRotator(0.0f, 0.0f, 0.0f), PickupData.Location);

		//ensure(PickupClass);

		if (APickup* Pickup = GetWorld()->SpawnActor<APickup>(PickupData.PickupType, SpawnTransform, SpawnParams))
		{
			Pickup->InitializePickup(PickupData.Item->GetClass(), PickupData.Item->GetQuantity());
		}
	}
}

uint8 AWorldHandler::IsChunkExists(const int32 X, const int32 Y)
{
	if (ChunkActors.Contains(FIntPoint(X, Y)))
	{
		return 1;
	}

	return 0;
}

void AWorldHandler::UpdateRegionData(TArray<FChunkChangeData> chunksToUpdate)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (chunksToUpdate.Num() > 0)
		{
			for (int32 idx = 0; idx < chunksToUpdate.Num(); idx++)
			{
				ChunkDataChange = chunksToUpdate;
				FIntPoint coord = chunksToUpdate[idx].ChunkCoordinate;

				//FChunkData& ChunkToUpdate = RegionData[FindChunkIndexByCoordinate(chunksToUpdate[idx].ChunkCoordinate)];
				FChunkData* ChunkToUpdate = WorldData.Find(coord);

				for (int32 blockID = 0; blockID < chunksToUpdate[idx].BlockIdx.Num(); blockID++)
				{
					ChunkToUpdate->BlockTextureID[chunksToUpdate[idx].BlockIdx[blockID]] = chunksToUpdate[idx].BlockTextureID[blockID];
					ChunkToUpdate->bHasCollision[chunksToUpdate[idx].BlockIdx[blockID]] = chunksToUpdate[idx].bHasCollision[blockID];
				}

				if (IsChunkExists(chunksToUpdate[idx].ChunkCoordinate.X, chunksToUpdate[idx].ChunkCoordinate.Y))
				{
					ChunkActors[coord]->RefreshChunkChange(chunksToUpdate[idx]);
					
				}
			}
		}
	}
}

//void AWorldHandler::UpdateRegionData(TArray<FChunkChangeData> chunksToUpdate)
//{
//	if (GetLocalRole() == ROLE_Authority)
//	{
//		if (chunksToUpdate.Num() > 0)
//		{
//			for (int32 idx = 0; idx < chunksToUpdate.Num(); idx++)
//			{
//				ChunkDataChange = chunksToUpdate;
//				FIntPoint coord = chunksToUpdate[idx].ChunkCoordinate;
//
//				//FChunkData& ChunkToUpdate = RegionData[FindChunkIndexByCoordinate(chunksToUpdate[idx].ChunkCoordinate)];
//				FChunkData* ChunkToUpdate = WorldData.Find(coord);
//
//				for (int32 blockID = 0; blockID < chunksToUpdate[idx].BlockIdx.Num(); blockID++)
//				{
//					ChunkToUpdate->BlockTextureID[chunksToUpdate[idx].BlockIdx[blockID]] = chunksToUpdate[idx].BlockTextureID[blockID];
//					ChunkToUpdate->bHasCollision[chunksToUpdate[idx].BlockIdx[blockID]] = chunksToUpdate[idx].bHasCollision[blockID];
//				}
//
//				if (IsChunkExists(chunksToUpdate[idx].ChunkCoordinate.X, chunksToUpdate[idx].ChunkCoordinate.Y))
//				{
//					ChunkActors[coord]->SetFChunkData(*ChunkToUpdate);
//					ChunkActors[coord]->RefreshChunk();
//				}
//			}
//		}
//	}
//}

//void AWorldHandler::OnRep_RegionDataChanged()
//{
//	UE_LOG(LogTemp, Warning, TEXT("OnRep_RegionDataChanged called"));
//	if (ChunkDataChange.Num() > 0)
//	{
//		for (int32 idx = 0; idx < ChunkDataChange.Num(); idx++)
//		{
//			FIntPoint coord = ChunkDataChange[idx].ChunkCoordinate;
//
//
//			FChunkData* ChunkToUpdate = WorldData.Find(coord);
//
//			//FChunkData& ChunkToUpdate = RegionData[FindChunkIndexByCoordinate(ChunkDataChange[idx].ChunkCoordinate)];
//			for (int32 blockID = 0; blockID < ChunkDataChange[idx].BlockIdx.Num(); blockID++)
//			{
//				ChunkToUpdate->BlockTextureID[ChunkDataChange[idx].BlockIdx[blockID]] = ChunkDataChange[idx].BlockTextureID[blockID];
//				ChunkToUpdate->bHasCollision[ChunkDataChange[idx].BlockIdx[blockID]] = ChunkDataChange[idx].bHasCollision[blockID];
//			}
//
//			if (IsChunkExists(ChunkDataChange[idx].ChunkCoordinate.X, ChunkDataChange[idx].ChunkCoordinate.Y))
//			{
//				FIntPoint coords = FIntPoint(ChunkDataChange[idx].ChunkCoordinate.X, ChunkDataChange[idx].ChunkCoordinate.Y);
//				AChunkActor* Chunk = *ChunkActors.Find(coord);
//					
//				Chunk->SetFChunkData(*ChunkToUpdate);
//				Chunk->RefreshChunk();
//			}
//		}
//	}
//}

/*OnRep_RegionDataChanged() !!!!*/ 
//void AWorldHandler::OnRep_RegionDataChanged()
//{
//	UE_LOG(LogTemp, Warning, TEXT("OnRep_RegionDataChanged called"));
//	if (ChunkDataChange.Num() > 0)
//	{
//		for (int32 idx = 0; idx < ChunkDataChange.Num(); idx++)
//		{
//			if (WorldData.Num() > 0)
//			{
//				FIntPoint coord = ChunkDataChange[idx].ChunkCoordinate;
//
//				//FChunkData& ChunkToUpdate = RegionData[FindChunkIndexByCoordinate(chunksToUpdate[idx].ChunkCoordinate)];
//				FChunkData* ChunkToUpdate = WorldData.Find(coord);
//
//				//FChunkData& ChunkToUpdate = RegionData[FindChunkIndexByCoordinate(ChunkDataChange[idx].ChunkCoordinate)];
//				for (int32 blockID = 0; blockID < ChunkDataChange[idx].BlockIdx.Num(); blockID++)
//				{
//					ChunkToUpdate->BlockTextureID[ChunkDataChange[idx].BlockIdx[blockID]] = ChunkDataChange[idx].BlockTextureID[blockID];
//					ChunkToUpdate->bHasCollision[ChunkDataChange[idx].BlockIdx[blockID]] = ChunkDataChange[idx].bHasCollision[blockID];
//				}
//
//				if (IsChunkExists(ChunkDataChange[idx].ChunkCoordinate.X, ChunkDataChange[idx].ChunkCoordinate.Y))
//				{
//					FIntPoint coords = FIntPoint(ChunkDataChange[idx].ChunkCoordinate.X, ChunkDataChange[idx].ChunkCoordinate.Y);
//					AChunkActor* Chunk = *ChunkActors.Find(coord);
//
//					Chunk->SetFChunkData(*ChunkToUpdate);
//					Chunk->RefreshChunk();
//				}
//			}
//		}
//	}
//}



//void AWorldHandler::Client_UpdateWorldData_Implementation(TArray<FChunkData> chunksToUpdate)
//{
//	for (int32 idx = 0; idx < chunksToUpdate.Num(); idx++)
//	{
//		WorldDATA[FindChunkIndexByCoordinate(chunksToUpdate[idx].ChunkCoordinate)] = chunksToUpdate[idx];
//		if (IsChunkExists(chunksToUpdate[idx].ChunkCoordinate.X, chunksToUpdate[idx].ChunkCoordinate.Y))
//		{
//			FIntPoint coords = FIntPoint(chunksToUpdate[idx].ChunkCoordinate.X, chunksToUpdate[idx].ChunkCoordinate.Y);
//			ChunksArray[FindActiveChunkIndexByCoordinate(coords)]->SetFChunkData(chunksToUpdate[idx]);
//		}
//	}
//}

//int32 AWorldHandler::FindChunkIndexByCoordinate(const FIntPoint& TargetCoordinate)
//{
//	for (int32 Index = 0; Index < RegionData.Num(); ++Index)
//	{
//		if (RegionData[Index].ChunkCoordinate == TargetCoordinate)
//		{
//			return Index;
//		}
//	}
//	return INDEX_NONE; // Return an invalid index if not found
//}

//int32 AWorldHandler::FindActiveChunkIndexByCoordinate(const FIntPoint& TargetCoordinate)
//{
//	for (int32 Index = 0; Index < RegionData.Num(); ++Index)
//	{
//		if (ChunkActors[Index]->ChunkData.ChunkCoordinate == TargetCoordinate)
//		{
//			return Index;
//		}
//	}
//	return INDEX_NONE; // Return an invalid index if not found
//}


//void AWorldHandler::GetFChunkDataByChunkCoordinate(FChunkData& desiredChunkData, const FIntPoint& TargetChunkCoordinate)
//{
//	const FChunkData* FoundChunkData = RegionData.FindByPredicate([&](const FChunkData& ChunkData) {
//		return ChunkData.ChunkCoordinate == TargetChunkCoordinate;
//		});
//
//	if ( FoundChunkData )
//	{
//		// Found the matching ChunkCoordinate, return the BlockTextureID
//		//return *FoundChunkData;
//		desiredChunkData = *FoundChunkData;
//	}
//
//	// If the function reaches here, the ChunkCoordinate was not found
//	// You may want to handle this case accordingly (e.g., return an empty array)
//	/*FChunkData empty;
//	return empty;*/
//}

void AWorldHandler::Server_SpawnPickup_Implementation(FPickupData PickupData)
{
	SpawnPickup(PickupData);
}

bool AWorldHandler::Server_SpawnPickup_Validate(FPickupData PickupData)
{
	return true;
}