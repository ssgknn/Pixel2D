// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldGenerator.h"

#include "../Framework/FileHandler.h"


UWorldGenerator::UWorldGenerator()
{
	// Assuming UFileHandler is a UObject, use NewObject to create an instance
	FileHandler = NewObject<UFileHandler>();
}

void UWorldGenerator::GenerateWorld(int32 ChunkElementCount)
{
	int32 GenerationRange = 100;

	FChunkData chunkData;

	//TArray<FUint32Vector2> blockCoordsInChunk;
	TArray<int32> blockTextureID;
	TArray<uint8> bCollisionValue;

	blockTextureID.SetNum(ChunkElementCount * ChunkElementCount);
	bCollisionValue.SetNum(ChunkElementCount * ChunkElementCount);


	for (int32 GlobalX = -GenerationRange; GlobalX <= GenerationRange; GlobalX++)
	{
		for (int32 GlobalZ = -GenerationRange; GlobalZ <= GenerationRange; GlobalZ++)
		{
			blockTextureID.Empty();
			bCollisionValue.Empty();

			for (int32 x = 0; x < ChunkElementCount; x++)
			{
				for (int32 z = 0; z < ChunkElementCount; z++)
				{
					//blockCoordsInChunk.Add(FUint32Vector2(x, z));
					if ( GlobalZ > 0 )
					{
						blockTextureID.Add(5);
						bCollisionValue.Add(0);
					}
					/*else if (GlobalZ == 0)
					{
						blockTextureID.Add(53);
						bCollisionValue.Add(1);
					}
					else if (GlobalZ == -1)
					{
						blockTextureID.Add(58);
						bCollisionValue.Add(1);
					}*/
					else if (GlobalZ <= 0)
					{
						/*if ( FMath::RandRange(0, 100) < 90 )
						{*/
							blockTextureID.Add(176);
							bCollisionValue.Add(1);
						/*}
						else
						{
							blockTextureID.Add(5);
							bCollisionValue.Add(0);
						}*/
						
					}
				}
			}
			chunkData.ChunkCoordinate = FIntPoint(GlobalX, GlobalZ);
			chunkData.BlockTextureID = blockTextureID;
			chunkData.bHasCollision = bCollisionValue;
			WorldData.Add(chunkData);
		}
	}
}

bool UWorldGenerator::GenerateWorld2(const FString& WorldName, int32 NumRegionsX, int32 NumRegionsZ)
{
	// Calculate the number of chunks per region
	const int32 ChunksPerRegion = 16;
	const int32 BlocksPerChunk = 32;

	// Loop through each region
	for (int32 RegionX = -NumRegionsX / 2; RegionX < NumRegionsX / 2; ++RegionX)
	{
		for (int32 RegionY = -NumRegionsZ / 2; RegionY < NumRegionsZ / 2; ++RegionY)
		{
			TArray<FChunkData> ChunkDataArray;

			// Loop through each chunk in the region
			for (int32 ChunkX = 0; ChunkX < ChunksPerRegion; ++ChunkX)
			{
				for (int32 ChunkY = 0; ChunkY < ChunksPerRegion; ++ChunkY)
				{
					FChunkData ChunkData;
					ChunkData.ChunkCoordinate = FIntPoint(RegionX * ChunksPerRegion + ChunkX, RegionY * ChunksPerRegion + ChunkY);
					ChunkData.BlockTextureID.SetNum(BlocksPerChunk * BlocksPerChunk);
					ChunkData.bHasCollision.SetNum(BlocksPerChunk * BlocksPerChunk);

					// Loop through each block in the chunk
					for (int32 BlockX = 0; BlockX < BlocksPerChunk; ++BlockX)
					{
						for (int32 BlockY = 0; BlockY < BlocksPerChunk; ++BlockY)
						{
							int32 BlockIndex = BlockY * BlocksPerChunk + BlockX;

							// Determine the world Z coordinate of the block
							int32 WorldZ = ChunkData.ChunkCoordinate.Y * BlocksPerChunk + BlockY - (BlocksPerChunk / 2);

							// Set block type based on WorldZ
							if (WorldZ < 0)
							{
								// Below ground level, set to dirt
								ChunkData.BlockTextureID[BlockIndex] = 176; // Dirt
								ChunkData.bHasCollision[BlockIndex] = 1;   // Collision
							}
							else
							{
								// Above ground level, set to air
								ChunkData.BlockTextureID[BlockIndex] = 5;  // Air
								ChunkData.bHasCollision[BlockIndex] = 0;   // No Collision
							}
						}
					}

					// Add chunk data to the array
					ChunkDataArray.Add(ChunkData);
				}
			}

			// Save the region
			if (!FileHandler->SaveRegion(WorldName, FIntPoint(RegionX, RegionY), ChunkDataArray, ChunksPerRegion * ChunksPerRegion))
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to save region: (%d, %d)"), RegionX, RegionY);
			}
		}
	}

	return true;
}
