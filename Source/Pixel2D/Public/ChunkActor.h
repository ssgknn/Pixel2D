// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChunkActor.generated.h"

UCLASS()
class PIXEL2D_API AChunkActor : public AActor
{
	GENERATED_BODY()
	
public:

#pragma region DataVariables

	UPROPERTY()
	int32 ChunkElementCount;

	UPROPERTY()
	int32 ChunkSize;

	UPROPERTY()
	int32 BlockSize;

#pragma endregion DataVariables


#pragma region Tilesets

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tilesets")
	class UPaperTileSet* TileSet0;

#pragma endregion Tilesets

	//Components

	UPROPERTY()
	USceneComponent* AttachComponent;

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* CollisionBoxComponent;

	UPROPERTY(EditAnywhere, Category = "Componens")
	class UTextRenderComponent* TextComponent;

	UPROPERTY(EditAnywhere, Category = "Componens")
	class UPaperTileMapComponent* TileMapComponent;


	// Sets default values for this actor's properties
	AChunkActor(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void LoadChunk();

	void ModifyBlock(FVector HitLocation, int32 DesiredBlockID);

};
