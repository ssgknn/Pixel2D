// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldHandler.h"
#include "ZDPlayerCharacterBase.h"
#include "ChunkActor.h"
#include "PlayerStatsData.h"
#include "WorldGenerator.h"
#include "FileHandler.h"
#include "Item.h"

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
	RefreshChunks();
}

void AWorldHandler::InitializeData()
{
	//Manually set data... later on from file
	RenderRange = 1;
	ChunkElementCount = 32;
	BlockSize = 16;
	CenterChunkCoords = FIntPoint(0, 0);
	bFirstLaunch = 1;


	//Calculated data to initialize
	ChunkSize = ChunkElementCount * BlockSize;
	ChunkSizeHalf = ChunkSize / 2;
	ChunksCount = (RenderRange * 2 + 1) ^ 2;
	ChunkCoordinatesShouldBeActive.SetNum(ChunksCount);


	//Called data  to initialize
	ChunkActorTemplate = AChunkActor::StaticClass();
	WorldGen = NewObject<UWorldGenerator>(this, UWorldGenerator::StaticClass());
	FileHandler = NewObject<UFileHandler>(this, UFileHandler::StaticClass());
	PlayerActorRef = Cast<AZDPlayerCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

void AWorldHandler::AddChunks()
{
	ChunkCoordinatesShouldBeActive.Empty();

	if (ChunkActorTemplate == nullptr)
	{
		return;
	}
	for (int32 IndexX = -RenderRange; IndexX <= RenderRange; IndexX++)
	{
		for (int32 IndexZ = -RenderRange; IndexZ <= RenderRange; IndexZ++)
		{
			int32 ChunkCoordX = IndexX + CenterChunkCoords.X;
			int32 ChunkCoordZ = IndexZ + CenterChunkCoords.Y;

			ChunkCoordinatesShouldBeActive.Add(FIntPoint(ChunkCoordX, ChunkCoordZ));

			// chunk already exists
			if ( !IsChunkExists(ChunkCoordX, ChunkCoordZ) )
			{
				ActiveChunkCoordinates.Add(FIntPoint(ChunkCoordX, ChunkCoordZ));

				//GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Green, FString::Printf(TEXT("NewX: %i NewZ: %i"), ChunkCoordX, ChunkCoordZ));

				float ChunkSpawnPositionX = (ChunkCoordX * ChunkSize) - ChunkSizeHalf;
				float ChunkSpawnPositionZ = (ChunkCoordZ * ChunkSize) + ChunkSizeHalf;


				FTransform SpawnTransform = FTransform(FVector(ChunkSpawnPositionX, 0.0f, ChunkSpawnPositionZ));

				AChunkActor* SpawnedActor = GetWorld()->SpawnActorDeferred<AChunkActor>(ChunkActorTemplate, SpawnTransform);

				FChunkData* FoundChunkData = WorldDATA.FindByPredicate([&](const FChunkData& ChunkData)
					{ return ChunkData.ChunkCoordinate == FIntPoint(ChunkCoordX, ChunkCoordZ); });

				SpawnedActor->WorldHandlerRef = this;
				SpawnedActor->SetFChunkData(*FoundChunkData);
				SpawnedActor->LoadChunk();

				if (FoundChunkData->Pickups.Num())
				{
					for (int32 i = 0; i < FoundChunkData->Pickups.Num(); i++)
					{
						SpawnPickup(FoundChunkData->Pickups[i]);
					}
				}
				
				FString FormattedText = FString::Printf(TEXT("(X%i, Y%i)"), ChunkCoordX, ChunkCoordZ);
				//SpawnedActor->TextComponent->SetText(FText::FromString(FormattedText));
				ChunksArray.Add(SpawnedActor);
			}
		}
	}
}

void AWorldHandler::RemoveChunks()
{
	for (int32 Index = 0; Index < ActiveChunkCoordinates.Num(); Index++)
	{
		if ( ChunkCoordinatesShouldBeActive.Find(ActiveChunkCoordinates[Index]) == INDEX_NONE )
		{
			ActiveChunkCoordinates.RemoveAt(Index, 1, false);
			ChunksArray[Index]->Destroy();
			ChunksArray.RemoveAt(Index, 1, false);
		}
	}

	ActiveChunkCoordinates.Shrink();
	ChunksArray.Shrink();
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
	if ( ActiveChunkCoordinates.Find(FIntPoint(X, Y)) == INDEX_NONE )
	{
		return 0;
	}

	return 1;
}

void AWorldHandler::UpdatePlayerPosition()
{
	if ( PlayerActorRef )
	{
		PlayerPosition = PlayerActorRef->GetActorLocation();
	}
}

void AWorldHandler::RefreshChunks()
{
	//const FVector PlayerDistanceFromChunkCenter = ChunkCenterPosition - PlayerPosition;
	//if (FMath::Abs(PlayerDistanceFromChunkCenter.X) > ChunkSize || FMath::Abs(PlayerDistanceFromChunkCenter.Z) > ChunkSize)
	if ( PlayerActorRef && GetChunkCoordPlayerAt() != CenterChunkCoords )
	{
		CenterChunkCoords = GetChunkCoordPlayerAt();
		AddChunks();
		RemoveChunks();
		return;
	}
	if ( bFirstLaunch )
	{
		bFirstLaunch = 0;
		AddChunks();
		return;
	}
}

FIntPoint AWorldHandler::GetChunkCoordPlayerAt()
{
	if ( PlayerActorRef && ChunkSize > 0 )
	{
		FIntPoint toReturn = FIntPoint(FMath::Floor((PlayerActorRef->GetActorLocation().X - ChunkSizeHalf) / ChunkSize) + 1, FMath::Floor((PlayerActorRef->GetActorLocation().Z + ChunkSizeHalf) / ChunkSize));
		return toReturn;
	}
	return FIntPoint(0, 0);
}

FChunkData AWorldHandler::GetFChunkDataByChunkCoordinate(const FIntPoint& TargetChunkCoordinate)
{
	const FChunkData* FoundChunkData = WorldDATA.FindByPredicate([&](const FChunkData& ChunkData) {
		return ChunkData.ChunkCoordinate == TargetChunkCoordinate;
		});

	if ( FoundChunkData )
	{
		// Found the matching ChunkCoordinate, return the BlockTextureID
		return *FoundChunkData;
	}

	// If the function reaches here, the ChunkCoordinate was not found
	// You may want to handle this case accordingly (e.g., return an empty array)
	FChunkData empty;
	return empty;
}

// -------- RPC --------
// 

void AWorldHandler::Server_SpawnPickup_Implementation(FPickupData PickupData)
{
	SpawnPickup(PickupData);
}

bool AWorldHandler::Server_SpawnPickup_Validate(FPickupData PickupData)
{
	return true;
}