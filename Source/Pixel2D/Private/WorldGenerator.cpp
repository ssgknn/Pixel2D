// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldGenerator.h"

void UWorldGenerator::GenerateWorld(int32 ChunkElementCount)
{
	int32 GenerationRange = 20;

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
					if (GlobalZ > 0)
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
						blockTextureID.Add(176);
						bCollisionValue.Add(1);
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
